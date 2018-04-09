
#include <zmq.hpp>
#include "platform/SgSystem.h"
#include <GoUctUtil.h>
#include <GoUctPlayoutPolicy.h>
#include "UctDeepPlayer.h"
#include "lib/StringUtil.h"
#include "SgGameWriter.h"
#include "UctTreeUtil.h"
#include "lib/FileUtil.h"
#include "SgUUID.h"
#include "network/AtomCurl.h"
#include "msg/MinioStub.h"

const static int MAX_SEARCH_ITERATIONS = 1600;
const static int SELF_PLAY_GAMES = 2;

UctDeepPlayer::UctDeepPlayer(GoGame& game, GoRules& rules) : GoPlayer(game.Board()),
                                                             m_logReuse(false),
                                                             m_search(
                                                                 Board(),
                                                                 new GoUctPlayoutPolicyFactory<GoUctBoard>(
                                                                     m_playoutPolicyParam),
                                                                 m_playoutPolicyParam,
                                                                 m_treeFilterParam),
                                                             m_maxiterations(10000),
                                                             m_engineRules(rules),
                                                             m_game(game),
                                                             m_aborted(false),
                                                             m_maxGameLength(GO_MAX_SIZE * GO_MAX_SIZE * 2),
                                                             m_forcedOpeningMoves(false),
                                                             m_ignoreClock(false),
                                                             m_reuseSubtree(true),
                                                             m_earlyPass(true),
                                                             m_sureWinThreshold(0.80f),
                                                             m_resignThreshold(-1),
                                                             m_resignMinGames(5000),
                                                             m_timeControl(Board()),
                                                             m_mpiSynchronizer(NullMpiSynchronizer::Create()),
                                                             m_writeDebugOutput(false),
                                                             m_policyAllocator(new UctPolicyAllocator()),
                                                             m_nodeAllocator(new UctNodeAllocator) {
  m_policyAllocator->SetMaxPolicies((size_t)GO_MAX_NUM_MOVES);
  m_nodeAllocator->SetMaxNodes((size_t)GO_MAX_NUM_MOVES);

  m_game.Init(game.Board().Size(), game.Board().Rules());
  m_game.UpdateDate(SgTime::TodaysDate());
  m_game.SetTimeSettingsGlobal(game.TimeSettings(), game.TimeRecord().Overhead());

  m_search.SetPruneMinCount(0);
}

void UctDeepPlayer::ClearBoard() {
  OnGameFinished();
  m_game.Reset(m_engineRules);
  UpdateSubscriber();
  OnNewGame();
  UpdateSubscriber();
}

void UctDeepPlayer::SelfPlayOneGame(const std::string& path, int gameID, bool uploadToServer) {
  UctNode root(GO_NULLMOVE);
  std::vector<UctNode*> selectedChild;
  SgBlackWhite toPlay = SG_BLACK;
  int length = 0;
  double tau = 1.0;
  float coef = 1.0f / 50;
  m_policyAllocator->Clear();
  m_nodeAllocator->Clear();
  m_nodeSequence.clear();
  m_nodeSequence.push_back(&root);
  ClearBoard();

  while (length < m_maxGameLength) {
    selectedChild.clear();
    float* policy_ = m_policyAllocator->CreateOneBlock();
    double maxTime = std::numeric_limits<double>::max();
    GoMove move = GO_NULLMOVE;
    m_mpiSynchronizer->SynchronizeMove(move);
    ++m_statistics.m_nuGenMove;
    move = DoSearch(toPlay, maxTime, tau, &selectedChild, policy_);
    if (move == UCT_RESIGN || (move == GO_PASS && m_nodeSequence.back()->Move() == GO_PASS)) {
      if (move == UCT_RESIGN) {
        m_game.AddResignNode(toPlay);
        SgDebug() << "Player " << toPlay << " resigned" << '\n';
      }
      break;
    }

    if (uploadToServer) {
      SgDebug() << SgBW(toPlay) << "[" << GoPointUtil::ToString2(move) << "," << move << "] ";
      if (length > 0 && length % 20 == 0)
        SgDebug() << "\n";
    }

    UctNode* node = m_nodeAllocator->CreateOne(move, m_nodeSequence.back());
    m_nodeSequence.back()->SetFirstChild(node);
    m_nodeSequence.back()->SetNumChildren(1);
    node->SetPolicy(policy_);
    node->CopyNonPointerData(*selectedChild[0]);
    m_nodeSequence.push_back(node);

    m_game.AddMove(move, toPlay);
    UpdateSubscriber();

    toPlay = SgOppBW(toPlay);
    ++length;
    if (length >= 30) {
      tau = pow(M_E, (30 - length) * coef);
    }
#if !defined(NDEBUG) && defined(LOG_SELFPLAY_STEPS)
    if (length % 10 == 0 || length > 350)
      SgDebug() << "UctDeepPlayer::SelfPlayOneGame " << gameID << ", in-game step:" << length << '\n';
#endif
  }

  if (uploadToServer)
    UploadTFRecordToServer(&root, path);
  else
    LogSelfPlayGame(&root, path, gameID);
}

