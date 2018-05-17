
#include "platform/SgSystem.h"
#include "UctSearch.h"

#include <boost/lexical_cast.hpp>
#include <GoUctGlobalSearch.h>
#include <lib/ArrayUtil.h>
#include "platform/SgPlatform.h"
#include "Nums.h"
#include "lib/FileUtil.h"
#include "SgGameWriter.h"
#include "UctTreeUtil.h"
#include "funcapproximator/DlTFRecordWriter.h"

using boost::barrier;
using boost::condition;
using boost::format;
using boost::mutex;
using boost::shared_ptr;
using boost::io::ios_all_saver;
using std::vector;
using std::setprecision;
using std::numeric_limits;
using std::fixed;

namespace {
size_t GetMaxNodesDefault() {
  size_t totalMemory = SgPlatform::TotalMemory();
#ifdef DEBUG_MEMORY
  SgDebug() << "Total RAM:";
  if (totalMemory == 0)
      SgDebug() << "unknown";
  else
      SgDebug() << totalMemory;
#endif
  size_t searchMemory = size_t(totalMemory * 0.32);
  if (searchMemory < 384000000)
    searchMemory = 384000000;
  size_t memoryPerTree = searchMemory;
  size_t nodesPerTree = memoryPerTree / sizeof(UctNode);
#ifdef DEBUG_MEMORY
  SgDebug() << ", " << searchMemory << " used for current player\n";
#endif
  return nodesPerTree;
}

void Notify(mutex& aMutex, condition& aCondition) {
  mutex::scoped_lock lock(aMutex);
  aCondition.notify_all();
}

}

const size_t INVALID_THREAD_ID = numeric_limits<size_t>::max();

void UctGameInfo::Clear(std::size_t numberPlayouts) {
  m_nodes.clear();
  m_inTreeSequence.clear();
  if (numberPlayouts != m_sequence.size()) {
    m_sequence.resize(numberPlayouts);
    m_eval.resize(numberPlayouts);
    m_aborted.resize(numberPlayouts);
  }
  for (size_t i = 0; i < numberPlayouts; ++i) {
    m_sequence[i].clear();
  }
}

void UctGameInfo::Clear() {
  m_aborted.clear();
  m_inTreeSequence.clear();
  m_eval.clear();
  m_nodes.clear();
  for (auto it = m_sequence.begin();
       it != m_sequence.end(); ++it)
    it->clear();
  m_sequence.clear();
}

EvalMsg::EvalMsg() :
    thread_id(INVALID_THREAD_ID), msg_type(MSG_UNKNOWN) {
}

EvalMsg::EvalMsg(size_t threadID, EvalMsgType type) :
    thread_id(threadID), msg_type(type), thread_exited(false), state_ready(false) {
}

void EvalMsg::WaitEvalFinish() {
  boost::mutex::scoped_lock lock(mutex);
  condition_variable.wait(lock, [this] { return state_evaluated; });
}

void EvalMsg::SetThreadExited(bool exited) {
  thread_exited = exited;
}
UctThreadState::UctThreadState(unsigned int threadId, int moveRange)
    : thread_id(threadId),
      eval_msg(threadId),
      search_initialised(false),
      tree_exceed_memory_limit(false) {
  if (moveRange > 0) {
    first_play.reset(new size_t[moveRange]);
    first_play_opp.reset(new size_t[moveRange]);
  }
}

UctThreadState::~UctThreadState() {
}

void UctThreadState::Clear() {
  game_info.Clear();
  move_info.clear();
  excluded_moves.clear();
}

void UctThreadState::EndPlayout() {
}

void UctThreadState::GameStart() {
}

void UctThreadState::StartPlayout() {
}

void UctThreadState::StartPlayouts() {
}

UctThreadStateFactory::~UctThreadStateFactory() {}

UctSearch::NetworkEvalThread::Function::Function(NetworkEvalThread& thread)
    : thread(thread) {}

void UctSearch::NetworkEvalThread::Function::operator()() {
  thread();
}

UctSearch::NetworkEvalThread::NetworkEvalThread(UctSearch& search) :
    neural_initialized(false),
    evaluator(),
    searcher(search),
    to_quit(false),
    paused(true),
    thread_ready_barrier(2),
    sys_thread(Function(*this)) {
  for (size_t i = 0; i < searcher.num_threads; ++i)
    thread_msg[i] = &search.ThreadState(i).eval_msg;
  for (size_t i = searcher.num_threads; i < MAX_BATCHES; ++i) {
    thread_msg[i] = new EvalMsg(INVALID_THREAD_ID);
  }

  thread_ready_barrier.wait();
}

UctSearch::NetworkEvalThread::~NetworkEvalThread() {
  to_quit = true;
  mutex::scoped_lock lock(wait_mutex);
  wait_cv.notify_all();
  sys_thread.join();
}

void UctSearch::NetworkEvalThread::OnSearchThreadExit(size_t threadID) {
  thread_msg[threadID]->thread_exited = true;
}
void UctSearch::NetworkEvalThread::operator()() {
#ifndef NDEBUG
  SgDebug() << "neural network evaluation thread started!" << '\n';
#endif
  thread_ready_barrier.wait();

  while (true) {
    if (paused) {
      boost::mutex::scoped_lock lock(wait_mutex);
      wait_cv.wait(lock, [this] { return !paused; });
    } else {

      if (to_quit)
        break;
      if (!new_checkpoint.empty()) {
        TryLoadNeuralNetwork();
        evaluator.UpdateCheckPoint(new_checkpoint);
        new_checkpoint = "";
      }
      evaluator.EvaluateState(eval_buf.feature_buf,
                              eval_buf.policy_out,
                              eval_buf.values_out,
                              searcher.num_threads);

      for (size_t i = 0; i < searcher.num_threads; ++i) {
        EvalMsg* msg = thread_msg[i];
        if (!msg->thread_exited && msg->state_ready && !msg->state_evaluated) {
          {
            boost::mutex::scoped_lock mslk(msg->mutex);
            msg->state_evaluated = true;
            msg->state_ready = false;
          }
          msg->condition_variable.notify_all();
        }
      }
    }
  }

#ifndef NDEBUG
  SgDebug() << "neural network evaluation thread started terminated" << '\n';
#endif
}

