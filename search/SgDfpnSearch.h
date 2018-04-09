
#ifndef SG_DFPN_SEARCH_H
#define SG_DFPN_SEARCH_H

#include "board/GoBoardColor.h"
#include "SgHashTable.h"
#include "SgStatistics.h"
#include "platform/SgTimer.h"
#include "SgSearchTracer.h"

#include <limits>
#include <ostream>
#include <boost/scoped_ptr.hpp>

typedef std::vector<GoMove> PointSequence;

typedef SgStatisticsExt<float, std::size_t> DfpnStatistics;

typedef unsigned DfpnBoundType;

struct DfpnBounds {

  static const DfpnBoundType INFTY = 2000000000;

  static const DfpnBoundType MAX_WORK = INFTY / 2;

  DfpnBoundType phi;

  DfpnBoundType delta;
  DfpnBounds();
  DfpnBounds(DfpnBoundType p, DfpnBoundType d);

  bool GreaterThan(const DfpnBounds &other) const;

  bool IsWinning() const;

  bool IsLosing() const;

  bool IsSolved() const;
  void CheckConsistency() const;

  std::string Print() const;

  static void SetToWinning(DfpnBounds &bounds);

  static void SetToLosing(DfpnBounds &bounds);
};

inline DfpnBounds::DfpnBounds()
    : phi(INFTY),
      delta(INFTY) {}

inline DfpnBounds::DfpnBounds(DfpnBoundType p, DfpnBoundType d)
    : phi(p),
      delta(d) {}

inline std::string DfpnBounds::Print() const {
  std::ostringstream os;
  os << "[" << phi << ", " << delta << "]";
  return os.str();
}

inline bool DfpnBounds::GreaterThan(const DfpnBounds &other) const {
  return (phi > other.phi) && (delta > other.delta);
}

inline bool DfpnBounds::IsWinning() const {
  return phi == 0;
}

inline bool DfpnBounds::IsLosing() const {
  return delta == 0;
}

inline bool DfpnBounds::IsSolved() const {
  return IsWinning() || IsLosing();
}

inline void DfpnBounds::SetToWinning(DfpnBounds &bounds) {
  bounds.phi = 0;
  bounds.delta = INFTY;
}

inline void DfpnBounds::SetToLosing(DfpnBounds &bounds) {
  bounds.phi = INFTY;
  bounds.delta = 0;
}


inline std::ostream &operator<<(std::ostream &os, const DfpnBounds &bounds) {
  os << bounds.Print();
  return os;
}

class DfpnChildren {
 public:
  DfpnChildren();
  const std::vector<GoMove> &Children() const;
  std::vector<GoMove> &Children();
  std::size_t Size() const;
  GoMove MoveAt(std::size_t index) const;

 private:
  friend class DfpnSolver;
  std::vector<GoMove> m_children;
};

inline std::size_t DfpnChildren::Size() const {
  return m_children.size();
}

inline GoMove DfpnChildren::MoveAt(std::size_t index) const {
  return m_children[index];
}

inline const std::vector<GoMove> &DfpnChildren::Children() const {
  return m_children;
}

inline std::vector<GoMove> &DfpnChildren::Children() {
  return m_children;
}

class DfpnData {
 public:
  DfpnBounds m_bounds;
  GoMove m_bestMove;
  size_t m_work;
  DfpnData();
  DfpnData(const DfpnBounds &bounds, GoMove bestMove, size_t work);
  ~DfpnData();
  std::string Print() const;


  bool IsValid() const;
  void Invalidate();
  bool IsBetterThan(const DfpnData &data) const;

 private:
  bool m_isValid;
};

inline DfpnData::DfpnData()
    : m_isValid(false) {}

inline DfpnData::DfpnData(const DfpnBounds &bounds,
                          GoMove bestMove, size_t work)
    : m_bounds(bounds),
      m_bestMove(bestMove),
      m_work(work),
      m_isValid(true) {}

inline DfpnData::~DfpnData() {}

inline std::string DfpnData::Print() const {
  std::ostringstream os;
  os << '['
     << "bounds=" << m_bounds << ' '
     << "bestmove=" << m_bestMove << ' '
     << "work=" << m_work << ' '
     << ']';
  return os.str();
}

inline bool DfpnData::IsBetterThan(const DfpnData &data) const {
  return m_work > data.m_work;
}

inline bool DfpnData::IsValid() const {
  return m_isValid;
}

inline void DfpnData::Invalidate() {
  m_isValid = false;
}


inline std::ostream &operator<<(std::ostream &os, const DfpnData &data) {
  os << data.Print();
  return os;
}


class DfpnHistory {
 public:
  DfpnHistory();

  void Push(GoMove m_move, SgHashCode hash);

  void Pop();

  int Depth() const;

  SgHashCode LastHash() const;

  GoMove LastMove() const;

 private:


  std::vector<GoMove> m_move;

  std::vector<SgHashCode> m_hash;
};

inline DfpnHistory::DfpnHistory() {
  m_move.push_back(GO_NULLMOVE);
  m_hash.push_back(0);
}

inline void DfpnHistory::Push(GoMove move, SgHashCode hash) {
  m_move.push_back(move);
  m_hash.push_back(hash);
}

inline void DfpnHistory::Pop() {
  m_move.pop_back();
  m_hash.pop_back();
}

inline int DfpnHistory::Depth() const {
  DBG_ASSERT(!m_move.empty());
  return static_cast<int>(m_move.size() - 1);
}

