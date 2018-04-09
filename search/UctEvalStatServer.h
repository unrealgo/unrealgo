
#ifndef SG_UCT_EVALSTATSERVER_H
#define SG_UCT_EVALSTATSERVER_H

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <zmq.hpp>

class UctEvalStatServer {
 public:
  UctEvalStatServer();
  ~UctEvalStatServer() = default;
  void ProcessRequest();
  void ProcessUpdate();
  void Start();

  class StatsThread {
   public:
    explicit StatsThread(UctEvalStatServer &trainer);
    ~StatsThread();
    void NotifyExecuteCommand();

   private:
    class Function {
     public:
      explicit Function(StatsThread &TrainThread);
      void operator()();

     private:
      StatsThread &m_StatsThread;
    };
    friend class StatsThread::Function;
    UctEvalStatServer &m_server;
    boost::barrier m_threadReadyBarrier;
    boost::mutex m_waitCommandMutex;
    boost::condition m_waitCommand;
    boost::mutex::scoped_lock m_commandReceivedLock;

    boost::thread m_thread;
    void operator()();
  };

  class CPUpdateThread {
   public:
    explicit CPUpdateThread(UctEvalStatServer &trainer);
    ~CPUpdateThread();
    void NotifyExecuteCommand();

   private:
    class Function {
     public:
      explicit Function(CPUpdateThread &TrainThread);
      void operator()();

     private:
      CPUpdateThread &m_updateThread;
    };
    friend class CPUpdateThread::Function;
    bool m_quit;
   public:
    void setQuit(bool quit);

   private:
    UctEvalStatServer &stat_server;
    boost::barrier thread_ready_barrier;
    boost::mutex wait_command_mutex;
    boost::condition wait_command;

    boost::thread m_thread;
    void operator()();
  };

 private:
  boost::shared_ptr<StatsThread> m_statsThread;
  boost::shared_ptr<CPUpdateThread> m_updateThread;
  bool m_quit;
 public:
  void setQuit(bool quit);

 private:
  zmq::context_t m_ctx;
  zmq::socket_t m_socket;
};

#endif