void UctSearch::NetworkEvalThread::Start() {
  for (std::size_t i = 0; i < searcher.num_threads; ++i)
    thread_msg[i]->SetThreadExited(false);
  {
    mutex::scoped_lock lock(wait_mutex);
    paused = false;
  }
  wait_cv.notify_all();
}

void UctSearch::NetworkEvalThread::Stop() {
  paused = true;
}

const std::string& UctSearch::NetworkEvalThread::getCheckPoint() {
  return new_checkpoint;
}

bool UctSearch::NetworkEvalThread::TryLoadNeuralNetwork() {
  if (!neural_initialized) {
    evaluator.LoadGraph(DlConfig::GetInstance().get_metagraph());
    neural_initialized = true;

    return evaluator.GraphLoaded();
  }

  return false;
}

void UctSearch::NetworkEvalThread::UpdateCheckPoint(const std::string& checkpoint) {
  if (checkpoint != new_checkpoint) {
    new_checkpoint = checkpoint;
  }
}

UctSearch::Thread::Function::Function(Thread& thread)
    : thread(thread) {}

void UctSearch::Thread::Function::operator()() {
  thread();
}

UctSearch::Thread::Thread(UctSearch& search, std::unique_ptr<UctThreadState>& state)
    : m_state(std::move(state)),
      searcher(search),
      should_quit(false),
      thread_ready_barrier(2),
      play_finish_lock(play_finish_mutex),
      global_lock(search.global_mutex, boost::defer_lock),
      sys_thread(Function(*this)) {
  thread_ready_barrier.wait();
}

UctSearch::Thread::~Thread() {
  should_quit = true;
  NotifyStartPlay();
  sys_thread.join();
}
void UctSearch::Thread::operator()() {
#ifndef NDEBUG
  SgDebug() << "Search Thread " << m_state->thread_id << " started\n";
#endif
  mutex::scoped_lock lock(start_play_mutex);
  thread_ready_barrier.wait();
  while (true) {
    start_cond.wait(lock);
    if (should_quit)
      break;
    while (!msg_q.empty()) {
      Msg& msg = msg_q.front();
      switch (msg.type) {
        case MSG_DEEP_UCT_SEARCH:searcher.DeepUCTSearchLoop(*m_state, &global_lock);
          break;
        case MSG_ESTIMATE_SCORE: {
          UctValueType score = searcher.EstimateScore(*m_state);
          *((UctValueType*)msg.data) = score;
          break;
        }
        default:break;
      }
      msg_q.pop();
    }
    Notify(play_finish_mutex, finish_cond);
  }

#ifndef NDEBUG
  SgDebug() << "UctSearch::Thread: finishing thread " << m_state->thread_id << '\n';
#endif
}

void UctSearch::Thread::HandleMsg(Msg& msg) {
  msg_q.push(msg);
}

void UctSearch::Thread::NotifyStartPlay(Msg& msg) {
  HandleMsg(msg);
  NotifyStartPlay();
}

void UctSearch::Thread::NotifyStartPlay() {
  Notify(start_play_mutex, start_cond);
}

void UctSearch::Thread::WaitPlayFinished() {
  finish_cond.wait(play_finish_lock);
}

UctSearchStat::UctSearchStat() : time_elapsed(0), searches_per_second(0) {}

void UctSearchStat::Clear() {
  time_elapsed = 0;
  searches_per_second = 0;
  game_length.Clear();
  moves_in_tree.Clear();
  search_aborted.Clear();
}

void UctSearchStat::Write(std::ostream& out) const {
  ios_all_saver saver(out);
  out << SgWriteLabel("Time") << setprecision(2) << time_elapsed << '\n'
      << SgWriteLabel("GameLen") << fixed << setprecision(1);
  game_length.Write(out);
  out << '\n'
      << SgWriteLabel("InTree");
  moves_in_tree.Write(out);
  out << '\n'
      << SgWriteLabel("Aborted")
      << static_cast<int>(100 * search_aborted.Mean()) << "%\n"
      << SgWriteLabel("Games/s") << fixed << setprecision(1)
      << searches_per_second << '\n';
}
UctEarlyAbortParam::UctEarlyAbortParam() : abort_threshold(0), min_searches_to_abort(0), reduction_factor(0) {}

UctSearch::UctSearch(UctThreadStateFactory* threadStateFactory, int moveRange)
    : th_state_factory(threadStateFactory),
#ifndef NDEBUG
    log_games(true),
#else
      log_games(false),
