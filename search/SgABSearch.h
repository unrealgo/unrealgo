
#ifndef SG_SEARCH_H
#define SG_SEARCH_H

#include "board/GoBlackWhite.h"
#include "lib/SgHash.h"
#include "board/GoPoint.h"
#include "SgSearchStatistics.h"
#include "SgSearchTracer.h"
#include "lib/SgStack.h"
#include "platform/SgTimer.h"
#include "lib/SgVector.h"

template<class DATA, int BLOCK_SIZE>
class SgHashTable;
class SgNode;
class SgProbCut;
class SgSearchControl;

class SgKiller {
 public:
  SgKiller();
  void MarkKiller(GoMove killer);
  void Clear();
  GoMove GetKiller1() const;
  GoMove GetKiller2() const;

 private:
  GoMove m_killer1;
  GoMove m_killer2;
  int m_count1;
  int m_count2;
};

inline SgKiller::SgKiller()
    : m_killer1(GO_NULLMOVE),
      m_killer2(GO_NULLMOVE),
      m_count1(0),
      m_count2(0) {}

inline GoMove SgKiller::GetKiller1() const {
  return m_killer1;
}

inline GoMove SgKiller::GetKiller2() const {
  return m_killer2;
}


class SgSearchHashData {
 public:
  SgSearchHashData();
  SgSearchHashData(int depth, signed value, GoMove bestMove,
                   bool isOnlyUpperBound = false,
                   bool isOnlyLowerBound = false,
                   bool isExactValue = false);
  ~SgSearchHashData();
  int Depth() const;
  int Value() const;
  GoMove BestMove() const;
  bool IsOnlyUpperBound() const;
  bool IsOnlyLowerBound() const;
  void AdjustBounds(int *lower, int *upper);
  bool IsBetterThan(const SgSearchHashData &data) const;
  bool IsValid() const;
  bool IsExactValue() const;
  void Invalidate();
  void AgeData();

 private:
  unsigned m_depth : 12;
  unsigned m_isUpperBound : 1;
  unsigned m_isLowerBound : 1;
  unsigned m_isValid : 1;
  unsigned m_isExactValue : 1;
  signed m_value : 16;
  GoMove m_bestMove;
};
typedef SgHashTable<SgSearchHashData, 4> SgSearchHashTable;

inline SgSearchHashData::SgSearchHashData()
    : m_depth(0),
      m_isUpperBound(0),
      m_isLowerBound(0),
      m_isValid(0),
      m_isExactValue(0),
      m_value(-1),
      m_bestMove(GO_NULLMOVE) {}

inline SgSearchHashData::SgSearchHashData(int depth, signed value,
                                          GoMove bestMove,
                                          bool isOnlyUpperBound,
                                          bool isOnlyLowerBound,
                                          bool isExactValue)
    :
    m_depth(depth & ((1 << 12) - 1)),
    m_isUpperBound(isOnlyUpperBound),
    m_isLowerBound(isOnlyLowerBound),
    m_isValid(1),
    m_isExactValue(isExactValue),
    m_value((short int) (value) & (short int) ((1 << 16) - 1)),
    m_bestMove(bestMove) {
  DBG_ASSERT(m_value == value);
}

inline SgSearchHashData::~SgSearchHashData() {}

inline int SgSearchHashData::Depth() const {
  return static_cast<int> (m_depth);
}

inline int SgSearchHashData::Value() const {
  return static_cast<int> (m_value);
}

inline GoMove SgSearchHashData::BestMove() const {
  return m_bestMove;
}

inline bool SgSearchHashData::IsOnlyUpperBound() const {
  return m_isUpperBound != 0;
}

inline bool SgSearchHashData::IsOnlyLowerBound() const {
  return m_isLowerBound != 0;
}

inline void SgSearchHashData::AdjustBounds(int *lower, int *upper) {
  if (IsOnlyUpperBound())
    *upper = std::min(*upper, Value());
  else if (IsOnlyLowerBound())
    *lower = std::max(*lower, Value());
  else {
    *lower = Value();
    *upper = Value();
  }
}

inline bool SgSearchHashData::IsValid() const {
  return m_isValid;
}