void UctDeepPlayer::UpdateGameInfo(DlCheckPoint::CheckPointInfo& ckInfo, const std::string& gameName) {
  m_game.UpdateGameName(gameName);
  m_game.UpdatePlayerName(SG_BLACK, APP_NAME "-black-" + ckInfo.sha1);
  m_game.UpdatePlayerName(SG_WHITE, APP_NAME "-white-" + ckInfo.sha1);
}

void UctDeepPlayer::TryInitNeuralNetwork() {
  if (!m_bestCheckPoint.name.empty())
    return;

  if (!DlCheckPoint::DownloadMetagraph())
    return;

  m_game.UpdateGameName("self-play games-multi-threaded");
  m_game.UpdatePlayerName(SG_BLACK, APP_NAME "-black-mt");
  m_game.UpdatePlayerName(SG_WHITE, APP_NAME "-white-mt");

  bool gotCheckPoint =
      DlCheckPoint::DownloadCheckPoint(DlConfig::GetInstance().get_bestcheckpointinfo_url(), m_bestCheckPoint);
  if (!gotCheckPoint) {
    gotCheckPoint = DlCheckPoint::CheckDefaultCheckPoint(m_bestCheckPoint);
  }
  if (gotCheckPoint)
    m_search.UpdateCheckPoint(m_bestCheckPoint.name);
}
std::string UctDeepPlayer::SelfPlayAsServer(int roundID) {
  m_game.UpdateGameName("self-play games-multi-threaded");
  m_game.UpdatePlayerName(SG_BLACK, APP_NAME "-black-mt");
  m_game.UpdatePlayerName(SG_WHITE, APP_NAME "-white-mt");

  std::string subDir = "multi-thread_boardsize-" + UnrealGo::StringUtil::Int2Str(m_game.Board().Size(), 2) + "_round-" +
      UnrealGo::StringUtil::Int2Str(roundID, 8);
  std::string trainDataPath = DlConfig::GetInstance().get_traindata_dir() + "/" + subDir;
  UnrealGo::CreatePath(0777, trainDataPath, "/");

  for (int gameID = 0; gameID < SELF_PLAY_GAMES && !m_aborted; gameID++) {
    SelfPlayOneGame(trainDataPath, gameID);
  }

  return trainDataPath;
}
void UctDeepPlayer::SelfPlayAsClient() {
  if (!DlCheckPoint::DownloadMetagraph())
    return;

  m_game.UpdateGameName("self-play games-multi-threaded");
  m_game.UpdatePlayerName(SG_BLACK, APP_NAME "-black-mt");
  m_game.UpdatePlayerName(SG_WHITE, APP_NAME "-white-mt");

  const std::string path = UnrealGo::GetCWD();
  int gameID = 0;

  while (!m_aborted) {
    bool gotCheckPoint =
        DlCheckPoint::DownloadCheckPoint(DlConfig::GetInstance().get_bestcheckpointinfo_url(), m_bestCheckPoint);
    if (!gotCheckPoint) {
      gotCheckPoint = DlCheckPoint::CheckDefaultCheckPoint(m_bestCheckPoint);
    }
    if (gotCheckPoint)
      m_search.UpdateCheckPoint(m_bestCheckPoint.name);

    if (!m_bestCheckPoint.name.empty())
      SelfPlayOneGame(path, gameID++, true);
    else {
      std::cerr << "No checkpoint available now, waiting..." << std::endl;
      sleep(20);
    }
  }
}

