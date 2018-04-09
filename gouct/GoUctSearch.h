
#ifndef GOUCT_SEARCH_H
#define GOUCT_SEARCH_H

#include <iosfwd>
#include "GoBoard.h"
#include "GoBoardHistory.h"
#include "GoBoardSynchronizer.h"
#include "GoUctBoard.h"
#include "UctSearch.h"
#include "board/GoBlackWhite.h"
#include "SgStatistics.h"

class SgNode;
class GoUctState : public UctThreadState {
 public:
  GoUctState(unsigned int threadId, const GoBoard &bd);
  void Clear();
  void StartSearch();
  void Execute(GoMove move);
  void Apply(GoMove move);
  GoMove LastMove();
  const GoBoard &Board();
  void ExecutePlayout(GoMove move);
  void TakeBackInTree(std::size_t nuMoves);
  void TakeBackPlayout(std::size_t nuMoves);
  void GameStart();
  void StartPlayout();
  void StartPlayouts();
  const GoBoard &Board() const;
  const GoUctBoard &UctBoard() const;
  bool IsInPlayout() const;
  std::size_t GameLength() const;
  void Dump(std::ostream &out) const;

 private:
  class AssertionHandler : public AssertionHandlerInterface {
   public:
    explicit AssertionHandler(const GoUctState &state);
    void Run();

   private:
    const GoUctState &m_state;
  };
  AssertionHandler m_assertionHandler;
  GoBoard m_bd;
  GoUctBoard m_uctBd;
  GoBoardSynchronizer m_synchronizer;
  bool m_isInPlayout;
  std::size_t m_gameLength;
};

inline const GoBoard &GoUctState::Board() const {
  return m_bd;
}

inline std::size_t GoUctState::GameLength() const {
  return m_gameLength;
}

inline bool GoUctState::IsInPlayout() const {
  return m_isInPlayout;
}

inline const GoUctBoard &GoUctState::UctBoard() const {
  return m_uctBd;
}

enum GoUctLiveGfx {
  GOUCT_LIVEGFX_NONE,
  GOUCT_LIVEGFX_COUNTS,
  GOUCT_LIVEGFX_SEQUENCE
};

class GoUctSearch : public UctSearch {
 public:
  GoUctSearch(GoBoard &bd, UctThreadStateFactory *factory);
  ~GoUctSearch();
  std::string MoveString(GoMove move) const;
  SgBlackWhite ToPlay() const;
  void OnSearchIteration(UctValueType gameNumber, std::size_t threadId,
                         const UctGameInfo &info);
  void OnStartSearch();
  void OnStartGamePlay();
  void OnStartSelfPlay();
  GoBoard &Board();
  const GoBoard &Board() const;
  void SaveGames(const std::string &fileName) const;
  void SaveTree(std::ostream &out, int maxDepth = -1) const;
  void SetToPlay(SgBlackWhite toPlay);
  const GoBoardHistory &BoardHistory() const;
  bool KeepGames() const;
  void SetKeepGames(bool enable);
  GoUctLiveGfx LiveGfx() const;
  bool NeedLiveGfx(UctValueType gameNumber);
  void SetLiveGfx(GoUctLiveGfx mode);
  UctValueType LiveGfxInterval() const;
  void SetLiveGfxInterval(UctValueType interval);
  GoUctSearch(const GoUctSearch &search) = delete;
  GoUctSearch &operator=(const GoUctSearch &search)= delete;

 protected:
  virtual void DisplayGfx();

 private:
  bool m_keepGames;
  UctValueType m_liveGfxInterval;
  volatile UctValueType m_nextLiveGfx;
  SgBlackWhite m_toPlay;
  GoBWSet m_stones;
  GoBoard &m_bd;
  SgNode *m_root;
  GoUctLiveGfx m_liveGfx;
  GoBoardHistory m_boardHistory;
};

inline GoBoard &GoUctSearch::Board() {
  return m_bd;
}

inline const GoBoard &GoUctSearch::Board() const {
  return m_bd;
}

inline const GoBoardHistory &GoUctSearch::BoardHistory() const {
  return m_boardHistory;
}

inline bool GoUctSearch::KeepGames() const {
  return m_keepGames;
}

inline GoUctLiveGfx GoUctSearch::LiveGfx() const {
  return m_liveGfx;
}

inline UctValueType GoUctSearch::LiveGfxInterval() const {
  return m_liveGfxInterval;
}

inline void GoUctSearch::SetKeepGames(bool enable) {
  m_keepGames = enable;
}

inline void GoUctSearch::SetLiveGfx(GoUctLiveGfx mode) {
  m_liveGfx = mode;
}

inline void GoUctSearch::SetLiveGfxInterval(UctValueType interval) {
  DBG_ASSERT(interval > 0);
  m_liveGfxInterval = interval;
}

inline void GoUctSearch::SetToPlay(SgBlackWhite toPlay) {
  m_toPlay = toPlay;
  m_bd.SetToPlay(toPlay);
}

namespace GoUctSearchUtil {
GoPoint TrompTaylorPassCheck(GoPoint move, const GoUctSearch &search);
}

#endif // GOUCT_SEARCH_H
