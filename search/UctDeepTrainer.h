
#ifndef SG_UCT_DEEPTRAINER_H
#define SG_UCT_DEEPTRAINER_H

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <zmq.hpp>
#include "GoGame.h"
#include "GoPlayer.h"
#include "GoTimeControl.h"
#include "gouct/GoUctGlobalSearch.h"
#include "gouct/GoUctSearchObject.h"
#include "SgTimeControl.h"
#include "UctSearch.h"
#include "board/GoBlackWhite.h"
#include "lib/SgRandom.h"
#include "funcapproximator/DlTFRecordWriter.h"
#include "UctDeepPlayer.h"

enum EvalMode {
  EVAL_FROM_CLIENT,
  EVAL_FROM_SERVER,
  EVAL_FROM_SERVER_MSG
};

class UctDeepTrainer {
 public:
  UctDeepTrainer(const GoGame &game, GoRules &rules);
  ~UctDeepTrainer();
  void StartInitCK();
  void SvrInitCheckPointsInfo();
  void StartTrainEvalPipeLine(bool withSelfplay = false, EvalMode evalMode = EVAL_FROM_CLIENT);
  void StartSelfPlay(bool uploadSgf = true);
  void SelfPlayGamesServerMode();
  void SelfPlayGamesClientMode();
  void SvrEvalLoop();
  void EvalPlayAsServer(const std::string &checkpoint);
  void EvalPlayAsClient();
  bool EvalPlayNetworkModel(int gameID);
  bool BoardConsistent();
  bool ShouldStop(SgBlackWhite trainColor, GoMove trainerMove, GoMove oppMove, int steps, bool &trainerWin);
  void SaveSgf(GoGame *game);
  bool IsSelfPlayRunning();
  class SelfPlayThread {
   public:
    explicit SelfPlayThread(UctDeepTrainer &trainer, bool initCK = false);
    ~SelfPlayThread();
    void NotifyExecuteCommand(bool upload = false);
    bool IsRunning();

   private:
    class Function {
     public:
      explicit Function(SelfPlayThread &TrainThread);
      void operator()();

     private:
      SelfPlayThread &m_selfplayThread;
    };
    friend class SelfPlayThread::Function;
    bool m_initCheckPoints;
    bool m_quit;
    UctDeepTrainer &m_trainer;
    bool m_uploadSgf;
    bool m_running;
    boost::barrier m_threadReadyBarrier;
    boost::mutex m_wait_command_mutex;
    boost::condition m_wait_command_cv;
    boost::mutex::scoped_lock m_commandReceivedLock;

    boost::thread m_thread;
    void operator()();
  };
  class EvaluateThread {
   public:
    explicit EvaluateThread(UctDeepTrainer &trainer);
    ~EvaluateThread();
    void HandleMsg(Msg &msg);
    void NotifyExecuteCommand();
    void NotifyExecuteCommand(Msg &msg);
    bool IsEvaluating();
    void ClearMsgQ();

   private:
    class Function {
     public:
      explicit Function(EvaluateThread &TrainThread);
      void operator()();

     private:
      EvaluateThread &m_evalThread;
    };
    friend class EvaluateThread::Function;
    std::queue<Msg> m_queue;
    bool m_quit;
    bool m_evaluating;
    UctDeepTrainer &m_trainer;
    boost::barrier m_threadReadyBarrier;
    boost::mutex m_waitCommandMutex;
    boost::condition m_waitCommand;
    boost::mutex::scoped_lock m_commandReceivedLock;

    boost::thread m_thread;
    void operator()();
  };
  class NetworkThread {
   public:
    explicit NetworkThread(UctDeepTrainer &trainer);
    ~NetworkThread();
            void NotifyExecuteCommand();

   private:
    class Function {
     public:
      explicit Function(NetworkThread &TrainThread);
      void operator()();

     private:
      NetworkThread &m_netThread;
    };
    friend class NetworkThread::Function;
    bool m_quit;
    UctDeepTrainer &m_trainer;
    boost::barrier m_threadReadyBarrier;
    boost::mutex m_waitCommandMutex;
    boost::condition m_waitCommand;
    boost::mutex::scoped_lock m_commandReceivedLock;

    boost::thread m_thread;
    void operator()();
  };

 private:
  bool m_quit;
  bool m_selfplay;
  EvalMode m_evalMode;
  boost::shared_ptr<SelfPlayThread> m_selfplayThread;
  boost::shared_ptr<NetworkThread> m_networkThread;
  boost::shared_ptr<EvaluateThread> m_evalThread;
  GoGame m_trainGame;
  GoGame m_oppGame;
  UctDeepPlayer m_trainer;
  UctDeepPlayer m_opponent;
  SgRandom m_random;
  zmq::context_t m_ctx;
  zmq::socket_t m_send_socket; // training data send socket
  zmq::socket_t m_receive_socket; // checkpoint receive socket
  zmq::socket_t m_evalres_socket;
  GoRules &m_engineRules;
  int m_roundID;
  std::string m_evalPath;
};

#endif // SG_UCT_DEEPTRAINER_H