#endif
      prune_tree(true),
      max_knowledge_threads(1024),
      move_select(SG_UCTMOVESELECT_COUNT),
      randomize_rave_freq(20),
      lock_free(SgPlatform::GetLockFreeDefault()),
      weight_rave_updates(true),
      prune_full_tree(true),
      check_float_precision(true),
      num_threads(1),
      num_playouts(1),
      m_updateMultiplePlayoutsAsSingle(true),
      max_nodes(GetMaxNodesDefault()),
      min_prune_cnt(16),
      move_range(moveRange),
      max_move_length(numeric_limits<size_t>::max()),
      expand_thredhold(numeric_limits<UctValueType>::epsilon()),
      bias_term_const(0.7f),
      bias_term_freq(1),
      bias_term_depth(0),
      first_play_urgency(0),
      initial_rave_weight(0.9f),
      final_rave_weight(20000),
      // puct_const(sqrt(2.0)),
      puct_const(2.5),
      use_virtual_loss(false),
      select_with_dirichlet(false),
      log_file_name("uctsearch.log"),
#if USE_FASTLOG
      fast_logrithm(10),
#endif
      sync_state(true),
      mpi_synchronizer(NullMpiSynchronizer::Create()) {
}

UctSearch::~UctSearch() {
  search_aborted = true;
  DeleteThreads();

#ifdef USE_NNEVALTHREAD
  eval_thread.reset(0);
#endif
}

void UctSearch::UpdateCheckPoint(const std::string& checkpoint) {
  check_point = checkpoint;
}

const std::string& UctSearch::getCheckPoint() {
  return eval_thread->getCheckPoint();
}

UctValueType UctSearch::GamesPlayed() const {
  return search_tree.Root().MoveCount() - start_root_move_cnt;
}

bool UctSearch::CheckAbortForDeepSearch(UctThreadState& state) {
  if (ForceAbort()) {
    Debug(state, "UctSearch: abort flag");
    return true;
  }
  const UctNode& root = search_tree.Root();
  if (!UctValueUtil::IsPrecise(root.MoveCount()) && check_float_precision) {
    Debug(state, "UctSearch: floating point type precision reached");
    return true;
  }
  if (GamesPlayed() >= next_check_time) {
    next_check_time = GamesPlayed() + check_interval;
    double time = search_timer.GetTime();

    if (time > max_time) {
      Debug(state, "UctSearch: max time reached");
      return true;
    }
  }
  return false;
}

bool UctSearch::CheckCountAbort(UctThreadState& state,
                                UctValueType remainingGames) const {
  const UctNode& root = search_tree.Root();
  const UctNode* bestChild = FindBestChild(root);
  if (bestChild == 0)
    return false;
  UctValueType bestCount = bestChild->MoveCount();
  vector<GoMove>& excludeMoves = state.excluded_moves;
  excludeMoves.clear();
  excludeMoves.push_back(bestChild->Move());
  const UctNode* secondBestChild = FindBestChild(root, &excludeMoves);
  if (secondBestChild == 0)
    return false;
  UctValueType secondBestCount = secondBestChild->MoveCount();
  DBG_ASSERT(secondBestCount <= bestCount || num_threads > 1);
  return (remainingGames <= bestCount - secondBestCount);
}

bool UctSearch::CheckEarlyAbort() const {
  const UctNode& root = search_tree.Root();
  return early_abort_param != 0
      && root.HasMean()
      && root.MoveCount() > early_abort_param->min_searches_to_abort
      && root.Mean() > early_abort_param->abort_threshold;
}

void UctSearch::CreateThreads() {
  DeleteThreads();
  for (unsigned int i = 0;
       i < num_threads; ++i) {
    std::unique_ptr<UctThreadState> state(th_state_factory->Create(i, *this));
    shared_ptr<Thread> thread(new Thread(*this, state));
    search_threads.push_back(thread);
  }
  search_tree.CreateAllocators(num_threads);
  search_tree.SetMaxNodes(max_nodes);

  search_finish_barier.reset(new barrier(num_threads));
}


void UctSearch::Debug(const UctThreadState& state,
                      const std::string& textLine) {
  if (num_threads > 1) {
    GlobalRecursiveLock lock(global_mutex);
    SgDebug() << (format("[%1%] %2%\n") % state.thread_id % textLine);
  } else
    SgDebug() << (format("%1%\n") % textLine);
}

void UctSearch::DeleteThreads() {
  search_threads.clear();
}

bool UctSearch::ExpandAndBackup(UctThreadState& state, const UctNode* root, const UctNode& leafNode) {
  state.move_info.clear();
  UctProvenType provenType = PROVEN_NONE;
  state.GenerateAllMoves(1, state.move_info, provenType);
  if (state.move_info.empty()) {
#ifndef NDEBUG
    if (!terminate_logged) {
      SgDebug() << "ExpandAndBackup:: no legal moves to select, even PASS:" << GO_PASS << '\n';
      terminate_logged = true;
    }
#endif
    return false;
  }

  for (UctMoveInfo& moveInfo : state.move_info) {
    if (moveInfo.uct_move == leafNode.MoveNoCheck()) {
      SgDebug() << "Illegal Move generated" << "\n";
    }
  }

  size_t threadId = state.thread_id;
  if (!search_tree.HasCapacity(threadId, state.move_info.size())) {
    Debug(state, str(format("UctSearch: maximum tree size %1% reached") % search_tree.MaxNodes()));
    if (logger_stream)
      logger_stream << "OutOfMemory" << '\n';
    state.tree_exceed_memory_limit = true;
    tree_exceeds_mem_limit = true;
    SgSynchronizeThreadMemory();
    state.move_info.clear();
    return false;
  }
  search_tree.Expand(threadId, leafNode, state.move_info);

#ifdef USE_NNEVALTHREAD
  state.CollectFeatures(eval_thread->eval_buf.feature_buf[threadId], NUM_MAPS);
  // printTransformedFeatures(eval_thread->eval_buf.feature_buf[threadId]);

  state.eval_msg.state_ready = true;
  state.eval_msg.state_evaluated = false;
  state.eval_msg.WaitEvalFinish();

  UpdatePrior(leafNode, eval_thread->eval_buf.policy_out[threadId]);
  BackupTree(root, &leafNode, eval_thread->eval_buf.values_out[threadId]);
#endif

  return true;
}

