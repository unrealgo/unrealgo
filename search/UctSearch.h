
#ifndef SG_UCTSEARCH_H
#define SG_UCTSEARCH_H

#include <fstream>
#include <vector>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <queue>
#include "GoGame.h"
#include "UctBoardEvaluator.h"

#include "SgAdditiveKnowledge.h"
#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "platform/SgTimer.h"
#include "UctSearchTree.h"
#include "UctValue.h"
#include "MpiSynchronizer.h"
#include "lib/SgRandom.h"

#define USE_FASTLOG 1
#if USE_FASTLOG
#include "lib/SgFastLog.h"
#endif

#include "funcapproximator/DlConfig.h"

struct UctGameInfo {
  std::vector<UctValueType> m_eval;
  std::vector<GoMove> m_inTreeSequence;
  std::vector<std::vector<GoMove> > m_sequence;

  std::vector<bool> m_aborted;
  std::vector<const UctNode *> m_nodes;
  std::vector<std::vector<bool> > m_skipRaveUpdate;

  void Clear(std::size_t numberPlayouts);
  void Clear();
};

enum UctMoveSelect {
  SG_UCTMOVESELECT_VALUE,
  SG_UCTMOVESELECT_COUNT,
  SG_UCTMOVESELECT_BOUND,
  SG_UCTMOVESELECT_PUCT,
  SG_UCTMOVESELECT_ESTIMATE
};
enum EvalMsgType {
  MSG_EVAL,
  MSG_END_SEARCH,
  MSG_EXIT,
  MSG_UNKNOWN
};

struct EvalMsg {
  const size_t thread_id;
  EvalMsgType msg_type;
  bool thread_exited;
  bool state_ready;
  bool state_evaluated;
  boost::condition condition_variable;
  boost::mutex mutex;

  EvalMsg();
  explicit EvalMsg(size_t threadID, EvalMsgType type = MSG_EVAL);
  void WaitEvalFinish();
  void SetThreadExited(bool exited);
};

class UctThreadState {
 public:
  const size_t thread_id;
  EvalMsg eval_msg;
  bool search_initialised;
  bool tree_exceed_memory_limit;
  UctGameInfo game_info;
  boost::scoped_array<std::size_t> first_play;
  boost::scoped_array<std::size_t> first_play_opp;
  std::vector<UctMoveInfo> move_info;
  std::vector<GoMove> excluded_moves;
  int randomize_rave_cnt;
  int randomize_bias_cnt;

  explicit UctThreadState(unsigned int threadId, int moveRange = 0);
  virtual ~UctThreadState();

  virtual UctValueType Evaluate() = 0;
  virtual UctValueType FinalScore() = 0;
  virtual bool WinTheGame() = 0;
  virtual bool TrompTaylorPassWins() = 0;
  virtual void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures) = 0;
  virtual void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures, int maxSteps) = 0;

  virtual void Execute(GoMove move) = 0;
  virtual void Apply(GoMove move) = 0;
  virtual GoMove LastMove() = 0;
  virtual const GoBoard &Board() = 0;

  virtual void ExecutePlayout(GoMove move) = 0;
  virtual bool GenerateAllMoves(UctValueType count,
                                std::vector<UctMoveInfo> &moves,
                                UctProvenType &provenType) = 0;
  virtual GoMove GeneratePlayoutMove(bool &skipRaveUpdate) = 0;
  virtual void StartSearch() = 0;
  virtual void TakeBackInTree(std::size_t nuMoves) = 0;
  virtual void TakeBackPlayout(std::size_t nuMoves) = 0;

  virtual void GameStart();
  virtual void StartPlayouts();
  virtual void StartPlayout();
  virtual void EndPlayout();
  virtual void Clear();
};


class UctSearch;
class UctThreadStateFactory {
 public:
  virtual ~UctThreadStateFactory();
  virtual UctThreadState *Create(unsigned int threadId, const UctSearch &search) = 0;
};

