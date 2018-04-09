

#include "platform/SgSystem.h"
#include "GoOpeningKnowledge.h"

#include "GoBoard.h"
#include "platform/SgDebug.h"
namespace {

bool IsEmpty3x3Box(const GoBoard& bd, GoPoint p) {
  return bd.IsEmpty(p)
      && bd.Num8EmptyNeighbors(p) == 8;
}

int ScanSide(const GoBoard& bd, GoPoint start, int direction) {
  int dist = 0;
  for (GoPoint p = start + direction;
       bd.Pos(p) >= 3 && IsEmpty3x3Box(bd, p);
       p += direction
      )
    ++dist;
  return dist;
}

void Check(int x, int y, const GoBoard& bd,
           std::vector<GoPoint>& moves) {
  const GoPoint p = GoPointUtil::Pt(x, y);
  if (IsEmpty3x3Box(bd, p))
    moves.push_back(p);
}

void CheckStarPoint(int x, int y, const GoBoard& bd,
                    std::vector<GoPoint>& moves) {
  const GoPoint p = GoPointUtil::Pt(x, y);
  DBG_ASSERT(bd.Line(p) == 4);
  DBG_ASSERT(bd.Pos(p) == 4);
  if (!bd.IsEmpty(p)
      && bd.Num8EmptyNeighbors(p) == 8
      )
    for (int side = 0; side <= 1; ++side) {
      const GoPoint p1 = p - bd.Up(p) + 3 * bd.Side(p, side);
      DBG_ASSERT(bd.Line(p1) == 3);
      DBG_ASSERT(bd.Pos(p1) == 6);
      if (IsEmpty3x3Box(bd, p1))
        moves.push_back(p1);
    }
}

}

std::vector<GoPoint> GoOpeningKnowledge::FindCornerMoves(const GoBoard& bd) {
  const int size1 = bd.Size() + 1;
  std::vector<GoPoint> moves;
  for (int mirrorX = 0; mirrorX <= 1; ++mirrorX)
    for (int mirrorY = 0; mirrorY <= 1; ++mirrorY)
      for (int x = 3; x <= 5; ++x)
        for (int y = 3; y <= 5; ++y) {
          if (x != 5 || y != 5)
            Check(mirrorX ? size1 - x : x,
                  mirrorY ? size1 - y : y,
                  bd, moves);
          if (x == 4 && y == 4)
            CheckStarPoint(mirrorX ? size1 - x : x,
                           mirrorY ? size1 - y : y,
                           bd, moves);

        }
  return moves;
}

std::vector<GoOpeningKnowledge::MoveBonusPair>
GoOpeningKnowledge::FindSideExtensions(const GoBoard& bd) {
  std::vector<MoveBonusPair> extensions;
  const GoBoardConst& bc = bd.BoardConst();
  for (SgLineIterator it(bc, 3); it; ++it) {
    const GoPoint p = *it;
    if (IsEmpty3x3Box(bd, p)
        && bc.SideExtensions().Contains(p)
        ) {
      int leftSpace = ScanSide(bd, p, bc.Left(p));
      int rightSpace = ScanSide(bd, p, bc.Right(p));
      if (leftSpace >= 1 && rightSpace >= 1) {
        const int bonus = leftSpace + rightSpace
            + std::min(leftSpace, rightSpace);
        extensions.push_back(std::make_pair(p, bonus));
      }
    }
  }
  return extensions;
}
