

#ifndef GO_PLAYER_H
#define GO_PLAYER_H

#include <string>
#include "GoBoard.h"
#include "GoBoardSynchronizer.h"
#include "GoPlayerMove.h"
#include "board/GoBoardColor.h"
#include "board/GoPoint.h"

class SgNode;
class SgTimeRecord;
class GoPlayer : public GoBoardSynchronizer {
 public:
  explicit GoPlayer(const GoBoard& bd);
  virtual ~GoPlayer();
  GoBoard& Board();
  const GoBoard& Board() const;
  virtual GoPoint GenMove(const SgTimeRecord& time,
                          SgBlackWhite toPlay) = 0;
  virtual std::string Name() const;
  SgNode* CurrentNode() const;
  void ClearSearchTraces();
  SgNode* TransferSearchTraces();
  virtual int MoveValue(GoPoint p);
  virtual void OnGameFinished();
  virtual void OnNewGame();
  virtual void Ponder();
  int Variant() const;
  void SetVariant(int variant);

 protected:

  SgNode* m_currentNode;

 private:

  GoBoard m_bd;
  int m_variant;
  GoPlayer(const GoPlayer&);
  GoPlayer& operator=(const GoPlayer&);
};

inline GoBoard& GoPlayer::Board() {
  return m_bd;
}

inline const GoBoard& GoPlayer::Board() const {
  return m_bd;
}

inline SgNode* GoPlayer::CurrentNode() const {
  return m_currentNode;
}

inline int GoPlayer::Variant() const {
  return m_variant;
}

inline void GoPlayer::SetVariant(int variant) {
  m_variant = variant;
}

#endif

