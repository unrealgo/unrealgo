
#include "platform/SgSystem.h"
#include <GoUctUtil.h>
#include <GoUctPlayoutPolicy.h>
#include "UctDeepTrainer.h"
#include "lib/StringUtil.h"
#include "SgGameWriter.h"
#include "UctTreeUtil.h"
#include "lib/FileUtil.h"
#include "msg/ZmqUtil.h"
#include "network/AtomZIP.h"
#include "network/AtomHash.h"

static const int CMD_TRAIN = 0;
static const int CMD_UPDATE_CKP = 1;
static const int CMD_GET_CKP = 2;
static const int CMD_GET_METAGRAPH = 3;
static const int CMD_EXIT = 4;

static void Notify(boost::mutex &mutex, boost::condition &cv) {
  boost::mutex::scoped_lock lock(mutex);
  cv.notify_all();
}

UctDeepTrainer::SelfPlayThread::Function::Function(SelfPlayThread &TrainThread)
    : m_selfplayThread(TrainThread) {}

void UctDeepTrainer::SelfPlayThread::Function::operator()() {
  m_selfplayThread();
}

UctDeepTrainer::SelfPlayThread::SelfPlayThread(UctDeepTrainer &trainer, bool initCK) :
    m_initCheckPoints(initCK),
    m_quit(false),
    m_trainer(trainer),
    m_uploadSgf(false),
    m_threadReadyBarrier(2),
    m_thread(Function(*this)) {
  m_threadReadyBarrier.wait();
}

void UctDeepTrainer::SelfPlayThread::operator()() {
  boost::mutex::scoped_lock lock(m_wait_command_mutex);
  m_threadReadyBarrier.wait();

  if (m_initCheckPoints) {
    m_trainer.SvrInitCheckPointsInfo();
  } else {
    while (true) {
      m_running = false;
      m_wait_command_cv.wait(lock);
      if (m_quit)
        break;

      m_running = true;
      if (m_uploadSgf)
        m_trainer.SelfPlayGamesClientMode();
      else
        m_trainer.SelfPlayGamesServerMode();
    }
  }
}

void UctDeepTrainer::SelfPlayThread::NotifyExecuteCommand(bool upload) {
  m_uploadSgf = upload;
  Notify(m_wait_command_mutex, m_wait_command_cv);
}

bool UctDeepTrainer::SelfPlayThread::IsRunning() {
  return m_running;
}

UctDeepTrainer::SelfPlayThread::~SelfPlayThread() {
  m_quit = true;
  NotifyExecuteCommand();
  m_thread.join();
}

UctDeepTrainer::EvaluateThread::EvaluateThread(UctDeepTrainer &trainer) :
    m_quit(false),
    m_trainer(trainer),
    m_threadReadyBarrier(2),
    m_thread(Function(*this)) {
  m_threadReadyBarrier.wait();
}

UctDeepTrainer::EvaluateThread::Function::Function(EvaluateThread &TrainThread)
    : m_evalThread(TrainThread) {}

void UctDeepTrainer::EvaluateThread::Function::operator()() {
  m_evalThread();
}

void UctDeepTrainer::EvaluateThread::ClearMsgQ() {
  std::queue<Msg> empty;
  std::swap(m_queue, empty);
}

void UctDeepTrainer::EvaluateThread::operator()() {
  boost::mutex::scoped_lock lock(m_waitCommandMutex);
  m_threadReadyBarrier.wait();

  if (m_trainer.m_evalMode == EVAL_FROM_SERVER_MSG) {
    while (true) {
      m_evaluating = false;
      m_waitCommand.wait(lock);
      if (m_quit)
        break;
      if (!m_queue.empty()) {
        Msg &msg = m_queue.back();
        switch (msg.type) {
          case CMD_UPDATE_CKP: {
            m_evaluating = true;
            std::string checkpoint = *(const std::string *) msg.data;
            delete (std::string *) msg.data;
            ClearMsgQ();
            m_trainer.EvalPlayAsServer(checkpoint);
            break;
          }
          default:break;
        }
      }
    }
  } else if (m_trainer.m_evalMode == EVAL_FROM_SERVER) {
    m_waitCommand.wait(lock);
    m_trainer.SvrEvalLoop();
  } else if (m_trainer.m_evalMode == EVAL_FROM_CLIENT) {
    m_waitCommand.wait(lock);
    m_trainer.EvalPlayAsClient();
  }
}

void UctDeepTrainer::EvaluateThread::HandleMsg(Msg &msg) {
  m_queue.push(msg);
}

void UctDeepTrainer::EvaluateThread::NotifyExecuteCommand() {
  Notify(m_waitCommandMutex, m_waitCommand);
}