void UctSearch::printTransformedFeatures(char feature[][BD_SIZE][BD_SIZE]) {
  int h = BD_SIZE;
  int w = BD_SIZE;
  char data[BD_SIZE*BD_SIZE*NUM_MAPS];

  for (int depth = 0; depth < NUM_MAPS; ++depth) {
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        int index = UnrealGo::ArrayUtil::GetOffset({BD_SIZE, BD_SIZE, NUM_MAPS}, 3,
                                                   {i, j, depth});
        data[index] = feature[depth][i][j];
      }
    }
  }

  for (int i=0; i<BD_SIZE*BD_SIZE; ++i) {
    for (int j=0; j<NUM_MAPS; ++j) {
      SgDebug() << data[i*NUM_MAPS+j] << " ";
    }
    SgDebug() << "\n";
  }
  SgDebug() << "\n";
}

const UctNode*
UctSearch::FindBestChild(const UctNode& node,
                         const vector<GoMove>* excludeMoves) const {
  if (!node.HasChildren())
    return 0;
  const UctNode* bestChild = 0;
  UctValueType bestValue = 0;
  for (UctChildNodeIterator it(search_tree, node); it; ++it) {
    const UctNode& child = *it;
    if (excludeMoves != 0) {

      auto begin = excludeMoves->begin();
      auto end = excludeMoves->end();
      if (find(begin, end, child.Move()) != end)
        continue;
    }
    if (child.IsProvenLoss())
    {
      bestChild = &child;
      break;
    }
    UctValueType value;
    switch (move_select) {
      case SG_UCTMOVESELECT_VALUE:value = InverseEstimate(UctValueType(child.Mean()));
        break;
      case SG_UCTMOVESELECT_COUNT:value = child.MoveCount();
        break;
      case SG_UCTMOVESELECT_PUCT:value = GetCPUCTValue(node, child);
        break;
      default:DBG_ASSERT(false);
        value = child.MoveCount();
    }
    if (bestChild == 0 || value > bestValue) {
      bestChild = &child;
      bestValue = value;
    }
  }
  return bestChild;
}

void UctSearch::FindBestSequence(vector<GoMove>& sequence) const {
  sequence.clear();
  const UctNode* current = &search_tree.Root();
  while (true) {
    current = FindBestChild(*current);
    if (current == 0)
      break;
    sequence.push_back(current->Move());
    if (!current->HasChildren())
      break;
  }
}

void UctSearch::generateDirichlet(double out_[], size_t length) {
  boost::mt19937 m_generator;
  boost::gamma_distribution<> m_pdf(0.03);
  boost::variate_generator<boost::mt19937&, boost::gamma_distribution<> > m_gamma_generator(m_generator, m_pdf);
  for (size_t i = 0; i < length; i++) {
    out_[i] = m_gamma_generator();
  }
  Normalize(out_, length);
}

void UctSearch::GenerateAllMoves(std::vector<UctMoveInfo>& moves) {
  if (search_threads.empty())
    CreateThreads();
  moves.clear();
  OnStartSearch();
  UctThreadState& state = ThreadState(0);
  state.StartSearch();
  UctProvenType type;
  state.GenerateAllMoves(0, moves, type);
}

UctValueType UctSearch::GetBound(bool useRave, const UctNode& node,
                                 const UctNode& child) const {
  UctValueType posCount = node.PosCount();
  int virtualLossCount = node.VirtualLossCount();
  if (virtualLossCount > 0) {
    posCount += UctValueType(virtualLossCount);
  }
  return GetBound(useRave, true, Log(posCount), child);
}

UctValueType UctSearch::GetCPUCTValue(const UctNode& node, const UctNode& child) const {
#ifdef USE_DIRECT_VISITCOUNT
  return child.MeanActionValue() + puct_const * child.getPrior() * sqrt(node.VisitCount()) / (1+child.VisitCount());
#else
  return child.Mean() + puct_const * child.getPrior() * sqrt(node.VisitCount()) / (1 + child.VisitCount());
#endif
}

UctValueType UctSearch::GetBound(bool useRave, bool useBiasTerm,
                                 UctValueType logPosCount,
                                 const UctNode& child) const {
  UctValueType value;
  return value;
}

UctSearchTree& UctSearch::GetTempTree() {
  tmp_search_tree.Clear();
  if (tmp_search_tree.NuAllocators() != NumberThreads()) {
    tmp_search_tree.CreateAllocators(NumberThreads());
    tmp_search_tree.SetMaxNodes(MaxNodes());
  } else if (tmp_search_tree.MaxNodes() != MaxNodes()) {
    tmp_search_tree.SetMaxNodes(MaxNodes());
  }
  return tmp_search_tree;
}

UctValueType UctSearch::GetMeanValue(const UctNode& child, UctValueType defaultMean) const {
#ifdef USE_DIRECT_VISITCOUNT
  if (child.VisitCount() > 0)
    return child.MeanActionValue();
  return defaultMean;
#else
  UctStatistics uctStats;
  if (child.HasMean()) {
    uctStats.Initialize(child.Mean(), child.MoveCount());
  }
  int virtualLossCount = child.VirtualLossCount();
  if (virtualLossCount > 0) {
    uctStats.Add(0, UctValueType(virtualLossCount));
  }

  if (uctStats.IsDefined())
    return UctValueType(uctStats.Mean());
  else
    return defaultMean;
#endif
}

