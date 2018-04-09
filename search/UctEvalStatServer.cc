
#include <boost/lexical_cast.hpp>
#include "UctEvalStatServer.h"
#include "../lib/FileUtil.h"
#include "../lib/StringUtil.h"
#include "funcapproximator/DlCheckPoint.h"
#include "platform/SgDebug.h"

static void Notify(boost::mutex &aMutex, boost::condition &aCondition) {
  boost::mutex::scoped_lock lock(aMutex);
  aCondition.notify_all();
}

UctEvalStatServer::StatsThread::Function::Function(StatsThread &TrainThread)
    : m_StatsThread(TrainThread) {}

void UctEvalStatServer::StatsThread::Function::operator()() {
  m_StatsThread();
}

UctEvalStatServer::StatsThread::StatsThread(UctEvalStatServer &trainer) :
    m_server(trainer),
    m_threadReadyBarrier(2),
    m_thread(Function(*this)) {
  m_threadReadyBarrier.wait();
}

void UctEvalStatServer::StatsThread::operator()() {
  boost::mutex::scoped_lock lock(m_waitCommandMutex);
  m_threadReadyBarrier.wait();
  m_waitCommand.wait(lock);

  m_server.ProcessRequest();
}

void UctEvalStatServer::StatsThread::NotifyExecuteCommand() {
  Notify(m_waitCommandMutex, m_waitCommand);
}

UctEvalStatServer::StatsThread::~StatsThread() {
  NotifyExecuteCommand();
  m_thread.join();
}

UctEvalStatServer::UctEvalStatServer() :
    m_quit(false),
    m_ctx(1),
    m_socket(m_ctx, ZMQ_REP) {
}
void UctEvalStatServer::ProcessRequest() {
  m_socket.bind(DlConfig::GetInstance().get_evalstatslisten_socket());
  SgDebug() << "CheckPoint Evaluation Stat Sever Started\n";
  zmq::message_t reply;
  memcpy(reply.data(), "updated", 7);
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  // std::string evalResFileName = UnrealGo::GetFullPathStr(minio_path, DlConfig::GetInstance().get_checkeval_subpath());

  while (!m_quit) {
    zmq::message_t request;
    m_socket.recv(&request);

    if (request.size() > 0) {
      std::string msg((const char *) request.data(), request.size());
      std::vector<std::string> splits;
      UnrealGo::StringUtil::Split(msg, ":", splits);
      if (splits.size() >= 3 && splits[0] != splits[1]) {
        DlCheckPoint::CheckPointInfo best(splits[0], "/");
        DlCheckPoint::CheckPointInfo latest(splits[1], "/");

        int bestWinCnt = 0;
        int total = 0;
        std::vector<std::string> lines;
        std::string evalResFileName = UnrealGo::GetFullPathStr(minio_path, best.sha1 + "-" + latest.sha1);
        UnrealGo::ReadLines(evalResFileName, lines);
        if (lines.size() >= 2) {
          bestWinCnt = boost::lexical_cast<int>(lines[0]);
          total = boost::lexical_cast<int>(lines[1]);
        }
        if (splits[2] == "1")
          bestWinCnt++;
        total++;

        lines.clear();
        float winRate = (float) bestWinCnt * 1.0f / total;
        if (total >= 500) {
          if (winRate < 0.45f) {
            DlCheckPoint::UpdateBestCheckPointList(latest);
            DlCheckPoint::WriteBestCheckpointInfo(latest);
            // TODO delete evalResFileName file
          }
          DlCheckPoint::WriteLatestCheckPointInfo(); // write latest checkpoint info
        } else {
          lines.push_back(std::to_string(bestWinCnt));
          lines.push_back(std::to_string(total));
          UnrealGo::WriteLines(evalResFileName, lines);
        }

        /*if (total >= 500 && winRate < 0.45f) {
            DlCheckPoint::UpdateBestCheckPointList(latest);
            DlCheckPoint::WriteBestCheckpointInfo(latest);
            DlCheckPoint::WriteLatestCheckPointInfo();
        } else {
          lines.push_back(std::to_string(bestWinCnt));
          lines.push_back(std::to_string(total));
          UnrealGo::WriteLines(evalResFileName, lines);
        }*/
      }
    }

    m_socket.send(reply);
  }
}

void UctEvalStatServer::Start() {
  if (m_statsThread.get() == nullptr)
    m_statsThread.reset(new StatsThread(*this));
  if (m_updateThread.get() == nullptr)
    m_updateThread.reset(new CPUpdateThread(*this));
  m_statsThread->NotifyExecuteCommand();
  m_updateThread->NotifyExecuteCommand();
}

void UctEvalStatServer::setQuit(bool quit) {
  UctEvalStatServer::m_quit = quit;
}

UctEvalStatServer::CPUpdateThread::Function::Function(CPUpdateThread &TrainThread)
    : m_updateThread(TrainThread) {}

void UctEvalStatServer::CPUpdateThread::Function::operator()() {
  m_updateThread();
}

UctEvalStatServer::CPUpdateThread::CPUpdateThread(UctEvalStatServer &trainer) :
    m_quit(false),
    stat_server(trainer),
    thread_ready_barrier(2),
    m_thread(Function(*this)) {
  thread_ready_barrier.wait();
}

void UctEvalStatServer::CPUpdateThread::operator()() {
  boost::mutex::scoped_lock lock(wait_command_mutex);
  thread_ready_barrier.wait();
  wait_command.wait(lock);
  stat_server.ProcessUpdate();
}

void UctEvalStatServer::CPUpdateThread::NotifyExecuteCommand() {
  Notify(wait_command_mutex, wait_command);
}

UctEvalStatServer::CPUpdateThread::~CPUpdateThread() {
  NotifyExecuteCommand();
  m_thread.join();
}

void UctEvalStatServer::CPUpdateThread::setQuit(bool quit) {
  m_quit = quit;
}

void UctEvalStatServer::ProcessUpdate() {
  while (!m_quit) {
    std::string latestCK = UnrealGo::ExtractFileName(DlCheckPoint::getLatestCheckPointPrefix());
    DlCheckPoint::CheckPointInfo best;
    DlCheckPoint::CheckPointInfo latest;

    DlCheckPoint::GetBestCheckPointInfo(best);
    DlCheckPoint::GetLatestCheckPointInfo(latest);

    if ((best.sha1 == latest.sha1 || latest.sha1.empty()) && latestCK != latest.name) {
      DlCheckPoint::WriteLatestCheckpointInfo(latestCK);
    }
    if (best.sha1.empty() && !latestCK.empty())
      DlCheckPoint::WriteBestCheckpointInfo(latestCK);

    sleep(60);
  }
}