void UctDeepTrainer::EvaluateThread::NotifyExecuteCommand(Msg &msg) {
  std::queue<Msg> empty;
  std::swap(m_queue, empty);
  HandleMsg(msg);
  NotifyExecuteCommand();
}

bool UctDeepTrainer::EvaluateThread::IsEvaluating() {
  return m_evaluating;
}

UctDeepTrainer::EvaluateThread::~EvaluateThread() {
  m_quit = true;
  NotifyExecuteCommand();
  m_thread.join();
}

UctDeepTrainer::NetworkThread::NetworkThread(UctDeepTrainer &trainer) :
    m_quit(false),
    m_trainer(trainer),
    m_threadReadyBarrier(2),
    m_thread(Function(*this)) {
  m_threadReadyBarrier.wait();
}

UctDeepTrainer::NetworkThread::Function::Function(NetworkThread &TrainThread)
    : m_netThread(TrainThread) {}

void UctDeepTrainer::NetworkThread::Function::operator()() {
  m_netThread();
}

void UctDeepTrainer::NetworkThread::operator()() {
  m_trainer.m_receive_socket.bind(DlConfig::GetInstance().get_deeptrainerlisten_socket());

  boost::mutex::scoped_lock lock(m_waitCommandMutex);
  m_threadReadyBarrier.wait();
  UnrealGo::Command command;
  zmq::message_t reply;
  memcpy(reply.data(), "ACK", 3);

  std::stack<std::string> checkpoints;
  while (true) {
    if (m_quit)
      break;
    ZmqUtil::receive_command(m_trainer.m_receive_socket, command, reply);
    if (command.type() == CMD_UPDATE_CKP) {
      std::cout << "new checkpoint received " << command.data() << std::endl;
      Msg msg(CMD_UPDATE_CKP, new std::string(command.data()));
      if (m_trainer.IsSelfPlayRunning()) {
        std::cout << "ignore checkpoint" << std::endl;
        m_trainer.m_evalThread->HandleMsg(msg);
      } else {
        std::cout << "start evaluating..." << std::endl;
        m_trainer.m_evalThread->NotifyExecuteCommand(msg);
      }
    }
  }
}

void UctDeepTrainer::NetworkThread::NotifyExecuteCommand() {
  Notify(m_waitCommandMutex, m_waitCommand);
}

UctDeepTrainer::NetworkThread::~NetworkThread() {
  m_quit = true;
  NotifyExecuteCommand();
  m_thread.join();
}

UctDeepTrainer::UctDeepTrainer(const GoGame &game, GoRules &rules) :
    m_quit(false),
    m_selfplay(false),
    m_evalMode(EVAL_FROM_CLIENT),
    m_trainGame(game.Board().Size()),
    m_oppGame(game.Board().Size()),
    m_trainer(m_trainGame, rules),
    m_opponent(m_oppGame, rules),
    m_ctx(1),
    m_send_socket(m_ctx, ZMQ_REQ),
    m_receive_socket(m_ctx, ZMQ_REP),
    m_evalres_socket(m_ctx, ZMQ_REQ),
    m_engineRules(rules),
    m_roundID(0),
    m_evalPath("eval") {

  m_evalres_socket.connect(DlConfig::GetInstance().get_evalstatconnect_socket());
  m_send_socket.connect(DlConfig::GetInstance().get("deeptrainertraindataconsocket"));

  UnrealGo::CreatePath(0777, m_evalPath, "/");
}

UctDeepTrainer::~UctDeepTrainer() {
  m_trainer.Abort();
  m_opponent.Abort();
}

void UctDeepTrainer::StartTrainEvalPipeLine(bool withSelfplay, EvalMode evalMode) {
  m_selfplay = withSelfplay;
  m_evalMode = evalMode;
  if (m_selfplay && m_selfplayThread.get() == nullptr)
    m_selfplayThread.reset(new SelfPlayThread(*this));
  if (m_evalMode == EVAL_FROM_SERVER_MSG && m_networkThread.get() == nullptr)
    m_networkThread.reset(new NetworkThread(*this));
  m_evalThread.reset(new EvaluateThread(*this));

  if (m_selfplay)
    m_selfplayThread->NotifyExecuteCommand();
  else
    m_evalThread->NotifyExecuteCommand();
  SgDebug() << "DeepTrainer: train pipeline started \n";
}

void UctDeepTrainer::StartSelfPlay(bool uploadSgf) {
  m_selfplay = true;
  m_selfplayThread.reset(new SelfPlayThread(*this));
  m_selfplayThread->NotifyExecuteCommand(uploadSgf);
}

bool UctDeepTrainer::IsSelfPlayRunning() {
  return m_selfplay && m_selfplayThread->IsRunning();
}