std::string UctSearch::LastGameSummaryLine() const {
  return SummaryLine(LastGameInfo());
}

UctValueType UctSearch::Log(UctValueType x) const {
#if USE_FASTLOG
  return UctValueType(fast_logrithm.Log(float(x)));
#else
  return log(x);
#endif
}

void UctSearch::OnStartSearch() {
  mpi_synchronizer->OnStartSearch(*this);
}

void UctSearch::OnStartGamePlay() {
}

void UctSearch::OnEndGamePlay() {}

void UctSearch::OnStartSelfPlay() {
}

void UctSearch::OnEndSelfPlay() {
}

void UctSearch::OnEndSearch() {
  mpi_synchronizer->OnEndSearch(*this);
}


void UctSearch::PrintSearchProgress(double currTime) const {
  const int MAX_SEQ_PRINT_LENGTH = 15;
  const UctValueType MIN_MOVE_COUNT = 10;
  const UctValueType rootMoveCount = search_tree.Root().MoveCount();
  std::ostringstream out;
  const UctNode* current = &search_tree.Root();
  if (rootMoveCount > 0) {
    const UctValueType rootMean = search_tree.Root().Mean();
    out << (format("%s | %.3f | %.0f | %.1f ")
        % SgTime::Format(currTime, true)
        % rootMean % rootMoveCount % search_stat.moves_in_tree.Mean());
  }
  for (int i = 0; i <= MAX_SEQ_PRINT_LENGTH && current->HasChildren(); ++i) {
    current = FindBestChild(*current);
    if (current == 0 || current->MoveCount() < MIN_MOVE_COUNT)
      break;
    if (i == 0)
      out << "|";
    if (i < MAX_SEQ_PRINT_LENGTH) {
      if (i == 0 || !IsPartialMove(current->Move()))
        out << " ";
      out << MoveString(current->Move());
    } else
      out << " *";
  }
  SgDebug() << out.str() << std::endl;
}

void UctSearch::OnSearchIteration(UctValueType gameNumber,
                                  std::size_t threadId,
                                  const UctGameInfo& info) {
  const int DISPLAY_INTERVAL = 5;

  mpi_synchronizer->OnSearchIteration(*this, gameNumber, threadId, info);
  double currTime = search_timer.GetTime();

  if (threadId == 0 && currTime - last_score_disp_time > DISPLAY_INTERVAL) {
    PrintSearchProgress(currTime);
    last_score_disp_time = currTime;
  }
}

UctNode* UctSearch::DeepUCTSelectBestChild(UctValueType tau, float* policy_) {
  GoArray<UctValueType, GO_MAX_MOVES> densePolicy;
  UctNode* parent = &search_tree.Root();
  if (!parent->HasChildren())
    return nullptr;

  for (UctChildNodeIterator it(search_tree, *parent); it; ++it) {
    const UctNode& child = *it;
    size_t index = (it() - parent->FirstChild());
    densePolicy[index] = child.MoveCount();

    if (policy_)
      policy_[GoPointUtil::Point2Index(child.Move())] = (float)child.MoveCount();
  }

  if (policy_)
    ControlAndNormalize(policy_, GO_MAX_MOVES, (float)tau);

  DivideAllByMax(&densePolicy[0], parent->NumChildren());
  auto sum = ControlW<UctValueType>(&densePolicy[0], parent->NumChildren(), tau);
  int select = CumulativeChoose<UctValueType>(&densePolicy[0], parent->NumChildren(), sum, rand_generator);
  auto* bestChild = const_cast<UctNode*>(parent->FirstChild() + select);

  return bestChild;
}

UctNode* UctSearch::DeepUCTSelectBestChild(float* policy_) {
  UctNode* parent = &search_tree.Root();
  if (!parent->HasChildren())
    return nullptr;

  const UctNode* bestChild = nullptr;
  UctValueType maxVisit = std::numeric_limits<UctValueType>::min();
  for (UctChildNodeIterator it(search_tree, *parent); it; ++it) {
    const UctNode& child = *it;
    if (maxVisit < child.MoveCount()) {
      maxVisit = child.MoveCount();
      bestChild = &child;
    }

    if (policy_)
      policy_[GoPointUtil::Point2Index(child.Move())] = (float)child.MoveCount();
  }

  return const_cast<UctNode*>(bestChild);
}

int UctSearch::DeepUctSearchTree(UctThreadState& state, GlobalRecursiveLock* lock) {
  SuppressUnused(lock);
  state.tree_exceed_memory_limit = false;
  state.GameStart();
  UctGameInfo& gameInfo = state.game_info;
  vector<GoMove>& sequence = gameInfo.m_inTreeSequence;
  vector<const UctNode*>& nodes = gameInfo.m_nodes;
  sequence.clear();
  nodes.clear();
  const UctNode* root = &search_tree.Root();
  const UctNode* node = root;
  if (use_virtual_loss && num_threads > 1)
    search_tree.AddVirtualLoss(*node);
  nodes.push_back(node);

  while (node->HasChildren()) {
    if (node == root && select_with_dirichlet)
      node = SelectWithDirichletNoise(state, *node, puct_const);
    else
      node = Select(state, *node, puct_const);
    GoMove move = node->Move();
    state.Apply(move); // update state's board
    sequence.push_back(move);
    nodes.push_back(node);
    if (use_virtual_loss && num_threads > 1)
      search_tree.AddVirtualLoss(*node);
  }

  if (root->Parent() == root) {
#ifndef NDEBUG
    SgDebug() << "DeepUctSearchTree:: exception checked " << '\n';
#endif
  }

  bool expanded = false;
  if (node != nullptr) {
    expanded = ExpandAndBackup(state, root, *node);
  }
#ifndef NDEBUG
  else
    SgDebug() << "DeepUctSearchTree:: NULL node " << '\n';
#endif

  state.TakeBackInTree(sequence.size());
  if (use_virtual_loss && num_threads > 1) {
    for (auto& vnode : nodes)
      search_tree.RemoveVirtualLoss(*vnode);
  }
  search_stat.moves_in_tree.Add(static_cast<float>(gameInfo.m_inTreeSequence.size()));

  return expanded ? 1 : 0;
}

