
#ifndef SG_UCT_DEEPPLAYER_H
#define SG_UCT_DEEPPLAYER_H

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
#include "funcapproximator/DlCheckPoint.h"
#include "Allocator.h"

typedef GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>, GoUctPlayoutPolicyFactory<GoUctBoard> > GoUctGlobalSearchType;
class UctDeepPlayer
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

  UctDeepPlayer(GoGame &game, GoRules &rules);
  ~UctDeepPlayer() = default;
  void ClearBoard();
  std::string SelfPlayAsServer(int roundID);
  void SelfPlayAsClient();
  void UpdateCheckPoint(DlCheckPoint::CheckPointInfo &ckInfo);
  void SelfPlayOneGame(const std::string &path, int gameID, bool uploadToServer = false);
  void UpdateGameInfo(DlCheckPoint::CheckPointInfo &ckInfo, const std::string &gameName);
  void UploadTFRecordToServer(UctNode *root, const std::string &path);
  void WriteSgf(UctNode *root, const std::string &path);
  void WriteTFRecord(UctNode *root, const std::string &filename);
  void
  LogSelfPlayGame(UctNode *root, const std::string &path, int gameID);

  void SyncState(GoMove move, SgBlackWhite color);
  GoPoint SearchAgainstAndSync(SgBlackWhite toPlay, double maxTime, double tau,
                               std::vector<UctNode *> *child, float *policy_ = 0, bool syncState = true);
  GoPoint DoSearch(SgBlackWhite toPlay, double maxTime, double tau, std::vector<UctNode *> *child, float *policy_ = 0,
                   bool syncState = true);
  void FindInitTree(UctSearchTree &initTree, SgBlackWhite toPlay, double maxTime);
  GoPoint GenMove(const SgTimeRecord &timeRecord, SgBlackWhite toPlay) final;
  void TryInitNeuralNetwork();
  SgDefaultTimeControl &TimeControl();
  const SgDefaultTimeControl &TimeControl() const;
  GoUctSearch &Search();
  const GoUctSearch &Search() const;
  void SetLogReuse(bool reuse);
  void Abort();

 private:
  bool m_logReuse;
  GoUctGlobalSearchType m_search;
  int m_maxiterations;
  DlCheckPoint::CheckPointInfo m_bestCheckPoint;

  GoRules &m_engineRules;
  GoGame &m_game;
  volatile bool m_aborted;
  int m_maxGameLength;
  bool m_forcedOpeningMoves;
  bool m_ignoreClock;
  bool m_reuseSubtree;
  bool m_earlyPass;
  const UctValueType m_sureWinThreshold;
  UctValueType m_resignThreshold;
  UctValueType m_resignMinGames;
  GoTimeControl m_timeControl;
  Statistics m_statistics;
  MpiSynchronizerHandle m_mpiSynchronizer;
  bool m_writeDebugOutput;
  std::vector<UctNode *> m_nodeSequence;
  std::unique_ptr<UctPolicyAllocator> m_policyAllocator;
  std::unique_ptr<UctNodeAllocator> m_nodeAllocator;
};

inline SgDefaultTimeControl &UctDeepPlayer::TimeControl() {
  return m_timeControl;
}

inline const SgDefaultTimeControl &UctDeepPlayer::TimeControl() const {
  return m_timeControl;
}

inline void UctDeepPlayer::UpdateCheckPoint(DlCheckPoint::CheckPointInfo &ckInfo) {
  m_bestCheckPoint = ckInfo;
  m_search.UpdateCheckPoint(m_bestCheckPoint.name);
}

#endif