void UctDeepTrainer::StartInitCK() {
  m_selfplay = true;
  m_selfplayThread.reset(new SelfPlayThread(*this, true));
  m_selfplayThread->NotifyExecuteCommand(true);
}
void UctDeepTrainer::SvrInitCheckPointsInfo() {
  std::string bestcheckpoint = DlConfig::GetInstance().get_bestcheckpoint_fullpath();
  DlCheckPoint::WriteBestCheckpointInfo(bestcheckpoint, false);
  DlCheckPoint::WriteLatestCheckpointInfo(bestcheckpoint, false);
  DlCheckPoint::InitMetagraph();
}

void UctDeepTrainer::SvrEvalLoop() {
  std::string dir = UnrealGo::GetFullPathStr(DlConfig::GetInstance().get_minio_path(),
                                             DlConfig::GetInstance().get_checkdata_subpath());
  int cnt = 0;
  while (cnt < 10000000) {
    std::string checkpointToEval = DlCheckPoint::getLatestCheckPointPrefix();
    if (checkpointToEval[0] != '/') {
      checkpointToEval = UnrealGo::GetFullPathStr(dir, checkpointToEval);
    }
    if (checkpointToEval != m_opponent.Search().getCheckPoint()) {
      std::cout << "evaluating new checkpoint" << std::endl;
      EvalPlayAsServer(checkpointToEval);
    } else {
      sleep(60);
    }
    ++cnt;
  }
}

bool UctDeepTrainer::EvalPlayNetworkModel(int gameID) {
  SuppressUnused(gameID);
  SgBlackWhite trainColor = m_random.Int(2);
  SgBlackWhite oppColor = SgOpp(trainColor);
  m_trainer.Search().PrepareGamePlay();
  m_opponent.Search().PrepareGamePlay();
  double tau = 0.0001;
  GoMove oppMove = GO_NULLMOVE;
  GoMove trainerMove;
  bool trainerWin;
  double maxTime = std::numeric_limits<double>::max();
  m_trainer.ClearBoard();
  m_opponent.ClearBoard();

  if (trainColor == SG_WHITE)
  {
    oppMove = m_opponent.SearchAgainstAndSync(oppColor, maxTime, tau, 0, 0, false);
    m_trainer.SyncState(oppMove, oppColor);
  }
  m_trainer.SetLogReuse(false);
  m_opponent.SetLogReuse(false);

  int steps = 0;

  while (true) {
    maxTime = std::numeric_limits<double>::max();
    DBG_ASSERT(BoardConsistent());
    trainerMove =
        m_trainer.SearchAgainstAndSync(trainColor, maxTime, tau, 0, 0, false);
    if (ShouldStop(trainColor, trainerMove, oppMove, steps, trainerWin))
      break;
    m_opponent.SyncState(trainerMove, trainColor);

    oppMove = m_opponent.SearchAgainstAndSync(oppColor, maxTime, tau, 0, 0, false);
    if (ShouldStop(trainColor, trainerMove, oppMove, steps, trainerWin))
      break;
    m_trainer.SyncState(oppMove, oppColor);

    ++steps;
#ifndef NDEBUG
    if (steps % 30 == 0 || steps > 350)
      SgDebug() << "UctDeepTrainer::EvalPlayNetworkModel " << gameID << ", in-game step:" << steps
                << '\n';
#endif
  }

  SaveSgf(&m_trainGame);

#ifndef NDEBUG
  std::string result = ((trainColor == SG_BLACK && trainerWin) || (trainColor == SG_WHITE && !trainerWin)) ? "B+"
                                                                                                           : "W+";
  SgDebug() << result << '\n';
#endif

  return trainerWin;
}