struct EvalBuffer {
  char feature_buf[MAX_BATCHES][NUM_MAPS][GO_MAX_SIZE][GO_MAX_SIZE];
  UctValueType policy_out[MAX_BATCHES][GO_MAX_MOVES];
  UctValueType values_out[MAX_BATCHES];
};
enum MsgType {
  MSG_DEEP_UCT_SEARCH,
  MSG_SEARCH_LOOP,
  MSG_ESTIMATE_SCORE,
};
struct Msg {
  int type;
  void *data;
  Msg();
  Msg(int _type, void *_data);
  Msg(const Msg &right);
};

inline Msg::Msg(int _type, void *_data) {
  type = _type;
  data = _data;
}

inline Msg::Msg() {
  type = -1;
  data = nullptr;
}

inline Msg::Msg(const Msg &right) {
  type = right.type;
  data = right.data;
}

struct UctSearchStat {
  double time_elapsed;
  double searches_per_second;
  SgStatisticsExt<UctValueType, UctValueType> game_length;
  SgStatisticsExt<UctValueType, UctValueType> moves_in_tree;
  UctStatistics search_aborted;

  UctSearchStat();
  void Clear();
  void Write(std::ostream &out) const;
};


struct UctEarlyAbortParam {
  UctValueType abort_threshold;
  UctValueType min_searches_to_abort;
  UctValueType reduction_factor;
  UctEarlyAbortParam();
};


class UctSearch {
  typedef boost::recursive_mutex::scoped_lock GlobalRecursiveLock;
 public:
  static UctValueType InverseEval(UctValueType eval);
  static UctValueType InverseEstimate(UctValueType eval);
  explicit UctSearch(UctThreadStateFactory *threadStateFactory, int moveRange);
  virtual ~UctSearch();
  void SetThreadStateFactory(UctThreadStateFactory *factory);


  virtual std::string MoveString(GoMove move) const = 0;
  virtual bool IsPartialMove(GoMove move) const;
  virtual UctValueType UnknownEval() const = 0;

  virtual void OnSearchIteration(UctValueType gameNumber,
                                 std::size_t threadId,
                                 const UctGameInfo &info);
  virtual void OnStartSearch();
  virtual void OnStartGamePlay();
  virtual void OnEndGamePlay();
  virtual void OnStartSelfPlay();
  virtual void OnEndSelfPlay();
  virtual void OnEndSearch();
  virtual void OnThreadStartSearch(UctThreadState &state);
  virtual void OnThreadEndSearch(UctThreadState &state);
  virtual UctValueType GamesPlayed() const;


  int DeepUctSearchTree(UctThreadState &state, GlobalRecursiveLock *lock);
  const UctNode *Select(UctThreadState &state, const UctNode &parent, UctValueType c_puct);
  const UctNode *SelectWithDirichletNoise(UctThreadState &state, const UctNode &parent, UctValueType pCut);
  bool ExpandAndBackPropogate(UctThreadState &state, const UctNode *root, const UctNode &leafNode, int depth);
  void generateDirichlet(double out_[], size_t length);

  void GenerateAllMoves(std::vector<UctMoveInfo> &moves);
  void PlayGame();
  void PreStartSearch(const std::vector<GoMove> &rootFilter = std::vector<GoMove>(), UctSearchTree *initTree = nullptr,
                      bool syncState = true);
  void EndSearch();
  UctValueType StartSearchThread(UctValueType maxGames, double maxTime,
                               std::vector<GoMove> &sequence,
                               const std::vector<GoMove> &rootFilter
                               = std::vector<GoMove>(),
                               UctSearchTree *initTree = 0,
                               UctEarlyAbortParam *earlyAbort = nullptr);
  UctValueType StartDeepUCTSearchThread(UctValueType maxGames, double maxTime,
                                      std::vector<GoMove> &sequence_,
                                      std::vector<UctNode *> *bestchild_,
                                      float *policy_,
                                      double tau,
                                      const std::vector<GoMove> &rootFilter
                                      = std::vector<GoMove>(),
                                      UctSearchTree *initTree = nullptr,
                                      UctEarlyAbortParam *earlyAbort = nullptr,
                                      bool syncState = true);
  void SyncStateAgainst(GoMove move, SgBlackWhite color);
  GoPoint SearchOnePly(UctValueType maxGames, double maxTime,
                       UctValueType &value);
  void PrepareGamePlay();
  UctValueType EstimateGameScore();
  void UpdateCheckPoint(const std::string &checkpoint);
  const std::string &getCheckPoint();
  const UctNode *FindBestChild(const UctNode &node, const std::vector<GoMove> *excludeMoves = nullptr) const;
  void FindBestSequence(std::vector<GoMove> &sequence) const;

