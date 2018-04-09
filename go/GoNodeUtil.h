

#ifndef GO_NODEUTIL_H
#define GO_NODEUTIL_H

#include "GoKomi.h"
#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class GoBoard;
class SgNode;
namespace GoNodeUtil {

template<class BOARD>
SgNode* CreateRoot(const BOARD& board);
SgNode* CreatePosition(int boardSize, SgBlackWhite toPlay,
                       const SgVector<GoPoint>& bPoints,
                       const SgVector<GoPoint>& wPoints);
GoKomi GetKomi(const SgNode* node);
int GetHandicap(const SgNode* node);
int GetBoardSize(const SgNode* node);
}

template<class BOARD>
SgNode* GoNodeUtil::CreateRoot(const BOARD& board) {
  SgBWArray<SgVector<GoPoint> > pointList;
  for (typename BOARD::Iterator it(board); it; ++it) {
    if (board.Occupied(*it))
      pointList[board.GetColor(*it)].PushBack(*it);
  }
  return CreatePosition(board.Size(), board.ToPlay(),
                        pointList[SG_BLACK], pointList[SG_WHITE]);
}

#endif

