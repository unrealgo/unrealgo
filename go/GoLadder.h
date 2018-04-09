

#ifndef GO_LADDER_H
#define GO_LADDER_H

#include "GoBoard.h"
#include "board/GoBoardColor.h"
#include "GoModBoard.h"
#include "board/GoPoint.h"
#include "board/GoPointSet.h"
#include "lib/SgVector.h"

enum GoLadderStatus {
  GO_LADDER_UNKNOWN,
  GO_LADDER_CAPTURED,
  GO_LADDER_UNSETTLED,
  GO_LADDER_ESCAPED
};
class GoLadder {
 public:
  GoLadder();
  int Ladder(const GoBoard& bd, GoPoint prey, SgBlackWhite toPlay,
             SgVector<GoPoint>* sequence, bool twoLibIsEscape = false);

 private:
  static const int MAX_LADDER_MOVES = 200;
  int m_maxMoveNumber;
  GoBoard* m_bd;
  GoPointSet m_partOfPrey;
  SgBlackWhite m_preyColor;
  SgBlackWhite m_hunterColor;
  bool CheckMoveOverflow() const;
  void InitMaxMoveNumber();
  bool PointIsAdjToPrey(GoPoint p);
  bool BlockIsAdjToPrey(GoPoint p, int numAdj);
  void MarkStonesAsPrey(GoPoint p, SgVector<GoPoint>* stones = 0);
  void FilterAdjacent(GoPointList& adjBlocks);
  int PlayHunterMove(int depth, GoPoint move, GoPoint lib1, GoPoint lib2,
                     const GoPointList& adjBlk,
                     SgVector<GoPoint>* sequence);
  int PlayPreyMove(int depth, GoPoint move, GoPoint lib1,
                   const GoPointList& adjBlk, SgVector<GoPoint>* sequence);
  bool IsSnapback(GoPoint prey);
  int PreyLadder(int depth, GoPoint lib1, const GoPointList& adjBlk,
                 SgVector<GoPoint>* sequence);
  int HunterLadder(int depth, GoPoint lib1, const GoPointList& adjBlk,
                   SgVector<GoPoint>* sequence);
  int HunterLadder(int depth, GoPoint lib1, GoPoint lib2,
                   const GoPointList& adjBlk, SgVector<GoPoint>* sequence);
  void ReduceToBlocks(GoPointList& stones);
};
namespace GoLadderUtil {

bool Ladder(const GoBoard& board, GoPoint prey, SgBlackWhite toPlay,
            bool fTwoLibIsEscape = false, SgVector<GoPoint>* sequence = 0);
GoLadderStatus LadderStatus(const GoBoard& bd, GoPoint prey,
                            bool fTwoLibIsEscape = false,
                            GoPoint* toCapture = 0, GoPoint* toEscape = 0);
bool IsProtectedLiberty(const GoBoard& bd, GoPoint liberty, SgBlackWhite col,
                        bool& byLadder, bool& isKoCut, bool tryLadder = true);
bool IsProtectedLiberty(const GoBoard& bd, GoPoint liberty, SgBlackWhite col);
GoPoint TryLadder(const GoBoard& bd, GoPoint prey, SgBlackWhite firstPlayer);
bool IsLadderCaptureMove(const GoBoard& bd, GoPoint prey, GoPoint firstMove);
bool IsLadderEscapeMove(const GoBoard& bd, GoPoint prey, GoPoint firstMove);
void FindLadderEscapeMoves(const GoBoard& bd, GoPoint prey,
                           SgVector<GoPoint>& escapeMoves);

}

#endif