  UctValueType GetBound(bool useRave, const UctNode &node,
                      const UctNode &child) const;
  UctValueType GetCPUCTValue(const UctNode &node, const UctNode &child) const;


  const UctGameInfo &LastGameInfo() const;
  std::string LastGameSummaryLine() const;
  bool WasEarlyAbort() const;

  const UctSearchTree &Tree() const;
  UctSearchTree &GetTempTree();

  float BiasTermConstant() const;
  void SetBiasTermConstant(float biasTermConstant);
  int BiasTermFrequency() const;
  void SetBiasTermFrequency(int frequency);
  std::size_t BiasTermDepth() const;
  void SetBiasTermDepth(std::size_t depth);

  std::size_t MaxNodes() const;
  void SetMaxNodes(std::size_t maxNodes);
  std::size_t NumberThreads() const;
  void SetNumberThreads(std::size_t n);
  void SetThreadsNumberOnly(std::size_t n);

  UctValueType CheckTimeInterval() const;
  void SetCheckTimeInterval(UctValueType n);
  bool LockFree() const;
  void SetLockFree(bool enable);

  int RandomizeRaveFrequency() const;
  void SetRandomizeRaveFrequency(int frequency);
  bool RaveCheckSame() const;
  void SetRaveCheckSame(bool enable);

  UctValueType FirstPlayUrgency() const;
  void SetFirstPlayUrgency(UctValueType firstPlayUrgency);
  bool LogGames() const;
  void SetLogGames(bool enable);
  std::size_t MaxGameLength() const;
  void SetMaxGameLength(std::size_t maxGameLength);

  UctValueType ExpandThreshold() const;
  void SetExpandThreshold(UctValueType expandThreshold);
  std::size_t NumberPlayouts() const;
  void SetNumberPlayouts(std::size_t n);

  bool Rave() const;
  void SetRave(bool enable);
  UctMoveSelect MoveSelect() const;
  void SetMoveSelect(UctMoveSelect moveSelect);
  float RaveWeightInitial() const;
  void SetRaveWeightInitial(float value);
  float RaveWeightFinal() const;
  void SetRaveWeightFinal(float value);
  bool UpdateMultiplePlayoutsAsSingle() const;
  void SetUpdateMultiplePlayoutsAsSingle(bool enable);

  bool WeightRaveUpdates() const;
  void SetWeightRaveUpdates(bool enable);
  bool VirtualLoss() const;
  void SetVirtualLoss(bool enable);

  bool PruneFullTree() const;
  void SetPruneFullTree(bool enable);
  UctValueType PruneMinCount() const;
  void SetPruneMinCount(UctValueType n);
  bool CheckFloatPrecision() const;
  void SetCheckFloatPrecision(bool enable);
  void SetMpiSynchronizer(const MpiSynchronizerHandle &synchronizerHandle);
  MpiSynchronizerHandle GetMpiSynchronizer();
  const MpiSynchronizerHandle GetMpiSynchronizer() const;
  const UctSearchStat &Statistics() const;
  void WriteStatistics(std::ostream &out) const;
  UctThreadState &ThreadState(int i) const;
  bool ThreadsCreated() const;
  void CreateThreads();

 private:
  class Thread {
   public:
    std::unique_ptr<UctThreadState> m_state;
    Thread(UctSearch &search, std::unique_ptr<UctThreadState> &state);
    ~Thread();
    void HandleMsg(Msg &msg);
    void NotifyStartPlay(Msg &msg);
    void NotifyStartPlay();
    void WaitPlayFinished();

   private:
    class Function {
     public:
      explicit Function(Thread &thread);
      void operator()();

     private:
      Thread& thread;
    };
    friend class Function;
    UctSearch& searcher;
    std::queue<Msg> msg_q;
    bool should_quit;
    boost::barrier thread_ready_barrier;
    boost::mutex start_play_mutex;
    boost::mutex play_finish_mutex;
    boost::condition start_cond;
    boost::condition finish_cond;
    boost::mutex::scoped_lock play_finish_lock;
    GlobalRecursiveLock global_lock;
    boost::thread sys_thread;
    void operator()();
  };