UctValueType UctSearch::StartSearchThread(UctValueType maxGames, double maxTime,
                                          vector<GoMove>& sequence,
                                          const vector<GoMove>& rootFilter,
                                          UctSearchTree* initTree,
                                          UctEarlyAbortParam* earlyAbort) {
  return 0;
}

UctValueType UctSearch::StartDeepUCTSearchThread(UctValueType maxGames, double maxTime,
                                                 std::vector<GoMove>& sequence_,
                                                 std::vector<UctNode*>* bestchild_,
                                                 float* policy_,
                                                 double tau,
                                                 const std::vector<GoMove>& rootFilter,
                                                 UctSearchTree* initTree,
                                                 UctEarlyAbortParam* earlyAbort, bool syncState) {
  search_timer.Start();
  root_filter = rootFilter;
  if (log_games) {
    logger_stream.open(mpi_synchronizer->ToNodeFilename(log_file_name).c_str());
    logger_stream << "PreStartSearch maxGames=" << maxGames << '\n';
  }
  max_games = maxGames;
  max_time = maxTime;
  early_abort_param.reset(0);
  if (earlyAbort != 0)
    early_abort_param.reset(new UctEarlyAbortParam(*earlyAbort));

  Msg msg;
  msg.type = MSG_DEEP_UCT_SEARCH;
  for (auto& thread : search_threads) {
    thread->m_state->search_initialised = false;
  }

  PreStartSearch(rootFilter, initTree, syncState);

#ifdef USE_NNEVALTHREAD
  if (eval_thread == nullptr)
    eval_thread.reset(new NetworkEvalThread(*this));
  if (!check_point.empty()) {
    eval_thread->UpdateCheckPoint(check_point);
    check_point = "";
  }
  eval_thread->Start();
#endif

  UctValueType pruneMinCount = min_prune_cnt;
  while (true) {
    tree_exceeds_mem_limit = false;
    SgSynchronizeThreadMemory();
    for (auto& thread : search_threads) {
      thread->NotifyStartPlay(msg);
    }
    for (auto& thread : search_threads) {
      thread->WaitPlayFinished();
    }

    if (search_aborted || !prune_full_tree)
      break;
    else {
      double startPruneTime = search_timer.GetTime();
      SgDebug() << "UctSearch: pruning nodes with count < "
                << pruneMinCount << " (at time " << fixed << setprecision(1)
                << startPruneTime << ")\n";
      UctSearchTree& tempTree = GetTempTree();
      search_tree.CopyPruneLowCount(tempTree, pruneMinCount, true);
      auto prunedSizePercentage =
          static_cast<int>(tempTree.NuNodes() * 100 / search_tree.NuNodes());
      SgDebug() << "UctSearch: pruned size: " << tempTree.NuNodes()
                << " (" << prunedSizePercentage << "%) time: "
                << (search_timer.GetTime() - startPruneTime) << "\n";
      if (prunedSizePercentage > 50)
        pruneMinCount *= 2;
      else
        pruneMinCount = min_prune_cnt;
      search_tree.Swap(tempTree);
    }
  }
#ifdef USE_NNEVALTHREAD
  eval_thread->Stop();
#endif

  EndSearch();
  search_stat.time_elapsed = search_timer.GetTime();
  if (search_stat.time_elapsed > numeric_limits<double>::epsilon())
    search_stat.searches_per_second = GamesPlayed() / search_stat.time_elapsed;
  if (log_games)
    logger_stream.close();
  // UctNode* node = DeepUCTSelectBestChild(tau, policy_);
  UctNode* node = DeepUCTSelectBestChild(policy_);
  if (node) {
    sequence_.push_back(node->Move());
    if (bestchild_)
      bestchild_->push_back(node);
  }
  return search_tree.Root().MoveCount() > 0 ? search_tree.Root().Mean()
                                            : UctValueType(0.0);
}

UctValueType UctSearch::EstimateGameScore() {
  if (search_threads.empty())
    return -1;

  UctValueType score = -1;
  Msg msg;
  msg.type = MSG_ESTIMATE_SCORE;
  msg.data = &score;
  search_threads[0]->NotifyStartPlay(msg);
  search_threads[0]->WaitPlayFinished();
  return score;
}

void UctSearch::DeepUCTSearchLoop(UctThreadState& state, GlobalRecursiveLock* lock) {
  if (!state.search_initialised) {
    OnThreadStartSearch(state);
    state.search_initialised = true;
  }
  if (NumberThreads() == 1 || lock_free)
    lock = nullptr;
  if (lock != nullptr)
    lock->unlock();

  state.tree_exceed_memory_limit = false;
  int evaluated_times = 0;
  while (!state.tree_exceed_memory_limit && !search_aborted) {
    evaluated_times += DeepUctSearchTree(state, lock);

    ++num_games;
    if (num_games >= max_games) {
      search_aborted = true;
      break;
    }
    if (tree_exceeds_mem_limit) {
      SgDebug() << "tree limit exceeded" << "\n";
      break;
    }
    /*if (search_aborted || CheckAbortForDeepSearch(state)) {
      search_aborted = true;
      SgSynchronizeThreadMemory();
      break;
    }*/
  }

#ifdef USE_NNEVALTHREAD
  eval_thread->OnSearchThreadExit(state.thread_id);
#endif

#ifndef NDEBUG
#endif

  if (lock != 0)
    lock->lock();

  search_finish_barier->wait();
  if (search_aborted || !prune_full_tree)
    OnThreadEndSearch(state);
}