inline bool SgSearchHashData::IsExactValue() const {
  return m_isExactValue;
}

inline void SgSearchHashData::Invalidate() {
  m_isValid = false;
}

inline void SgSearchHashData::AgeData() {
  m_depth = 0;
}
namespace SgSearchLimit {
static const int MAX_DEPTH = 256;
}
typedef SgStack<GoMove, SgSearchLimit::MAX_DEPTH> SgSearchStack;


class SgABSearch {
 public:
  
  static const int DEPTH_UNIT = 100;
  
  static const int MAX_DEPTH = 256;
  
  static const int SG_INFINITY;
  
  SgABSearch(SgSearchHashTable *hash);
  virtual ~SgABSearch();
  
  virtual bool CheckDepthLimitReached() const = 0;
  const SgSearchHashTable *HashTable() const;
  void SetHashTable(SgSearchHashTable *hashtable);
  const SgSearchControl *SearchControl() const;
  
  void SetSearchControl(SgSearchControl *control);
  
  void SetProbCut(SgProbCut *probcut);
  
  virtual std::string MoveString(GoMove move) const = 0;
  
  virtual void SetToPlay(SgBlackWhite toPlay) = 0;
  
  virtual void OnStartSearch();
  
  int DepthFirstSearch(int depthLimit, int boundLo, int boundHi,
                       SgVector<GoMove> *sequence, bool clearHash = true,
                       SgNode *traceNode = 0);
  
  int DepthFirstSearch(int depthLimit, SgVector<GoMove> *sequence,
                       bool clearHash = true, SgNode *traceNode = 0);
  
  int IteratedSearch(int depthMin, int depthMax, int boundLo,
                     int boundHi, SgVector<GoMove> *sequence,
                     bool clearHash = true, SgNode *traceNode = 0);
  
  int IteratedSearch(int depthMin, int depthMax, SgVector<GoMove> *sequence,
                     bool clearHash = true, SgNode *traceNode = 0);
  
  int IteratedSearchDepthLimit() const;
  
  virtual void StartOfDepth(int depthLimit);
  
  bool Aborted() const;
  
  void SetAbortSearch(bool fAborted = true);
  void SetScout(bool flag = true);
  void SetKillers(bool flag = true);
  void SetOpponentBest(bool flag = true);
  void SetNullMove(bool flag = true);
  void SetNullMoveDepth(int depth);
  
  void GetStatistics(SgSearchStatistics *stat);

  const SgSearchStatistics &Statistics() const {
    return m_stat;
  }

  
  void StartTime();
  
  void StopTime();
  
  virtual void Generate(SgVector<GoMove> *moves, int depth) = 0;
  
  virtual int Evaluate(bool *isExact, int depth) = 0;
  
  virtual bool Execute(GoMove move, int *delta, int depth) = 0;
  
  virtual void TakeBack() = 0;
  
  virtual SgBlackWhite GetToPlay() const = 0;
  
  virtual bool AbortSearch();
  
  virtual SgHashCode GetHashCode() const = 0;
  
  int CurrentDepth() const;
  
  GoMove PrevMove() const;
  
  GoMove PrevMove2() const;
  
  virtual bool EndOfGame() const = 0;
  
  void InitSearch(int startDepth = 0);
  
  bool TraceIsOn() const;
  
  virtual void CreateTracer();
  
  void SetTracer(SgSearchTracer *tracer);
  SgSearchTracer *Tracer() const;
  void SetAbortFrequency(int value);
  
  int SearchEngine(int depth, int alpha, int beta, SgSearchStack &stack,
                   bool *isExactValue, bool lastNullMove = false);

 private:
  
  SgSearchHashTable *m_hash;
  
  SgSearchTracer *m_tracer;
  
  int m_currentDepth;
  int m_depthLimit;
  
  SgVector<GoMove> m_moveStack;
  bool m_useScout;
  bool m_useKillers;
  
  bool m_useOpponentBest;
  
  bool m_useNullMove;
  
  int m_nullMoveDepth;
  
  bool m_aborted;
  
  bool m_foundNewBest;
  
  bool m_reachedDepthLimit;
  SgSearchStatistics m_stat;
  SgTimer m_timer;
  int m_timerLevel;
  