GoPoint UctDeepPlayer::GenMove(const SgTimeRecord& timeRecord, SgBlackWhite toPlay) {
  ++m_statistics.m_nuGenMove;
  const GoBoard& bd = Board();
  GoMove move = GO_NULLMOVE;
  if (m_forcedOpeningMoves) {
    move = GoUctUtil::GenForcedOpeningMove(bd);
    if (move != GO_NULLMOVE)
      SgDebug() << "DeepUctPlayer: Forced opening move\n";
  }
  if (move == GO_NULLMOVE && GoBoardUtil::TrompTaylorPassWins(bd, toPlay)) {
    move = GO_PASS;
    SgDebug() << "DeepUctPlayer: Pass wins (By Tromp-Taylor Score)\n";
  }
  if (move == GO_NULLMOVE) {
    double maxTime;
    if (m_ignoreClock)
      maxTime = std::numeric_limits<double>::max();
    else
      maxTime = m_timeControl.TimeForCurrentMove(timeRecord, !m_writeDebugOutput);

    float tau = 0.001;
    std::vector<UctNode*> bestchild;
    move = DoSearch(toPlay, maxTime, tau, &bestchild, 0);
    m_statistics.m_gamesPerSecond.Add(m_search.Statistics().searches_per_second);
  }
  return move;
}

void UctDeepPlayer::FindInitTree(UctSearchTree& initTree, SgBlackWhite toPlay, double maxTime) {
  Board().SetToPlay(toPlay);
  std::vector<GoPoint> sequence;
  if (!((GoUctSearch&)m_search).BoardHistory().SequenceToCurrent(Board(), sequence)) {
    SgDebug() << "DeepTrainer: No nodes to reuse\n";
    return;
  }
  UctTreeUtil::ExtractSubtree(m_search.Tree(), initTree, sequence, true,
                              maxTime, m_search.PruneMinCount());
#ifdef CHECKTREECONSISTENCY
  bool initTreeConsistent = UctTreeUtil::CheckTreeConsistency(initTree, initTree.Root());
  bool treeConsistent = UctTreeUtil::CheckTreeConsistency(m_search.Tree(), m_search.Tree().Root());
#endif
  const size_t initTreeNodes = initTree.NuNodes();
  const size_t oldTreeNodes = m_search.Tree().NuNodes();
  if (oldTreeNodes > 1 && initTreeNodes >= 1) {
    const float reuse = float(initTreeNodes) / float(oldTreeNodes);
    if (m_logReuse) {
      auto reusePercent = static_cast<int>(100 * reuse);
      SgDebug() << "DeepPlayer: Reusing " << initTreeNodes
                << " nodes (" << reusePercent << "%)\n";
    }
    m_statistics.m_reuse.Add(reuse);
  } else {
    SgDebug() << "UctDeepPlayer: Subtree to reuse has 0 nodes\n";
    m_statistics.m_reuse.Add(0.f);
  }
  if (initTree.Root().HasChildren()) {
    for (UctChildNodeIterator it(initTree, initTree.Root()); it; ++it)
      if (!Board().IsLegal((*it).Move())) {
        SgWarning() << "UctDeepPlayer: illegal move in root child of init tree\n";
        initTree.Clear();
        DBG_ASSERT(false);
      }
  }
}

void UctDeepPlayer::SyncState(GoMove move, SgBlackWhite color) {
  m_search.SyncStateAgainst(move, color);
  m_game.AddMove(move, color);
  UpdateSubscriber();
}