  class NetworkEvalThread {
   public:
    explicit NetworkEvalThread(UctSearch &search);
    ~NetworkEvalThread();
    void Start();
    void Stop();
    UctBoardEvaluator &GetEvaluator();
    void UpdateCheckPoint(const std::string &checkpoint);
    bool TryLoadNeuralNetwork();
    void OnSearchThreadExit(size_t threadID);
    const std::string &getCheckPoint();
    EvalBuffer eval_buf;

   private:
    class Function {
     public:
      explicit Function(NetworkEvalThread &thread);
      void operator()();

     private:
      NetworkEvalThread &thread;
    };
    friend class Function;
    EvalMsg* thread_msg[128];
    bool neural_initialized;
    UctBoardEvaluator evaluator;
    UctSearch& searcher;
    bool to_quit;
    bool paused;
    std::string new_checkpoint;
    boost::barrier thread_ready_barrier;
    boost::thread sys_thread;
    boost::mutex wait_mutex;
    boost::condition wait_cv;
    void operator()();
  };

  std::unique_ptr<UctThreadStateFactory> th_state_factory;
  bool log_games;
  bool prune_tree;
  bool use_rave;
  unsigned int max_knowledge_threads;
  volatile bool search_aborted;
  volatile bool tree_exceeds_mem_limit;
  std::unique_ptr<boost::barrier> search_finish_barier;

  bool early_aborted;
  std::unique_ptr<UctEarlyAbortParam> early_abort_param;

  UctMoveSelect move_select;
  bool rave_check_same;
  int randomize_rave_freq;
  bool lock_free;
  bool weight_rave_updates;
  bool prune_full_tree;
  bool check_float_precision;
  std::size_t num_threads;
  std::size_t num_playouts;
  bool m_updateMultiplePlayoutsAsSingle;
  std::size_t max_nodes;
  UctValueType min_prune_cnt;
  const int move_range;
  std::size_t max_move_length;
  bool terminate_logged;
  UctValueType expand_thredhold;
  UctValueType max_games;

  UctValueType num_games;

  UctValueType start_root_move_cnt;
  UctValueType check_interval;
  volatile UctValueType next_check_time;
  double last_score_disp_time;
  float bias_term_const;
  int bias_term_freq;
  std::size_t bias_term_depth;
  UctValueType first_play_urgency;
  float initial_rave_weight;
  float final_rave_weight;
  UctValueType rave_weight_param_1;
  UctValueType rave_weight_param_2;
  UctValueType puct_const;
  double max_time;
  bool use_virtual_loss;
  std::string log_file_name;
  SgTimer search_timer;
  UctSearchTree search_tree;

  UctSearchTree tmp_search_tree;
  std::vector<GoMove> root_filter;
  std::ofstream logger_stream;

  boost::recursive_mutex global_mutex;
  UctSearchStat search_stat;
  SgRandom rand_generator;
  std::vector<boost::shared_ptr<Thread> > search_threads;
  std::string check_point;

#ifdef USE_NNEVALTHREAD
  std::unique_ptr<NetworkEvalThread> eval_thread;
#endif

#if USE_FASTLOG
  SgFastLog fast_logrithm;
#endif

  bool sync_state;
  boost::shared_ptr<MpiSynchronizer> mpi_synchronizer;