inline SgHashCode DfpnHistory::LastHash() const {
  return m_hash.back();
}

inline GoMove DfpnHistory::LastMove() const {
  return m_move.back();
}

typedef SgHashTable<DfpnData, 4> DfpnHashTable;

class DfpnSolver {
 public:

  DfpnSolver();
  virtual ~DfpnSolver();

  SgEmptyBlackWhite
  StartSearch(DfpnHashTable &positions, PointSequence &pv);
  SgEmptyBlackWhite
  StartSearch(DfpnHashTable &positions, PointSequence &pv,
              const DfpnBounds &maxBounds);

  bool Validate(DfpnHashTable &positions, const SgBlackWhite winner,
                SgSearchTracer &tracer);

  std::string EvaluationInfo() const;

  virtual void GenerateChildren(std::vector<GoMove> &children) const = 0;

  virtual void PlayMove(GoMove move) = 0;

  virtual void UndoMove() = 0;

  virtual bool TerminalState(SgBoardColor colorToPlay,
                             SgEmptyBlackWhite &winner)  = 0; //const


  virtual float Score(GoMove move) const; //


  virtual SgBoardColor GetColorToMove() const = 0;

  virtual SgHashCode Hash() const = 0;

  virtual void WriteMoveSequence(std::ostream &stream,
                                 const PointSequence &sequence) const = 0;
  size_t NumGenerateMovesCalls() const;
  size_t NumMIDcalls() const;
  size_t NumTerminalNodes() const;

  virtual void WriteMove(std::ostream &stream, GoMove move) const;

  double Timelimit() const;

  void SetTimelimit(double timelimit);

  int WideningBase() const;

  void SetWideningBase(int wideningBase);

  float WideningFactor() const;

  void SetWideningFactor(float wideningFactor);

  float Epsilon() const;

  void SetEpsilon(float epsilon);

 private:

  DfpnHashTable *m_hashTable;
  SgTimer m_timer;

  double m_timelimit;

  int m_wideningBase;

  float m_wideningFactor;

  float m_epsilon;

  size_t m_checkTimerAbortCalls;
  bool m_aborted;
  size_t m_numTerminal;
  size_t m_numMIDcalls;
  size_t m_generateMoves;
  SgStatisticsExt<float, std::size_t> m_prunedSiblingStats;
  SgStatisticsExt<float, std::size_t> m_moveOrderingPercent;
  SgStatisticsExt<float, std::size_t> m_moveOrderingIndex;
  SgStatisticsExt<float, std::size_t> m_deltaIncrease;
  size_t m_totalWastedWork;
  size_t MID(const DfpnBounds &n, DfpnHistory &history);
  void SelectChild(std::size_t &bestIndex, DfpnBoundType &delta2,
                   const std::vector<DfpnData> &childrenDfpnBounds,
                   size_t maxChildIndex) const;
  void UpdateBounds(DfpnBounds &bounds,
                    const std::vector<DfpnData> &childBounds,
                    size_t maxChildIndex) const;
  bool CheckAbort();
  size_t ComputeMaxChildIndex(const std::vector<DfpnData> &
  childrenData) const;
  // reconstruct the pv by following the best moves in hash table.
  // todo make const, use ModBoard. (game-specific)
  void GetPVFromHash(PointSequence &pv);

  void LookupChildDataNonConst(GoMove move, DfpnData &data);

  virtual void LookupChildData(GoMove move, DfpnData &data) const;

  void LookupData(DfpnData &data, const DfpnChildren &children,
                  std::size_t childIndex) const;
  void PrintStatistics(SgEmptyBlackWhite winner, const PointSequence &p)
  const;

  virtual bool TTRead(DfpnData &data) const;

  virtual bool TTRead(SgHashCode hash, DfpnData &data) const;

  virtual void TTWrite(const DfpnData &data);
};

inline float DfpnSolver::Epsilon() const {
  return m_epsilon;
}

inline size_t DfpnSolver::NumGenerateMovesCalls() const {
  return m_generateMoves;
}

inline size_t DfpnSolver::NumMIDcalls() const {
  return m_numMIDcalls;
}

inline size_t DfpnSolver::NumTerminalNodes() const {
  return m_numTerminal;
}

inline float DfpnSolver::Score(GoMove move) const {
  SuppressUnused(move);
  return 1.0f; // override
}

inline void DfpnSolver::SetEpsilon(float epsilon) {
  m_epsilon = epsilon;
}

inline void DfpnSolver::SetTimelimit(double timelimit) {
  m_timelimit = timelimit;
}

inline void DfpnSolver::SetWideningBase(int wideningBase) {
  m_wideningBase = wideningBase;
}

inline void DfpnSolver::SetWideningFactor(float wideningFactor) {
  m_wideningFactor = wideningFactor;
}

inline double DfpnSolver::Timelimit() const {
  return m_timelimit;
}

inline bool DfpnSolver::TTRead(SgHashCode hash, DfpnData &data) const {
  return m_hashTable->Lookup(hash, &data);
}

inline bool DfpnSolver::TTRead(DfpnData &data) const {
  return TTRead(Hash(), data);
}

inline void DfpnSolver::TTWrite(const DfpnData &data) {
#ifndef NDEBUG
  data.m_bounds.CheckConsistency();
#endif
  m_hashTable->Store(Hash(), data);
}

inline int DfpnSolver::WideningBase() const {
  return m_wideningBase;
}

inline float DfpnSolver::WideningFactor() const {
  return m_wideningFactor;
}

#endif // SG_DFPN_SEARCH_H