void UctSearch::AddDirichletNoise(const UctNode* root) {
  static double noise[GO_MAX_MOVES];
  rand_generator.generateDirichlet(noise, root->NumChildren());
  int idx = 0;
  for (UctChildNodeIterator it(search_tree, *root); it; ++it) {
    auto& child = const_cast<UctNode&>(*it);
    child.SetPrior(0.75 * child.getPrior() + 0.25 * noise[idx]);
    idx++;
  }
}

UctValueType UctSearch::EstimateScore(UctThreadState& state) {
  return state.FinalScore();
}

void UctSearch::SyncStateAgainst(GoMove oppMove, SgBlackWhite color) {
  SuppressUnused(color);
  if (oppMove != GO_NULLMOVE) {
    for (unsigned int i = 0; i < search_threads.size(); ++i) {
      UctThreadState& state = ThreadState(i);
      DBG_ASSERT(state.Board().ToPlay() == color);
      state.Apply(oppMove);
    }
  }
}

void UctSearch::PrepareGamePlay() {
  for (unsigned int i = 0; i < search_threads.size(); ++i) {
    UctThreadState& state = ThreadState(i);
    state.tree_exceed_memory_limit = false;
    UctGameInfo& info = state.game_info;
    info.Clear(num_playouts);
    state.Clear();
  }

  search_tree.Clear();

  OnStartGamePlay();
}

void UctSearch::OnThreadStartSearch(UctThreadState& state) {
  mpi_synchronizer->OnThreadStartSearch(*this, state);
}

void UctSearch::OnThreadEndSearch(UctThreadState& state) {
  mpi_synchronizer->OnThreadEndSearch(*this, state);
}

const UctNode* UctSearch::Select(UctThreadState& state, const UctNode& parent, UctValueType c_puct) {
  SuppressUnused(state);
  DBG_ASSERT(parent.HasChildren());

  UctValueType total_movecount = 0; // = parent.PosCount();
  int virtualLossCount = parent.VirtualLossCount();
  if (virtualLossCount > 1) {
    total_movecount += UctValueType(virtualLossCount - 1);
  }
  for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
    const UctNode& child = *it;
    total_movecount += child.VisitCount();
  }

  float spc = (float)std::max(std::sqrt(total_movecount), 1.0);
  UctValueType defaultMean = -parent.MeanActionValue();
  UctValueType maxValue = std::numeric_limits<UctValueType>::min();
  const UctNode* bestChild = nullptr;
  for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
    const UctNode& child = *it;
    if (child.Move() != GO_PASS) {
      if (num_threads == 1)
        DBG_ASSERT(child.MoveCount() == child.VisitCount());
      UctValueType value = GetMeanValue(child, defaultMean) + c_puct * child.getPrior() * spc / (1 + child.VisitCount());

      if (bestChild == nullptr || value > maxValue) {
        bestChild = &child;
        maxValue = value;
      }
    }
  }
  if (bestChild == nullptr)
  {
    bestChild = parent.FirstChild();
  }
  return bestChild;
}

const UctNode*
UctSearch::SelectWithDirichletNoise(UctThreadState& state, const UctNode& parent, UctValueType pCut) {
  SuppressUnused(state);
  DBG_ASSERT(parent.HasChildren());
  UctValueType parent_count = 0; //= parent.PosCount();
  int virtualLossCount = parent.VirtualLossCount();
  if (virtualLossCount > 1) {
    parent_count += UctValueType(virtualLossCount - 1);
  }

  for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
    const UctNode& child = *it;
    parent_count += child.MoveCount();
  }
  float spc = (float)std::max(sqrt(parent_count), 1.0);

  double noise[GO_MAX_MOVES];
  generateDirichlet(noise, parent.NumChildren());
  int idx = 0;
  UctValueType maxValue = std::numeric_limits<UctValueType>::min();
  const UctNode* bestChild = nullptr;

  for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
    const UctNode& child = *it;
    if (child.Move() != GO_PASS) {
      UctValueType prior = 0.75 * child.getPrior() + 0.25 * noise[idx];
      UctValueType uctValue =
          GetMeanValue(child) + pCut * prior * spc / (1 + child.VisitCount() + child.VirtualLossCount());
      if (bestChild == nullptr || uctValue > maxValue) {
        bestChild = &child;
        maxValue = uctValue;
      }
    }
    idx++;
  }
  if (bestChild == nullptr)
  {
    bestChild = parent.FirstChild();
  }
  return bestChild;
}

void UctSearch::SetNumberThreads(std::size_t n) {
  DBG_ASSERT(n >= 1);
  if (num_threads == n)
    return;
  num_threads = n;
  CreateThreads();
}

void UctSearch::SetThreadsNumberOnly(std::size_t n) {
  DBG_ASSERT(n >= 1);
  if (num_threads == n)
    return;
  num_threads = n;
}

void UctSearch::SetCheckTimeInterval(UctValueType n) {
  DBG_ASSERT(n >= 0);
  check_interval = n;
}

void UctSearch::SetRave(bool enable) {
}