  void ApplyRootFilter(std::vector<UctMoveInfo> &moves);
  void PropagateProvenStatus(const std::vector<const UctNode *> &nodes);
  bool CheckAbortForDeepSearch(UctThreadState &state);
  bool CheckAbortSearch(UctThreadState &state);
  bool CheckEarlyAbort() const;
  bool CheckCountAbort(UctThreadState &state,
                       UctValueType remainingGames) const;
  void Debug(const UctThreadState &state, const std::string &textLine);
  void DeleteThreads();
  void ExpandNode(UctThreadState &state, const UctNode &node);
  void CreateChildren(UctThreadState &state, const UctNode &node,
                      bool deleteChildTrees);
  UctValueType GetBound(bool useRave, bool useBiasTerm,
                      UctValueType logPosCount,
                      const UctNode &child) const;
  UctValueType GetValueEstimate(bool useRave, const UctNode &child) const;
  UctValueType GetMeanValue(const UctNode& child) const;
  UctValueType GetValueEstimateRave(const UctNode &child) const;
  UctValueType Log(UctValueType x) const;
  void PlayGame(UctThreadState &state, GlobalRecursiveLock *lock);
  void AddDirichletNoise(const UctNode *root);
  void DeepUCTSearchLoop(UctThreadState &state, GlobalRecursiveLock *lock);
  UctValueType EstimateScore(UctThreadState &state);
  UctNode *DeepUCTSelectBestChild(UctValueType tau, float* policy = 0);
  bool PlayInTree(UctThreadState &state, bool &isTerminal);
  bool PlayoutGame(UctThreadState &state, std::size_t playout);
  void PrintSearchProgress(double currTime) const;
  void SearchLoop(UctThreadState &state, GlobalRecursiveLock *lock);
  const UctNode &SelectChild(int &randomizeCounter, bool useBiasTerm, const UctNode &node);
  std::string SummaryLine(const UctGameInfo &info) const;
  void UpdateCheckTimeInterval(double time);
  void UpdatePriorProbability(const UctNode &node, UctValueType actionProbs[]);
  void UpdateRaveValues(UctThreadState &state);
  void UpdateRaveValues(UctThreadState &state, std::size_t playout);
  void UpdateRaveValues(UctThreadState &state, std::size_t playout,
                        UctValueType eval, std::size_t i,
                        const std::size_t firstPlay[],
                        const std::size_t firstPlayOpp[]);
  void UpdateStatistics(const UctGameInfo &info);
  void UpdateTree(const UctGameInfo &info);
  void BackupTree(const UctNode *root, const UctNode *node, UctValueType value);
};

inline float UctSearch::BiasTermConstant() const {
  return bias_term_const;
}

inline bool UctSearch::CheckFloatPrecision() const {
  return check_float_precision;
}

inline UctValueType UctSearch::ExpandThreshold() const {
  return expand_thredhold;
}

inline UctBoardEvaluator &UctSearch::NetworkEvalThread::GetEvaluator() {
  return evaluator;
}

inline int UctSearch::BiasTermFrequency() const {
  return bias_term_freq;
}

inline void UctSearch::SetBiasTermFrequency(int frequency) {
  bias_term_freq = frequency;
}

inline std::size_t UctSearch::BiasTermDepth() const {
  return bias_term_depth;
}

inline void UctSearch::SetBiasTermDepth(std::size_t depth) {
  bias_term_depth = depth;
}

inline UctValueType UctSearch::FirstPlayUrgency() const {
  return first_play_urgency;
}

inline UctValueType UctSearch::InverseEval(UctValueType eval) {
  return UctValueUtil::InverseValue(eval);
}

inline UctValueType UctSearch::InverseEstimate(UctValueType eval) {
  return UctValueUtil::InverseValue(eval);
}

inline bool UctSearch::IsPartialMove(GoMove move) const {
  SuppressUnused(move);
  return false;
}

inline bool UctSearch::LockFree() const {
  return lock_free;
}

inline const UctGameInfo &UctSearch::LastGameInfo() const {
  return ThreadState(0).game_info;
}

inline bool UctSearch::LogGames() const {
  return log_games;
}

inline std::size_t UctSearch::MaxGameLength() const {
  return max_move_length;
}

inline std::size_t UctSearch::MaxNodes() const {
  return max_nodes;
}

inline UctMoveSelect UctSearch::MoveSelect() const {
  return move_select;
}

inline std::size_t UctSearch::NumberThreads() const {
  return num_threads;
}

inline UctValueType UctSearch::CheckTimeInterval() const {
  return check_interval;
}

inline std::size_t UctSearch::NumberPlayouts() const {
  return num_playouts;
}

inline bool UctSearch::UpdateMultiplePlayoutsAsSingle() const {
  return m_updateMultiplePlayoutsAsSingle;
}

inline void UctSearch::PlayGame() {
  PlayGame(ThreadState(0), 0);
}

inline bool UctSearch::PruneFullTree() const {
  return prune_full_tree;
}