  int m_prevValue;
  
  SgVector<GoMove> m_prevSequence;
  static const int MAX_KILLER_DEPTH = 10;
  
  GoArray<SgKiller, MAX_KILLER_DEPTH + 1> m_killers;
  SgSearchControl *m_control;
  SgProbCut *m_probcut;
  int m_abortFrequency;
  
  int DFS(int startDepth, int depthLimit, int boundLo, int boundHi,
          SgVector<GoMove> *sequence, bool *isExactValue);
  
  bool LookupHash(SgSearchHashData &data) const;
  bool NullMovePrune(int depth, int delta, int beta);
  
  void StoreHash(int depth, int value, GoMove move, bool isUpperBound,
                 bool isLowerBound, bool isExact);
  
  void AddSequenceToHash(const SgVector<GoMove> &sequence, int depth);
  
  int CallEvaluate(int depth, bool *isExact);
  
  bool CallExecute(GoMove move, int *delta, int depth);
  
  void CallGenerate(SgVector<GoMove> *moves, int depth);
  
  void CallTakeBack();
  bool TryMove(GoMove move, const SgVector<GoMove> &specialMoves,
               const int depth,
               const int alpha, const int beta,
               int &loValue, int &hiValue,
               SgSearchStack &stack,
               bool &allExact,
               bool &isCutoff);
  bool TrySpecialMove(GoMove move, SgVector<GoMove> &specialMoves,
                      const int depth,
                      const int alpha, const int beta,
                      int &loValue, int &hiValue,
                      SgSearchStack &stack,
                      bool &allExact,
                      bool &isCutoff);
  
  SgABSearch(const SgABSearch &);
  
  SgABSearch &operator=(const SgABSearch &);
};

inline bool SgABSearch::Aborted() const {
  return m_aborted;
}

inline int SgABSearch::CurrentDepth() const {
  return m_currentDepth;
}

inline int SgABSearch::DepthFirstSearch(int depthLimit,
                                        SgVector<GoMove> *sequence,
                                        bool clearHash, SgNode *traceNode) {
  return DepthFirstSearch(depthLimit, -SG_INFINITY, SG_INFINITY, sequence,
                          clearHash, traceNode);
}

inline const SgSearchHashTable *SgABSearch::HashTable() const {
  return m_hash;
}

inline void SgABSearch::SetHashTable(SgSearchHashTable *hashtable) {
  m_hash = hashtable;
}

inline int SgABSearch::IteratedSearchDepthLimit() const {
  return m_depthLimit;
}

inline int SgABSearch::IteratedSearch(int depthMin, int depthMax,
                                      SgVector<GoMove> *sequence,
                                      bool clearHash,
                                      SgNode *traceNode) {
  return IteratedSearch(depthMin, depthMax, -SG_INFINITY, SG_INFINITY,
                        sequence, clearHash, traceNode);
}

inline GoMove SgABSearch::PrevMove() const {
  return m_moveStack.Back();
}

inline GoMove SgABSearch::PrevMove2() const {
  return m_moveStack.TopNth(2);
}

inline const SgSearchControl *SgABSearch::SearchControl() const {
  return m_control;
}

inline void SgABSearch::SetAbortFrequency(int value) {
  m_abortFrequency = value;
}

inline void SgABSearch::SetAbortSearch(bool fAborted) {
  m_aborted = fAborted;
}

inline void SgABSearch::SetKillers(bool flag) {
  m_useKillers = flag;
}

inline void SgABSearch::SetNullMove(bool flag) {
  m_useNullMove = flag;
}

inline void SgABSearch::SetNullMoveDepth(int depth) {
  m_nullMoveDepth = depth;
}

inline void SgABSearch::SetOpponentBest(bool flag) {
  m_useOpponentBest = flag;
}

inline void SgABSearch::SetScout(bool flag) {
  m_useScout = flag;
}

inline void SgABSearch::SetTracer(SgSearchTracer *tracer) {
  DBG_ASSERT(!m_tracer || !tracer);
  m_tracer = tracer;
}

inline SgSearchTracer *SgABSearch::Tracer() const {
  return m_tracer;
}

#endif
