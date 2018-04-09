
#ifndef GOUCT_PLAYER_H
#define GOUCT_PLAYER_H

#include <boost/scoped_ptr.hpp>
#include <vector>
#include "GoBoard.h"
#include "GoBoardRestorer.h"
#include "GoPlayer.h"
#include "GoTimeControl.h"
#include "GoUctDefaultMoveFilter.h"
#include "GoUctGlobalSearch.h"
#include "GoUctSearchObject.h"
#include "GoUctPlayoutPolicy.h"
#include "GoUctMoveFilter.h"
#include "lib/SgArrayList.h"
#include "platform/SgDebug.h"
#include "board/GoNbIterator.h"
#include "SgNode.h"
#include "board/GoPointArray.h"
#include "platform/SgRestorer.h"
#include "MpiSynchronizer.h"
#include "platform/SgTime.h"
#include "platform/SgTimer.h"
#include "UctTreeUtil.h"
#include "board/SgWrite.h"

template<typename T, int SIZE>
class SgSList;
enum GoUctGlobalSearchMode {
  GOUCT_SEARCHMODE_PLAYOUTPOLICY,
  GOUCT_SEARCHMODE_DEEPUCT,
  GOUCT_SEARCHMODE_UCT,
  GOUCT_SEARCHMODE_ONEPLY
};
template<class SEARCH, class THREADSTATE>
class GoUctPlayer
    : public GoPlayer,
        public GoUctSearchObject,
        public SgObjectWithDefaultTimeControl {
 public:

  struct Statistics {
    std::size_t m_nuGenMove;
    SgStatisticsExt<float, std::size_t> m_reuse;
    SgStatisticsExt<double, std::size_t> m_gamesPerSecond;
    Statistics();
    void Clear();
    void Write(std::ostream &out) const;
  };
  GoUctPlayoutPolicyParam m_playoutPolicyParam;
  GoUctDefaultMoveFilterParam m_rootFilterParam;
  GoUctDefaultMoveFilterParam m_treeFilterParam;
  explicit GoUctPlayer(const GoBoard &bd);
  ~GoUctPlayer();
  void OnBoardChange();
  GoPoint GenMove(const SgTimeRecord &time, SgBlackWhite toPlay);
  std::string Name() const;
  void Ponder();
  SgDefaultTimeControl &TimeControl();
  const SgDefaultTimeControl &TimeControl() const;
  GoUctSearch &Search();
  const GoUctSearch &Search() const;
  bool AutoParam() const;
  void SetAutoParam(bool enable);
  bool EarlyPass() const;
  void SetEarlyPass(bool enable);
  bool ForcedOpeningMoves() const;
  void SetForcedOpeningMoves(bool enable);
  bool IgnoreClock() const;
  void SetIgnoreClock(bool enable);
  UctValueType MaxGames() const;
  void SetMaxGames(UctValueType maxGames);
  bool EnablePonder() const;
  void SetEnablePonder(bool enable);
  double MaxPonderTime() const;
  void SetMaxPonderTime(double seconds);
  UctValueType ResignMinGames() const;
  void SetResignMinGames(UctValueType n);
  bool UseRootFilter() const;
  void SetUseRootFilter(bool enable);
  bool ReuseSubtree() const;
  void SetReuseSubtree(bool enable);
  UctValueType ResignThreshold() const;
  void SetResignThreshold(UctValueType threshold);
  GoUctGlobalSearchMode SearchMode() const;
  void SetSearchMode(GoUctGlobalSearchMode mode);
  bool WriteDebugOutput() const;
  void SetWriteDebugOutput(bool flag);
  const Statistics &GetStatistics() const;
  void ClearStatistics();
  SEARCH &GlobalSearch();
  GoUctMoveFilter &RootFilter();
  void SetRootFilter(GoUctMoveFilter *filter);
  void SetMpiSynchronizer(const MpiSynchronizerHandle &synchronizerHandle);
  MpiSynchronizerHandle GetMpiSynchronizer();

 private:
  GoUctGlobalSearchMode m_searchMode;
  bool m_autoParam;
  bool m_forcedOpeningMoves;
  bool m_ignoreClock;
  bool m_enablePonder;
  bool m_useRootFilter;
  bool m_reuseSubtree;
  bool m_earlyPass;
  UctValueType m_resignThreshold;
  const UctValueType m_sureWinThreshold;
  int m_lastBoardSize;
  UctValueType m_maxGames;
  UctValueType m_resignMinGames;
  double m_maxPonderTime;
  SEARCH m_search;
  GoTimeControl m_timeControl;
  Statistics m_statistics;
  boost::scoped_ptr<GoUctMoveFilter> m_rootFilter;
  boost::scoped_ptr<GoUctPlayoutPolicy<GoBoard> > m_playoutPolicy;
  MpiSynchronizerHandle m_mpiSynchronizer;
  bool m_writeDebugOutput;
  GoMove GenMovePlayoutPolicy(SgBlackWhite toPlay);
  bool DoEarlyPassSearch(UctValueType maxGames, double maxTime,
                         GoPoint searchMove, GoPoint &move);
  GoPoint DoSearch(SgBlackWhite toPlay, double maxTime,
                   bool isDuringPondering);
  void FindInitTree(UctSearchTree &initTree, SgBlackWhite toPlay,
                    double maxTime);
  void SetDefaultParameters(int boardSize);
  bool VerifyNeutralMove(UctValueType maxGames, double maxTime, GoPoint move);
};

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::AutoParam() const {
  return m_autoParam;
}