void UctDeepTrainer::EvalPlayAsClient() {
  if (!DlCheckPoint::DownloadMetagraph())
    return;

  DlCheckPoint::CheckPointInfo bestCheckPoint;
  DlCheckPoint::CheckPointInfo latestCheckPoint;
  int cnt = 0;
  while (!m_quit) {
    bool gotCheckPoint =
        DlCheckPoint::DownloadCheckPoint(DlConfig::GetInstance().get_bestcheckpointinfo_url(), bestCheckPoint);
    if (gotCheckPoint) {
      m_trainer.UpdateCheckPoint(bestCheckPoint);
      latestCheckPoint = bestCheckPoint;
      gotCheckPoint =
          DlCheckPoint::DownloadCheckPoint(DlConfig::GetInstance().get_latestcheckpointinfo_url(), latestCheckPoint);
      if (gotCheckPoint)
        m_opponent.UpdateCheckPoint(latestCheckPoint);

      if (bestCheckPoint.sha1 != latestCheckPoint.sha1) {
        SgDebug() << "evaluating " << bestCheckPoint.sha1 << " vs " << latestCheckPoint.sha1 << "\n";
        bool trainerWin = EvalPlayNetworkModel(cnt);
        std::string result = bestCheckPoint.sha1 + "/" + bestCheckPoint.name + ":" +
            latestCheckPoint.sha1 + "/" + latestCheckPoint.name +
            (trainerWin ? "1" : "0");
        ZmqUtil::sendData(m_evalres_socket, result);
        SgDebug() << "game result uploaded\n";
      } else if (!bestCheckPoint.sha1.empty()) {
        SgDebug() << "selfplaying checkpoint: " << bestCheckPoint.sha1 << "\n";
        m_trainer.UpdateGameInfo(bestCheckPoint, "SelfPlayGame");
        m_trainer.SelfPlayOneGame(UnrealGo::GetCWD(), 0, true);
      }
    } else {
      std::cerr << "No checkpoint available, waiting..." << std::endl;
      sleep(15);
    }
  }
}
void UctDeepTrainer::EvalPlayAsServer(const std::string &checkpoint) {
  float winCount = 0;
  int maxGames = 400;
  if (!checkpoint.empty()) {
    m_opponent.Search().UpdateCheckPoint(checkpoint);
  }

  for (int i = 0; i < maxGames; i++) {
    winCount += EvalPlayNetworkModel(i) ? 1.0f : 0.0f;
#ifndef NDEBUG
    SgDebug() << "DeepTrainer::Eval real-time winRate " << (winCount / (i + 1)) << '\n';
#endif
  }

  float winRate = winCount / maxGames;
  if (winRate < 0.45f) {
    m_trainer.Search().UpdateCheckPoint(checkpoint);
    DlCheckPoint::UpdateBestCheckPointList(checkpoint);
    DlCheckPoint::WriteBestCheckpointInfo(checkpoint);
  }

  if (m_selfplay)
    m_selfplayThread->NotifyExecuteCommand();
}

bool UctDeepTrainer::ShouldStop(SgBlackWhite trainColor, GoMove trainerMove, GoMove oppMove, int steps,
                                  bool &trainerWin) {
  if (trainerMove == UCT_RESIGN || oppMove == UCT_RESIGN || (trainerMove == GO_PASS && oppMove == GO_PASS) ||
      steps >= GO_MAX_NUM_MOVES) {
    UctValueType trainerScore = m_trainer.Search().EstimateGameScore();
    UctValueType oppScore = m_opponent.Search().EstimateGameScore();
    SuppressUnused(oppScore);
    if (trainerMove == oppMove) {
      DBG_ASSERT(trainerScore == oppScore);
    }
    trainerWin = (trainerScore > 0 && trainColor == SG_BLACK) || (trainerScore < 0 && trainColor == SG_WHITE);

    return true;
  }
  return false;
}

void UctDeepTrainer::SaveSgf(GoGame *game) {
  time_t timeValue = time(nullptr);
  struct tm *timeStruct = localtime(&timeValue);
  char timeBuffer[128];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d%H%M%S", timeStruct);
  std::ostringstream stream;
  stream << "evaluate_" << timeBuffer << ".sgf";
  std::string outFileName = m_evalPath + "/" + stream.str();
  std::ofstream out(outFileName);
  SgGameWriter writer(out);
  writer.WriteGame(game->Root(), true, 0, 1, game->Board().Size());
}

bool UctDeepTrainer::BoardConsistent() {
  return (m_trainer.Board().GetHashCode() == m_trainGame.Board().GetHashCode()) &&
      (m_opponent.Board().GetHashCode() == m_oppGame.Board().GetHashCode()) &&
      (m_opponent.Board().GetHashCode() == m_trainer.Board().GetHashCode());
}

void UctDeepTrainer::SelfPlayGamesServerMode() {
  if (m_selfplay) {
    m_trainer.SetLogReuse(true);
    std::string path = m_trainer.SelfPlayAsServer(m_roundID);
    if (path[0] != '/') {
      path = UnrealGo::GetCWD() + "/" + path;
    }

    std::cout << "Sending training data to server: " << path << "…" << std::endl;
    ZmqUtil::send_command(m_send_socket, CMD_TRAIN, path);
    std::cout << "Waiting for new checkpoint" << std::endl;

    if (!m_evalThread->IsEvaluating())
      m_evalThread->NotifyExecuteCommand();

    m_roundID++;
  }
}

void UctDeepTrainer::SelfPlayGamesClientMode() {
  m_trainer.SetLogReuse(false);
  m_trainer.SelfPlayAsClient();
}