void UctSearch::SetThreadStateFactory(UctThreadStateFactory* factory) {
  DBG_ASSERT(th_state_factory == 0);
  th_state_factory.reset(factory);
  DeleteThreads();
}

void UctSearch::PreStartSearch(const vector<GoMove>& rootFilter, UctSearchTree* initTree, bool syncState) {
  if (search_threads.size() == 0)
    CreateThreads();
  if (num_threads > 1 && SgTime::DefaultMode() == SG_TIME_CPU)
    SgWarning() << "UctSearch: using cpu time with multiple threads\n";
  rave_weight_param_1 = (1.0 / initial_rave_weight);
  rave_weight_param_2 = (1.0 / final_rave_weight);
  if (initTree == nullptr)
    search_tree.Clear();
  else {
#ifdef CHECKTREECONSISTENCY
    bool initConsistency = UctTreeUtil::CheckTreeConsistency(*initTree, initTree->Root());
    DBG_ASSERT(initConsistency);
#endif
    search_tree.Swap(*initTree);
#ifdef CHECKTREECONSISTENCY
    bool consistency = UctTreeUtil::CheckTreeConsistency(search_tree, search_tree.Root());
#endif
    if (search_tree.HasCapacity(0, search_tree.Root().NumChildren()))
      search_tree.ApplyFilter(search_tree, 0, search_tree.Root(), rootFilter);
    else
      SgWarning() <<
                  "UctSearch: "
                      "root filter not applied (tree reached maximum size)\n";
#ifdef CHECKTREECONSISTENCY
    consistency = UctTreeUtil::CheckTreeConsistency(search_tree, search_tree.Root());
    if (!consistency)
        SgWarning() << "UctSearch::PreStartSearch Inconsistent tree \n";
#endif
  }
  search_stat.Clear();
  search_aborted = false;
  early_aborted = false;
  if (!SgDeterministic::IsDeterministicMode())
    check_interval = 1;
  num_games = 0;
  last_score_disp_time = search_timer.GetTime();
  OnStartSearch();

  next_check_time = UctValueType(check_interval);
  start_root_move_cnt = search_tree.Root().MoveCount();

  if (syncState) {
    for (unsigned int i = 0; i < search_threads.size(); ++i) {
      UctThreadState& state = ThreadState(i);
      state.randomize_rave_cnt = randomize_rave_freq;
      state.randomize_bias_cnt = bias_term_freq;
      state.StartSearch();
    }
  }
}

void UctSearch::EndSearch() {
  OnEndSearch();
}

std::string UctSearch::SummaryLine(const UctGameInfo& info) const {
  std::ostringstream buffer;
  const vector<const UctNode*>& nodes = info.m_nodes;
  for (size_t i = 1; i < nodes.size(); ++i) {
    const UctNode* node = nodes[i];
    GoMove move = node->Move();
    buffer << ' ' << MoveString(move) << " (" << fixed << setprecision(2)
           << node->Mean() << ',' << node->MoveCount() << ')';
  }
  for (size_t i = 0; i < info.m_eval.size(); ++i)
    buffer << ' ' << fixed << setprecision(2) << info.m_eval[i];
  return buffer.str();
}

void UctSearch::UpdateCheckTimeInterval(double time) {
  if (time < numeric_limits<double>::epsilon())
    return;
  double wantedTimeDiff = (max_time > 1 ? 0.1 : 0.1 * max_time);
  if (time < wantedTimeDiff / 10) {
    check_interval *= 2;
    return;
  }
  search_stat.searches_per_second = GamesPlayed() / time;
  double gamesPerSecondPerThread =
      search_stat.searches_per_second / double(num_threads);
  check_interval = UctValueType(wantedTimeDiff * gamesPerSecondPerThread);
  if (check_interval == 0)
    check_interval = 1;
}

void UctSearch::UpdatePrior(const UctNode& node, UctValueType* policies) {
  UctValueType legalSum = 0;
  for (UctChildNodeIterator it(search_tree, node); it; ++it) {
    const UctNode& child = *it;
    int index = GoPointUtil::Point2Index(child.Move());
    legalSum += policies[index];
  }

  for (UctChildNodeIterator it(search_tree, node); it; ++it) {
    const UctNode& child = *it;
    int index = GoPointUtil::Point2Index(child.Move());
    const_cast<UctNode&>(child).SetPrior(policies[index]/legalSum);
  }
}

void UctSearch::UpdateStatistics(const UctGameInfo& info) {
  search_stat.moves_in_tree.Add(
      static_cast<float>(info.m_inTreeSequence.size()));
  for (size_t i = 0; i < num_playouts; ++i) {
    search_stat.game_length.Add(
        static_cast<float>(info.m_sequence[i].size()));
    search_stat.search_aborted.Add(info.m_aborted[i] ? 1.f : 0.f);
  }
}

void UctSearch::BackupTree(const UctNode* root, const UctNode* node, UctValueType value) {
  auto* current = const_cast<UctNode*> (node);
  while (true) {
#ifdef USE_DIRECT_VISITCOUNT
    current->AddValue(value);
#else
    (const_cast<UctNode*>(node))->AddGameResults(value, 1);
#endif
    value = UctValueUtil::MinusInverseValue(value);
    if (current == root)
      break;
    current = current->Parent();
  }
}

void UctSearch::WriteStatistics(std::ostream& out) const {
  out << SgWriteLabel("Count") << search_tree.Root().MoveCount() << '\n'
      << SgWriteLabel("GamesPlayed") << GamesPlayed() << '\n'
      << SgWriteLabel("Nodes") << search_tree.NuNodes() << '\n';
  search_stat.Write(out);
  mpi_synchronizer->WriteStatistics(out);
}