template<class SEARCH, class THREADSTATE>
inline GoUctSearch &GoUctPlayer<SEARCH, THREADSTATE>::Search() {
  return m_search; // return subclass of UctSearch
}

template<class SEARCH, class THREADSTATE>
inline const GoUctSearch &GoUctPlayer<SEARCH, THREADSTATE>::Search() const {
  return m_search;
}

template<class SEARCH, class THREADSTATE>
inline SEARCH &
GoUctPlayer<SEARCH, THREADSTATE>::GlobalSearch() {
  return m_search;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::EarlyPass() const {
  return m_earlyPass;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::EnablePonder() const {
  return m_enablePonder;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::ForcedOpeningMoves() const {
  return m_forcedOpeningMoves;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::IgnoreClock() const {
  return m_ignoreClock;
}

template<class SEARCH, class THREADSTATE>
inline UctValueType GoUctPlayer<SEARCH, THREADSTATE>::MaxGames() const {
  return m_maxGames;
}

template<class SEARCH, class THREADSTATE>
inline double GoUctPlayer<SEARCH, THREADSTATE>::MaxPonderTime() const {
  return m_maxPonderTime;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::UseRootFilter() const {
  return m_useRootFilter;
}

template<class SEARCH, class THREADSTATE>
inline UctValueType GoUctPlayer<SEARCH, THREADSTATE>::ResignMinGames() const {
  return m_resignMinGames;
}

template<class SEARCH, class THREADSTATE>
inline UctValueType GoUctPlayer<SEARCH, THREADSTATE>::ResignThreshold() const {
  return m_resignThreshold;
}

template<class SEARCH, class THREADSTATE>
inline bool GoUctPlayer<SEARCH, THREADSTATE>::ReuseSubtree() const {
  return m_reuseSubtree;
}

template<class SEARCH, class THREADSTATE>
inline GoUctMoveFilter &GoUctPlayer<SEARCH, THREADSTATE>::RootFilter() {
  return *m_rootFilter;
}

template<class SEARCH, class THREADSTATE>
inline GoUctGlobalSearchMode GoUctPlayer<SEARCH, THREADSTATE>::SearchMode() const {
  return m_searchMode;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetAutoParam(bool enable) {
  m_autoParam = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetEarlyPass(bool enable) {
  m_earlyPass = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetEnablePonder(bool enable) {
  m_enablePonder = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetForcedOpeningMoves(bool enable) {
  m_forcedOpeningMoves = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetIgnoreClock(bool enable) {
  m_ignoreClock = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetMaxGames(UctValueType maxGames) {
  m_maxGames = maxGames;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetMaxPonderTime(double seconds) {
  m_maxPonderTime = seconds;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetUseRootFilter(bool enable) {
  m_useRootFilter = enable;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetResignMinGames(UctValueType n) {
  m_resignMinGames = n;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetResignThreshold(UctValueType threshold) {
  m_resignThreshold = threshold;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetRootFilter(GoUctMoveFilter *
filter) {
  m_rootFilter.reset(filter);
}

template<class SEARCH, class THREADSTATE>
inline void
GoUctPlayer<SEARCH, THREADSTATE>::SetSearchMode(GoUctGlobalSearchMode mode) {
  m_searchMode = mode;
}

template<class SEARCH, class THREADSTATE>
inline void GoUctPlayer<SEARCH, THREADSTATE>::SetMpiSynchronizer(const MpiSynchronizerHandle &handle) {
  m_mpiSynchronizer = MpiSynchronizerHandle(handle);
  m_search.SetMpiSynchronizer(handle);
}

template<class SEARCH, class THREADSTATE>
inline MpiSynchronizerHandle
GoUctPlayer<SEARCH, THREADSTATE>::GetMpiSynchronizer() {
  return MpiSynchronizerHandle(m_mpiSynchronizer);
}

template<class SEARCH, class THREADSTATE>
GoUctPlayer<SEARCH, THREADSTATE>::Statistics::Statistics() {
  Clear();
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::Statistics::Clear() {
  m_nuGenMove = 0;
  m_gamesPerSecond.Clear();
  m_reuse.Clear();
}

template<class SEARCH, class THREADSTATE>
bool GoUctPlayer<SEARCH, THREADSTATE>::WriteDebugOutput() const {
  return m_writeDebugOutput;
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::SetWriteDebugOutput(bool flag) {
  m_writeDebugOutput = flag;
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::Statistics::Write(std::ostream &out) const {
  out << SgWriteLabel("NuGenMove") << m_nuGenMove << '\n'
      << SgWriteLabel("GamesPerSec");
  m_gamesPerSecond.Write(out);
  out << '\n'
      << SgWriteLabel("Reuse");
  m_reuse.Write(out);
  out << '\n';
}

typedef GoPointArray<UctStatistics> TerrArray;

inline bool HasStatsForAllMoves(const GoBoard &bd, const TerrArray &territory) {
  for (GoBoard::Iterator it(bd); it; ++it)
    if (territory[*it].Count() == 0) {
      // No statistics, maybe all simulations aborted due to
      // max length or mercy rule.
      SgDebug() << "GoUctPlayer: no early pass possible (no stat) for"
                << GoWritePoint(*it) << '\n';
      return false;
    }
  return true;
}

#endif // GOUCT_PLAYER_H