GoPoint UctDeepPlayer::SearchAgainstAndSync(SgBlackWhite toPlay, double maxTime, double tau,
                                            std::vector<UctNode*>* child, float* policy, bool syncState) {
  GoMove toMove = DoSearch(toPlay, maxTime, tau, child, policy, syncState);
  if (toMove != UCT_RESIGN)
    SyncState(toMove, toPlay);
  return toMove;
}

GoPoint UctDeepPlayer::DoSearch(SgBlackWhite toPlay, double maxTime, double tau, std::vector<UctNode*>* bestChild,
                                float* policy_, bool syncState) {
  UctSearchTree* initTree = nullptr;
  SgTimer timer;
  double timeInitTree = 0;
  if (m_reuseSubtree) {
    initTree = &m_search.GetTempTree();
    timeInitTree = -timer.GetTime();
    FindInitTree(*initTree, toPlay, maxTime);
    timeInitTree += timer.GetTime();
  }
#ifdef CHECKTREECONSISTENCY
  DBG_ASSERT(UctTreeUtil::CheckTreeConsistency(*initTree, initTree->Root()));
#endif
  std::vector<GoMove> rootFilter;
  maxTime -= timer.GetTime();
  ((GoUctGlobalSearchType&)m_search).SetToPlay(toPlay);
  std::vector<GoPoint> sequence;
  UctEarlyAbortParam earlyAbort;
  earlyAbort.abort_threshold = m_sureWinThreshold;
  earlyAbort.min_searches_to_abort = m_resignMinGames;
  earlyAbort.reduction_factor = 3;
  UctValueType value = m_search.StartDeepUCTSearchThread(MAX_SEARCH_ITERATIONS, maxTime, sequence,
                                                         bestChild,
                                                         policy_,
                                                         tau,
                                                         rootFilter,
                                                         initTree,
                                                         &earlyAbort,
                                                         syncState);
  bool wasEarlyAbort = m_search.WasEarlyAbort();
  UctValueType rootMoveCount = m_search.Tree().Root().MoveCount();
  m_mpiSynchronizer->SynchronizeSearchStatus(value, wasEarlyAbort, rootMoveCount);

  if (m_writeDebugOutput) {
    std::ostringstream out;
    m_search.WriteStatistics(out);
    out << SgWriteLabel("Value") << std::fixed << std::setprecision(2)
        << value << '\n' << SgWriteLabel("Sequence")
        << SgWritePointList(sequence, "", false);
    if (m_reuseSubtree)
      out << SgWriteLabel("TimeInitTree") << std::fixed
          << std::setprecision(2) << timeInitTree << '\n';
    SgDebug() << out.str();
  }

  if (value < m_resignThreshold && rootMoveCount > m_resignMinGames)
    return UCT_RESIGN;

  GoPoint move;
  if (sequence.empty())
    move = GO_PASS;
  else {
    move = *(sequence.begin());
  }

  m_mpiSynchronizer->SynchronizeMove(move);
  return move;
}

void checkNodeSequence(UctNode* root, std::vector<UctNode*>& sequence) {
  SuppressUnused(root);
  DBG_ASSERT(root == *sequence.begin());
  DBG_ASSERT(root->GetColor() == SG_WHITE);
  for (size_t i = 1; i < sequence.size(); ++i) {
    DBG_ASSERT(sequence[i]->GetColor() == SgOppBW(sequence[i - 1]->GetColor()));
    DBG_ASSERT(sequence[i]->Parent() == sequence[i - 1]);
    DBG_ASSERT(sequence[i - 1]->FirstChild() == sequence[i]);
  }
}