inline UctValueType UctSearch::PruneMinCount() const {
  return min_prune_cnt;
}

inline bool UctSearch::Rave() const {
  return use_rave;
}

inline bool UctSearch::RaveCheckSame() const {
  return rave_check_same;
}

inline float UctSearch::RaveWeightInitial() const {
  return initial_rave_weight;
}

inline float UctSearch::RaveWeightFinal() const {
  return final_rave_weight;
}

inline void UctSearch::SetBiasTermConstant(float biasTermConstant) {
  bias_term_const = biasTermConstant;
}

inline void UctSearch::SetCheckFloatPrecision(bool enable) {
  check_float_precision = enable;
}

inline void UctSearch::SetExpandThreshold(UctValueType expandThreshold) {
  DBG_ASSERT(expandThreshold >= 0);
  expand_thredhold = expandThreshold;
}

inline void UctSearch::SetFirstPlayUrgency(UctValueType firstPlayUrgency) {
  first_play_urgency = firstPlayUrgency;
}

inline void UctSearch::SetLockFree(bool enable) {
  lock_free = enable;
}

inline void UctSearch::SetLogGames(bool enable) {
  log_games = enable;
}

inline void UctSearch::SetMaxGameLength(std::size_t maxGameLength) {
  max_move_length = maxGameLength;
}

inline void UctSearch::SetMaxNodes(std::size_t maxNodes) {
  max_nodes = maxNodes;
  if (search_threads.size() > 0)
    search_tree.SetMaxNodes(max_nodes);
}

inline void UctSearch::SetMoveSelect(UctMoveSelect moveSelect) {
  move_select = moveSelect;
}

inline void UctSearch::SetNumberPlayouts(std::size_t n) {
  DBG_ASSERT(n >= 1);
  num_playouts = n;
}

inline void UctSearch::SetUpdateMultiplePlayoutsAsSingle(bool enable) {
  m_updateMultiplePlayoutsAsSingle = enable;
}

inline void UctSearch::SetPruneFullTree(bool enable) {
  prune_full_tree = enable;
}

inline void UctSearch::SetPruneMinCount(UctValueType n) {
  min_prune_cnt = n;
}

inline void UctSearch::SetMpiSynchronizer(const MpiSynchronizerHandle
                                            &synchronizerHandle) {
  mpi_synchronizer = MpiSynchronizerHandle(synchronizerHandle);
}

inline MpiSynchronizerHandle UctSearch::GetMpiSynchronizer() {
  return MpiSynchronizerHandle(mpi_synchronizer);
}

inline const MpiSynchronizerHandle UctSearch::GetMpiSynchronizer() const {
  return MpiSynchronizerHandle(mpi_synchronizer);
}

inline int UctSearch::RandomizeRaveFrequency() const {
  return randomize_rave_freq;
}

inline void UctSearch::SetRandomizeRaveFrequency(int frequency) {
  randomize_rave_freq = frequency;
}

inline void UctSearch::SetRaveCheckSame(bool enable) {
  rave_check_same = enable;
}

inline void UctSearch::SetRaveWeightFinal(float value) {
  final_rave_weight = value;
}

inline void UctSearch::SetRaveWeightInitial(float value) {
  initial_rave_weight = value;
}

inline void UctSearch::SetWeightRaveUpdates(bool enable) {
  weight_rave_updates = enable;
}

inline bool UctSearch::VirtualLoss() const {
  return use_virtual_loss;
}

inline void UctSearch::SetVirtualLoss(bool enable) {
  use_virtual_loss = enable;
}

inline const UctSearchStat &UctSearch::Statistics() const {
  return search_stat;
}

inline bool UctSearch::ThreadsCreated() const {
  return (search_threads.size() > 0);
}

inline UctThreadState &UctSearch::ThreadState(int i) const {
  DBG_ASSERT(static_cast<std::size_t>(i) < search_threads.size());
  return *search_threads[i]->m_state;
}

inline const UctSearchTree &UctSearch::Tree() const {
  return search_tree;
}

inline bool UctSearch::WasEarlyAbort() const {
  return early_aborted;
}

inline bool UctSearch::WeightRaveUpdates() const {
  return weight_rave_updates;
}

#endif
