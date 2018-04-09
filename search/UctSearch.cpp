
#include "platform/SgSystem.h"
#include "UctSearch.h"

#include <boost/lexical_cast.hpp>
#include <GoUctGlobalSearch.h>
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
    m_skipRaveUpdate.resize(numberPlayouts);
    m_eval.resize(numberPlayouts);
    m_aborted.resize(numberPlayouts);
  }
  for (size_t i = 0; i < numberPlayouts; ++i) {
    m_sequence[i].clear();
    m_skipRaveUpdate[i].clear();
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
  for (auto it = m_skipRaveUpdate.begin();
       it != m_skipRaveUpdate.end(); ++it)
    it->clear();
  m_skipRaveUpdate.clear();
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
        case MSG_SEARCH_LOOP:searcher.SearchLoop(*m_state, &global_lock);
          break;
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
      use_rave(false),
      max_knowledge_threads(1024),
      move_select(SG_UCTMOVESELECT_COUNT),
      rave_check_same(false),
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
      first_play_urgency(10000),
      initial_rave_weight(0.9f),
      final_rave_weight(20000),
      puct_const(sqrt(2.0)),
      use_virtual_loss(false),
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

void UctSearch::ApplyRootFilter(vector<UctMoveInfo>& moves) {
  vector<UctMoveInfo> filteredMoves;
  for (vector<UctMoveInfo>::const_iterator it = moves.begin();
       it != moves.end(); ++it)
    if (find(root_filter.begin(), root_filter.end(), it->uct_move)
        == root_filter.end())
      filteredMoves.push_back(*it);
  moves = filteredMoves;
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

bool UctSearch::CheckAbortSearch(UctThreadState& state) {
  if (ForceAbort()) {
    Debug(state, "UctSearch: abort flag");
    return true;
  }
  const UctNode& root = search_tree.Root();
  if (!UctValueUtil::IsPrecise(root.MoveCount()) && check_float_precision) {
    Debug(state, "UctSearch: floating point type precision reached");
    return true;
  }
  UctValueType rootCount = root.MoveCount();
  if (rootCount >= max_games) {
    Debug(state, "UctSearch: max games reached");
    return true;
  }
  if (root.IsProven()) {
    if (root.IsProvenWin())
      Debug(state, "UctSearch: root is proven win!");
    else
      Debug(state, "UctSearch: root is proven loss!");
    return true;
  }
  const bool isEarlyAbort = CheckEarlyAbort();
  if (isEarlyAbort
      && early_abort_param->reduction_factor * rootCount >= max_games
      ) {
    Debug(state, "UctSearch: max games reached (early abort)");
    early_aborted = true;
    return true;
  }
  if (GamesPlayed() >= next_check_time) {
    next_check_time = GamesPlayed() + check_interval;
    double time = search_timer.GetTime();

    if (time > max_time) {
      Debug(state, "UctSearch: max time reached");
      return true;
    }
    if (isEarlyAbort
        && early_abort_param->reduction_factor * time > max_time) {
      Debug(state, "UctSearch: max time reached (early abort)");
      early_aborted = true;
      return true;
    }
    if (!SgDeterministic::IsDeterministicMode())
      UpdateCheckTimeInterval(time);
    if (move_select == SG_UCTMOVESELECT_COUNT) {
      double remainingGamesDouble = max_games - rootCount - 1;
      if (time > 1.) {
        double remainingTime = max_time - time;
        remainingGamesDouble =
            std::min(remainingGamesDouble,
                     remainingTime * search_stat.searches_per_second);
      }
      UctValueType uctCountMax = numeric_limits<UctValueType>::max();
      UctValueType remainingGames;
      if (remainingGamesDouble >= static_cast<double>(uctCountMax - 1))
        remainingGames = uctCountMax;
      else
        remainingGames = UctValueType(remainingGamesDouble);
      if (CheckCountAbort(state, remainingGames)) {
        Debug(state, "UctSearch: move cannot change anymore");
        return true;
      }
    }
  }
  if (mpi_synchronizer->CheckAbort()) {
    Debug(state, "UctSearch: parallel mpi search finished");
    return true;
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


void UctSearch::ExpandNode(UctThreadState& state, const UctNode& node) {
  size_t threadId = state.thread_id;
  if (!search_tree.HasCapacity(threadId, state.move_info.size())) {
    Debug(state, str(format("UctSearch: maximum tree size %1% reached")
                         % search_tree.MaxNodes()));
    state.tree_exceed_memory_limit = true;
    tree_exceeds_mem_limit = true;
    SgSynchronizeThreadMemory();
    return;
  }
  search_tree.CreateChildren(threadId, node, state.move_info);
}
bool UctSearch::ExpandAndBackPropogate(UctThreadState& state, const UctNode* root, const UctNode& leafNode,
                                       int depth) {
  state.move_info.clear();
  UctProvenType provenType = PROVEN_NONE;
  state.GenerateAllMoves(1, state.move_info, provenType);
  if (state.move_info.empty()) {
#ifndef NDEBUG
    if (!terminate_logged) {
      SgDebug() << "ExpandAndBackPropogate:: no legal moves to select, even PASS:" << GO_PASS << '\n';
      terminate_logged = true;
    }
#endif
    return false;
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
  state.CollectFeatures(eval_thread->eval_buf.feature_buf[threadId], NUM_MAPS, depth);
  state.eval_msg.state_ready = true;
  state.eval_msg.state_evaluated = false;
  state.eval_msg.WaitEvalFinish();

  UpdatePriorProbability(leafNode, eval_thread->eval_buf.policy_out[threadId]);
  BackupTree(root, &leafNode, eval_thread->eval_buf.values_out[threadId]);
#endif

  return true;
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
    if (!child.HasMean()
        && !((move_select == SG_UCTMOVESELECT_BOUND
            || move_select == SG_UCTMOVESELECT_ESTIMATE
        )
            && use_rave
            && child.HasRaveValue()
        )
        )
      continue;
    UctValueType value;
    switch (move_select) {
      case SG_UCTMOVESELECT_VALUE:value = InverseEstimate(UctValueType(child.Mean()));
        break;
      case SG_UCTMOVESELECT_COUNT:value = child.MoveCount();
        break;
      case SG_UCTMOVESELECT_BOUND:value = GetBound(use_rave, node, child);
        break;
      case SG_UCTMOVESELECT_PUCT:value = GetCPUCTValue(node, child);
        break;
      case SG_UCTMOVESELECT_ESTIMATE:value = GetValueEstimate(use_rave, child);
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
  return child.getMeanActionValue() + puct_const * child.getPrior() * sqrt(node.VisitCount()) / (1+child.VisitCount());
#else
  return child.Mean() + puct_const * child.getPrior() * sqrt(node.VisitCount()) / (1 + child.VisitCount());
#endif
}

UctValueType UctSearch::GetBound(bool useRave, bool useBiasTerm,
                                 UctValueType logPosCount,
                                 const UctNode& child) const {
  UctValueType value;
  if (useRave)
    value = GetValueEstimateRave(child);
  else
    value = GetValueEstimate(false, child);
  if (bias_term_const == 0.0 || !useBiasTerm)
    return value;
  else {
    auto moveCount = UctValueType(child.MoveCount());
    UctValueType bound =
        value + bias_term_const * sqrt(logPosCount / (moveCount + 1));
    return bound;
  }
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

UctValueType UctSearch::GetValueEstimate(bool useRave, const UctNode& child) const {
  UctValueType value = 0;
  UctValueType weightSum = 0;
  bool hasValue = false;
  UctStatistics uctStats;
  if (child.HasMean()) {
    uctStats.Initialize(child.Mean(), child.MoveCount());
  }
  int virtualLossCount = child.VirtualLossCount();
  if (virtualLossCount > 0) {
    uctStats.Add(InverseEstimate(0), UctValueType(virtualLossCount));
  }

  if (uctStats.IsDefined()) {
    auto weight = UctValueType(uctStats.Count());
    value += weight * InverseEstimate(UctValueType(uctStats.Mean()));
    weightSum += weight;
    hasValue = true;
  }

  if (useRave) {
    UctStatistics raveStats;
    if (child.HasRaveValue()) {
      raveStats.Initialize(child.RaveValue(), child.RaveCount());
    }
    if (virtualLossCount > 0) {
      raveStats.Add(0, UctValueType(virtualLossCount));
    }
    if (raveStats.IsDefined()) {
      UctValueType raveCount = raveStats.Count();
      UctValueType weight =
          raveCount
              / (rave_weight_param_1
                  + rave_weight_param_2 * raveCount
              );
      value += weight * raveStats.Mean();
      weightSum += weight;
      hasValue = true;
    }
  }
  if (hasValue)
    return value / weightSum;
  else
    return first_play_urgency;
}
UctValueType UctSearch::GetMeanValue(const UctNode& child) const {
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
    return first_play_urgency;
}


UctValueType UctSearch::GetValueEstimateRave(const UctNode& child) const {
  DBG_ASSERT(use_rave);
  UctValueType value;
  UctStatistics uctStats;
  if (child.HasMean()) {
    uctStats.Initialize(child.Mean(), child.MoveCount());
  }
  UctStatistics raveStats;
  if (child.HasRaveValue()) {
    raveStats.Initialize(child.RaveValue(), child.RaveCount());
  }
  int virtualLossCount = child.VirtualLossCount();
  if (virtualLossCount > 0) {
    uctStats.Add(InverseEstimate(0), UctValueType(virtualLossCount));
    raveStats.Add(0, UctValueType(virtualLossCount));
  }
  bool hasRave = raveStats.IsDefined();

  if (uctStats.IsDefined()) {
    UctValueType moveValue = InverseEstimate((UctValueType)uctStats.Mean());
    if (hasRave) {
      UctValueType moveCount = uctStats.Count();
      UctValueType raveCount = raveStats.Count();
      UctValueType weight =
          raveCount
              / (moveCount
                  * (rave_weight_param_1 + rave_weight_param_2 * raveCount)
                  + raveCount);
      value = weight * raveStats.Mean() + (1.f - weight) * moveValue;
    } else {
      DBG_ASSERT(num_threads > 1 && lock_free);
      value = moveValue;
    }
  } else if (hasRave)
    value = raveStats.Mean();
  else
    value = first_play_urgency;
  DBG_ASSERT(num_threads > 1
                 || fabs(value - GetValueEstimate(use_rave, child)) < 1e-3);
  return value;
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


void UctSearch::CreateChildren(UctThreadState& state,
                               const UctNode& node,
                               bool deleteChildTrees) {
  size_t threadId = state.thread_id;
  if (!search_tree.HasCapacity(threadId, state.move_info.size())) {
    Debug(state, str(format("UctSearch: maximum tree size %1% reached")
                         % search_tree.MaxNodes()));
    state.tree_exceed_memory_limit = true;
    tree_exceeds_mem_limit = true;
    SgSynchronizeThreadMemory();
    return;
  }
  search_tree.MergeChildren(threadId, node, state.move_info, deleteChildTrees);
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
  bool hasKnowledge = current->KnowledgeCount() > 0;
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
    if (hasKnowledge && current->KnowledgeCount() == 0) {
      out << " ^";
      hasKnowledge = false;
    }
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
    densePolicy[index] = child.VisitCount();

    if (policy_)
      policy_[GoPointUtil::Point2Index(child.Move())] = (float)child.VisitCount();
  }

  if (policy_)
    ControlAndNormalize(policy_, GO_MAX_MOVES, (float)tau);

  DivideAllByMax(&densePolicy[0], parent->NumChildren());
  auto sum = ControlW<UctValueType>(&densePolicy[0], parent->NumChildren(), tau);
  int select = CumulativeChoose<UctValueType>(&densePolicy[0], parent->NumChildren(), sum, rand_generator);
  auto* bestChild = const_cast<UctNode*>(parent->FirstChild() + select);

  return bestChild;
}

void UctSearch::PlayGame(UctThreadState& state, GlobalRecursiveLock* lock) {
  state.tree_exceed_memory_limit = false;
  state.GameStart();
  UctGameInfo& info = state.game_info;
  info.Clear(num_playouts);
  bool isTerminal;
  bool abortInTree = !PlayInTree(state, isTerminal);
  if (lock != 0)
    lock->unlock();

  if (!info.m_nodes.empty() && isTerminal) {
    const UctNode& terminalNode = *info.m_nodes.back();
    UctValueType eval = state.Evaluate();
    if (eval > 0.6)
      search_tree.SetProvenType(terminalNode, PROVEN_WIN);
    else if (eval < 0.4)
      search_tree.SetProvenType(terminalNode, PROVEN_LOSS);
    PropagateProvenStatus(info.m_nodes);
  }

  size_t nuMovesInTree = info.m_inTreeSequence.size();
  if (!info.m_nodes.empty() && info.m_nodes.back()->IsProven()) {
    for (size_t i = 0; i < num_playouts; ++i) {
      info.m_sequence[i] = info.m_inTreeSequence;
      info.m_skipRaveUpdate[i].assign(nuMovesInTree, false);
      UctValueType eval = info.m_nodes.back()->IsProvenWin() ? 1 : 0;
      size_t nuMoves = info.m_sequence[i].size();
      if (nuMoves % 2 != 0)
        eval = InverseEval(eval);
      info.m_aborted[i] = abortInTree || state.tree_exceed_memory_limit;
      info.m_eval[i] = eval;
    }
  } else {
    state.StartPlayouts();
    for (size_t i = 0; i < num_playouts; ++i) {
      state.StartPlayout();
      info.m_sequence[i] = info.m_inTreeSequence;
      info.m_skipRaveUpdate[i].assign(nuMovesInTree, false);
      bool abort = abortInTree || state.tree_exceed_memory_limit;
      if (!abort && !isTerminal)
        abort = !PlayoutGame(state, i);
      UctValueType eval;
      if (abort)
        eval = UnknownEval();
      else
        eval = state.Evaluate();
      size_t nuMoves = info.m_sequence[i].size();
      if (nuMoves % 2 != 0)
        eval = InverseEval(eval);
      info.m_aborted[i] = abort;
      info.m_eval[i] = eval;

      state.EndPlayout();
      state.TakeBackPlayout(nuMoves - nuMovesInTree);
    }
  }
  state.TakeBackInTree(nuMovesInTree);
  if (lock != 0)
    lock->lock();

  UpdateTree(info);
  if (use_rave)
    UpdateRaveValues(state);
  UpdateStatistics(info);
}


void UctSearch::PropagateProvenStatus(const std::vector<const UctNode*>& nodes) {
  if (nodes.size() <= 1)
    return;
  size_t i = nodes.size() - 2;
  while (true) {
    const UctNode& parent = *nodes[i];
    UctProvenType type = PROVEN_LOSS;
    for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
      const UctNode& child = *it;
      if (!child.IsProven())
        type = PROVEN_NONE;
      else if (child.IsProvenLoss()) {
        type = PROVEN_WIN;
        break;
      }
    }
    if (type == PROVEN_NONE)
      break;
    else
      search_tree.SetProvenType(parent, type);
    if (i == 0)
      break;
    --i;
  }
}


bool UctSearch::PlayInTree(UctThreadState& state, bool& isTerminal) {
  vector<GoMove>& sequence = state.game_info.m_inTreeSequence;
  vector<const UctNode*>& nodes = state.game_info.m_nodes;
  const UctNode* root = &search_tree.Root();
  const UctNode* current = root;
  if (use_virtual_loss && num_threads > 1)
    search_tree.AddVirtualLoss(*current);
  nodes.push_back(current);
  bool breakAfterSelect = false;
  isTerminal = false;
  bool useBiasTerm = false;
  if (--state.randomize_bias_cnt == 0) {
    useBiasTerm = true;
    state.randomize_bias_cnt = bias_term_freq;
  }
  while (true) {
    if (bias_term_depth > 0 && sequence.size() == bias_term_depth)
      useBiasTerm = false;
    if (sequence.size() == max_move_length)
      return false;
    if (current->IsProven())
      break;
    if (!current->HasChildren())
    {
      state.move_info.clear();
      UctProvenType provenType = PROVEN_NONE;
      state.GenerateAllMoves(0, state.move_info, provenType);
      if (current == root)
        ApplyRootFilter(state.move_info);
      if (provenType != PROVEN_NONE) {
        search_tree.SetProvenType(*current, provenType);
        PropagateProvenStatus(nodes);
        break;
      }
      if (state.move_info.empty()) {
        isTerminal = true;
        break;
      }
      if (current->MoveCount() >= expand_thredhold) {
        ExpandNode(state, *current);
        if (state.tree_exceed_memory_limit)
          return true;
        breakAfterSelect = true;
      } else
        break;
    }
    current = &SelectChild(state.randomize_rave_cnt, useBiasTerm, *current);
    if (use_virtual_loss && num_threads > 1)
      search_tree.AddVirtualLoss(*current);
    nodes.push_back(current);
    GoMove move = current->Move();
    state.Execute(move);
    sequence.push_back(move);
    if (breakAfterSelect)
      break;
  }
  return true;
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
    if (node == root)
      node = SelectWithDirichletNoise(state, *node, puct_const);
    else
      node = Select(state, *node, puct_const);
    GoMove move = node->Move();
    state.Apply(move);
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
    expanded = ExpandAndBackPropogate(state, root, *node, (int)sequence.size());
  }
#ifndef NDEBUG
  else
    SgDebug() << "DeepUctSearchTree:: NULL node " << '\n';
#endif
  state.TakeBackInTree(sequence.size());
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (use_virtual_loss && num_threads > 1)
      search_tree.RemoveVirtualLoss(*nodes[i]);
  }
  search_stat.moves_in_tree.Add(static_cast<float>(gameInfo.m_inTreeSequence.size()));

  return expanded ? 1 : 0;
}
bool UctSearch::PlayoutGame(UctThreadState& state, std::size_t playout) {
  UctGameInfo& info = state.game_info;
  vector<GoMove>& sequence = info.m_sequence[playout];
  vector<bool>& skipRaveUpdate = info.m_skipRaveUpdate[playout];
  while (true) {
    if (sequence.size() == max_move_length)
      return false;
    bool skipRave = false;
    GoMove move = state.GeneratePlayoutMove(skipRave);
    if (move == GO_NULLMOVE)
      break;

    state.ExecutePlayout(move);
    sequence.push_back(move);
    skipRaveUpdate.push_back(skipRave);
  }
  return true;
}

UctValueType UctSearch::StartSearchThread(UctValueType maxGames, double maxTime,
                                          vector<GoMove>& sequence,
                                          const vector<GoMove>& rootFilter,
                                          UctSearchTree* initTree,
                                          UctEarlyAbortParam* earlyAbort) {
  search_timer.Start();
  root_filter = rootFilter;
  if (log_games) {
    logger_stream.open(mpi_synchronizer->ToNodeFilename(log_file_name).c_str());
    logger_stream << "PreStartSearch maxGames=" << maxGames << '\n';
  }
  max_games = maxGames;
  max_time = maxTime;
  early_abort_param.reset(nullptr);
  if (earlyAbort != nullptr)
    early_abort_param.reset(new UctEarlyAbortParam(*earlyAbort));

  Msg msg;
  msg.type = MSG_SEARCH_LOOP;
  for (auto& thread : search_threads) {
    thread->m_state->search_initialised = false;
  }
  PreStartSearch(rootFilter, initTree);
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
  EndSearch();
  search_stat.time_elapsed = search_timer.GetTime();
  if (search_stat.time_elapsed > numeric_limits<double>::epsilon())
    search_stat.searches_per_second = GamesPlayed() / search_stat.time_elapsed;
  if (log_games)
    logger_stream.close();
  FindBestSequence(sequence);
  return search_tree.Root().MoveCount() > 0 ?
         search_tree.Root().Mean() :
         UctValueType(0.5);
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
  if (eval_thread == 0)
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
  UctNode* node = DeepUCTSelectBestChild(tau, policy_);
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


void UctSearch::SearchLoop(UctThreadState& state, GlobalRecursiveLock* lock) {
  if (!state.search_initialised) {
    OnThreadStartSearch(state);
    state.search_initialised = true;
  }

  if (NumberThreads() == 1 || lock_free)
    lock = nullptr;
  if (lock != nullptr)
    lock->lock();
  state.tree_exceed_memory_limit = false;
  while (!state.tree_exceed_memory_limit) {
    PlayGame(state, lock);
    OnSearchIteration(num_games + 1, state.thread_id,
                      state.game_info);
    if (log_games)
      logger_stream << SummaryLine(state.game_info) << '\n';

#ifdef LOG_SEARCH
    SgDebug() << "Games:" << num_games << '\n';
#endif
    ++num_games;
    if (tree_exceeds_mem_limit)
      break;
    if (search_aborted || CheckAbortSearch(state)) {
      search_aborted = true;
      SgSynchronizeThreadMemory();
      break;
    }
  }
  if (lock != nullptr)
    lock->unlock();

  search_finish_barier->wait();
  if (search_aborted || !prune_full_tree)
    OnThreadEndSearch(state);
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
    if (tree_exceeds_mem_limit)
      break;
    if (search_aborted || CheckAbortForDeepSearch(state)) {
      search_aborted = true;
      SgSynchronizeThreadMemory();
      break;
    }
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

GoPoint UctSearch::SearchOnePly(UctValueType maxGames, double maxTime,
                                UctValueType& value) {
  if (search_threads.size() == 0)
    CreateThreads();
  OnStartSearch();
  UctThreadState& state = ThreadState(0);
  state.StartSearch();
  vector<UctMoveInfo> moves;
  UctProvenType provenType;
  state.GameStart();
  state.GenerateAllMoves(0, moves, provenType);
  vector<UctStatistics> statistics(moves.size());
  UctValueType games = 0;
  search_timer.Start();
  UctGameInfo& info = state.game_info;
  while (games < maxGames && search_timer.GetTime() < maxTime && !ForceAbort()) {
    for (size_t i = 0; i < moves.size(); ++i) {
      state.GameStart();
      info.Clear(1);
      GoMove move = moves[i].uct_move;
      state.Execute(move);
      info.m_inTreeSequence.push_back(move);
      info.m_sequence[0].push_back(move);
      info.m_skipRaveUpdate[0].push_back(false);
      state.StartPlayouts();
      state.StartPlayout();
      bool abortGame = !PlayoutGame(state, 0);
      UctValueType eval;
      if (abortGame)
        eval = UnknownEval();
      else
        eval = state.Evaluate();
      state.EndPlayout();
      state.TakeBackPlayout(info.m_sequence[0].size() - 1);
      state.TakeBackInTree(1);
      statistics[i].Add(info.m_sequence[0].size() % 2 == 0 ?
                        eval : InverseEval(eval));
      OnSearchIteration(games + 1, 0, info);
      games += 1;
    }
  }
  GoMove bestMove = GO_NULLMOVE;
  for (size_t i = 0; i < moves.size(); ++i) {
    SgDebug() << MoveString(moves[i].uct_move)
              << ' ' << statistics[i].Mean()
              << ", " << statistics[i].Count() << " Simulations"
              << '\n';
    if (bestMove == GO_NULLMOVE || statistics[i].Mean() > value) {
      bestMove = moves[i].uct_move;
      value = statistics[i].Mean();
    }
  }
  return bestMove;
}

const UctNode& UctSearch::SelectChild(int& randomizeCounter,
                                      bool useBiasTerm,
                                      const UctNode& node) {
  bool useRave = use_rave;
  if (randomize_rave_freq > 0 && --randomizeCounter == 0) {
    useRave = false;
    randomizeCounter = randomize_rave_freq;
  }
  DBG_ASSERT(node.HasChildren());
  UctValueType posCount = node.PosCount();
  int virtualLossCount = node.VirtualLossCount();
  if (virtualLossCount > 1) {
    posCount += UctValueType(virtualLossCount - 1);
  }
  if (posCount == 0)
    return *UctChildNodeIterator(search_tree, node);

  const UctValueType logPosCount = Log(posCount);
  const UctNode* bestChild = 0;
  UctValueType bestUpperBound = 0;
  const UctValueType predictorWeight = 0.0;
  auto epsilon = UctValueType(1e-7);
  for (UctChildNodeIterator it(search_tree, node); it; ++it) {
    const UctNode& child = *it;
    if (!child.IsProvenWin())
    {
      UctValueType bound = GetBound(useRave, useBiasTerm, logPosCount, child) -
          predictorWeight * child.PredictorValue();
      if (bestChild == 0 || bound > bestUpperBound + epsilon) {
        bestChild = &child;
        bestUpperBound = bound;
      }
    }
  }
  if (bestChild != nullptr)
    return *bestChild;
  return *node.FirstChild();
}

const UctNode* UctSearch::Select(UctThreadState& state, const UctNode& parent, UctValueType c_puct) {
  SuppressUnused(state);
  DBG_ASSERT(parent.HasChildren());

  UctValueType posCount = parent.PosCount();
  int virtualLossCount = parent.VirtualLossCount();
  if (virtualLossCount > 1) {
    posCount += UctValueType(virtualLossCount - 1);
  }

  UctValueType maxValue = std::numeric_limits<UctValueType>::min();
  const UctNode* bestChild = nullptr;
  for (UctChildNodeIterator it(search_tree, parent); it; ++it) {
    const UctNode& child = *it;
    if (child.Move() != GO_PASS) {
      if (num_threads == 1)
        DBG_ASSERT(child.MoveCount() == child.VisitCount());
      UctValueType value = GetMeanValue(child) + c_puct * child.getPrior() * sqrt(posCount) / (1 + child.MoveCount());

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
  UctValueType posCount = parent.PosCount();
  int virtualLossCount = parent.VirtualLossCount();
  if (virtualLossCount > 1) {
    posCount += UctValueType(virtualLossCount - 1);
  }

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
          GetMeanValue(child) + pCut * prior * sqrt(posCount) / (1 + child.VisitCount() + child.VirtualLossCount());
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
  if (enable && move_range <= 0)
    throw SgException("RAVE not supported for this game");
  use_rave = enable;
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
  rave_weight_param_1 = UctValueType(1.0 / initial_rave_weight);
  rave_weight_param_2 = UctValueType(1.0 / final_rave_weight);
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


void UctSearch::UpdateRaveValues(UctThreadState& state) {
  for (size_t i = 0; i < num_playouts; ++i)
    UpdateRaveValues(state, i);
}

void UctSearch::UpdateRaveValues(UctThreadState& state,
                                 std::size_t playout) {
  UctGameInfo& info = state.game_info;
  const vector<GoMove>& sequence = info.m_sequence[playout];
  if (sequence.size() == 0)
    return;
  DBG_ASSERT(move_range > 0);
  size_t* firstPlay = state.first_play.get();
  size_t* firstPlayOpp = state.first_play_opp.get();
  std::fill_n(firstPlay, move_range, numeric_limits<size_t>::max());
  std::fill_n(firstPlayOpp, move_range, numeric_limits<size_t>::max());
  const vector<const UctNode*>& nodes = info.m_nodes;
  const vector<bool>& skipRaveUpdate = info.m_skipRaveUpdate[playout];
  UctValueType eval = info.m_eval[playout];
  UctValueType invEval = InverseEval(eval);
  size_t nuNodes = nodes.size();
  size_t i = sequence.size() - 1;
  bool opp = (i % 2 != 0);
  for (; i >= nuNodes; --i) {
    DBG_ASSERT(i < skipRaveUpdate.size());
    DBG_ASSERT(i < sequence.size());
    if (!skipRaveUpdate[i]) {
      GoMove mv = sequence[i];
      size_t& first = (opp ? firstPlayOpp[mv] : firstPlay[mv]);
      if (i < first)
        first = i;
    }
    opp = !opp;
  }

  while (true) {
    DBG_ASSERT(i < skipRaveUpdate.size());
    DBG_ASSERT(i < sequence.size());
    DBG_ASSERT(i >= info.m_inTreeSequence.size() || !skipRaveUpdate[i]);
    if (!skipRaveUpdate[i]) {
      GoMove mv = sequence[i];
      size_t& first = (opp ? firstPlayOpp[mv] : firstPlay[mv]);
      if (i < first)
        first = i;
      if (opp)
        UpdateRaveValues(state, playout, invEval, i,
                         firstPlayOpp, firstPlay);
      else
        UpdateRaveValues(state, playout, eval, i,
                         firstPlay, firstPlayOpp);
    }
    if (i == 0)
      break;
    --i;
    opp = !opp;
  }
}

void UctSearch::UpdateRaveValues(UctThreadState& state,
                                 std::size_t playout, UctValueType eval,
                                 std::size_t i,
                                 const std::size_t firstPlay[],
                                 const std::size_t firstPlayOpp[]) {
  DBG_ASSERT(i < state.game_info.m_nodes.size());
  const UctNode* node = state.game_info.m_nodes[i];
  if (!node->HasChildren())
    return;
  size_t len = state.game_info.m_sequence[playout].size();
  for (UctChildNodeIterator it(search_tree, *node); it; ++it) {
    const UctNode& child = *it;
    GoMove mv = child.Move();
    size_t first = firstPlay[mv];
    DBG_ASSERT(first >= i);
    if (first == numeric_limits<size_t>::max())
      continue;
    if (rave_check_same && SgUtil::InRange(firstPlayOpp[mv], i, first))
      continue;
    UctValueType weight;
    if (weight_rave_updates)
      weight = 2 - UctValueType(first - i) / UctValueType(len - i);
    else
      weight = 1;
    search_tree.AddRaveValue(child, eval, weight);
  }
}

void UctSearch::UpdatePriorProbability(const UctNode& node, UctValueType actionProbs[]) {
  for (UctChildNodeIterator it(search_tree, node); it; ++it) {
    const UctNode& child = *it;
    int index = GoPointUtil::Point2Index(child.Move());
    const_cast<UctNode&>(child).SetPrior(actionProbs[index]);
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

void UctSearch::UpdateTree(const UctGameInfo& info) {
  UctValueType eval = 0;
  for (size_t i = 0; i < num_playouts; ++i)
    eval += info.m_eval[i];
  eval /= UctValueType(num_playouts);
  UctValueType inverseEval = InverseEval(eval);
  const vector<const UctNode*>& nodes = info.m_nodes;
  auto count =
      UctValueType(m_updateMultiplePlayoutsAsSingle ? 1 : num_playouts);
  for (size_t i = 0; i < nodes.size(); ++i) {
    const UctNode& node = *nodes[i];
    const UctNode* father = (i > 0 ? nodes[i - 1] : 0);
    search_tree.AddGameResults(node, father, i % 2 == 0 ? eval : inverseEval,
                               count);
    if (use_virtual_loss && num_threads > 1)
      search_tree.RemoveVirtualLoss(node);
  }
}
void UctSearch::BackupTree(const UctNode* root, const UctNode* node, UctValueType value) {
  auto* current = const_cast<UctNode*> (node);
  while (true) {
    search_tree.AddGameResults(*current, current->Parent(), value, 1);
#ifdef USE_DIRECT_VISITCOUNT
    current->AddValue(value);
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