void UctDeepPlayer::WriteTFRecord(UctNode* root, const std::string& fileName) {
  checkNodeSequence(root, m_nodeSequence);
  UctThreadState& state = m_search.ThreadState(0);
  DBG_ASSERT(state.Board().GetHashCode() == Board().GetHashCode());
  DBG_ASSERT(m_game.Board().GetHashCode() == Board().GetHashCode());
  state.WinTheGame();
  char m_feature[NUM_MAPS][GO_MAX_SIZE][GO_MAX_SIZE];
  UctNode* node = m_nodeSequence.back();
  while (node->Move() == GO_PASS) {
    node = node->Parent();
    state.TakeBackInTree(1);
  }

  tensorflow::io::DlTFRecordWriter tfRecordWriter(fileName);
  UctValueType score = state.FinalScore();
  SgBlackWhite color = node->GetColor();
  bool win = (score > 0 && color == SG_BLACK) || (score < 0 && color == SG_WHITE);
  float reward = win ? 1 : -1;
  while (state.LastMove() != GO_NULLMOVE) {
    state.CollectFeatures(m_feature, NUM_MAPS);
    tfRecordWriter.WriteExample((char*)m_feature, sizeof(m_feature),
                                node->Policy(), (size_t)GO_MAX_MOVES,
                                reward);
    state.TakeBackInTree(1);
    node = node->Parent();
    reward *= -1;
  }
}

void UctDeepPlayer::WriteSgf(UctNode* root, const std::string& path) {
  time_t timeValue = time(nullptr);
  struct tm* timeStruct = localtime(&timeValue);
  char timeBuffer[128];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d%H%M%S", timeStruct);
  std::ostringstream stream;
  stream << "selfplay_" << timeBuffer << ".sgf";
  std::string outFileName = stream.str();
  std::ofstream out(path + "/" + outFileName);
  SgGameWriter writer(out);
  writer.WriteGame(m_game.Root(), true, 0, 1, m_game.Board().Size());

  stream.str("");
  stream.clear();
  stream << "selfplay_" << timeBuffer << ".steps";
  std::ofstream fstep(path + "/" + stream.str());
  auto* node = const_cast<UctNode*> (m_nodeSequence.back());
  char colors[2] = {'B', 'W'};
  while (node != root) {
    std::string str = GoPointUtil::ToString(node->Move());
    fstep.put(colors[node->GetColor()]);
    fstep.put(' ');
    fstep.write(str.c_str(), str.length());
    fstep.put('\n');
    node = node->Parent();
  }
  fstep.close();
}

void UctDeepPlayer::LogSelfPlayGame(UctNode* root, const std::string& path,
                                    int gameID)
{
  std::string
      fileName = path + "/sp_" + SgTime::Time2String() + "_" + UnrealGo::StringUtil::Int2Str(gameID, 8) + ".tfrecords";
  WriteTFRecord(root, fileName);

  if (true) {
    WriteSgf(root, path);
  }
}

void UctDeepPlayer::UploadTFRecordToServer(UctNode* root, const std::string& path) {

  std::string uuid;
  SgUUID::generateUUID(uuid);
  std::string fileName = uuid + ".tfrecords";
  std::string fullPath = UnrealGo::GetFullPathStr(path, fileName);
  WriteTFRecord(root, fileName);
  UnrealGo::MinioStub::Upload("train-data", m_bestCheckPoint.sha1 + "/" + fileName, fullPath);
  boost::filesystem::remove(fullPath);
}

const GoUctSearch& UctDeepPlayer::Search() const {
  return m_search;
}

GoUctSearch& UctDeepPlayer::Search() {
  return m_search;
}

UctDeepPlayer::Statistics::Statistics() {
  Clear();
}

void UctDeepPlayer::Statistics::Clear() {
  m_nuGenMove = 0;
  m_gamesPerSecond.Clear();
  m_reuse.Clear();
}

void UctDeepPlayer::Statistics::Write(std::ostream& out) const {
  out << SgWriteLabel("NuGenMove") << m_nuGenMove << '\n'
      << SgWriteLabel("GamesPerSec");
  m_gamesPerSecond.Write(out);
  out << '\n'
      << SgWriteLabel("Reuse");
  m_reuse.Write(out);
  out << '\n';
}

void UctDeepPlayer::SetLogReuse(bool reuse) {
  m_logReuse = reuse;
}

void UctDeepPlayer::Abort() {
  m_aborted = true;
}