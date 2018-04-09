

#include "platform/SgSystem.h"
#include "GoStaticLadder.h"

#include "GoBoard.h"

bool GoStaticLadder::IsEdgeLadder(const GoBoard& bd, GoPoint target,
                                  SgBlackWhite toPlay) {
  SgBlackWhite defender = bd.GetColor(target);
  int nuLibs = bd.NumLiberties(target);
  if (nuLibs > 2)
    return false;
  GoPoint attackPoint = GO_NULLMOVE;
  GoPoint defensePoint = GO_NULLMOVE;
  if (nuLibs == 1) {
    if (toPlay != defender)
      return true;
    GoPoint theLiberty = bd.TheLiberty(target);
    for (GoNbIterator it(bd, theLiberty); it; ++it)
      if (bd.IsEmpty(*it)) {
        if (attackPoint == GO_NULLMOVE)
          attackPoint = *it;
        else {
          DBG_ASSERT(defensePoint == GO_NULLMOVE);
          defensePoint = *it;
        }
      }
  } else {
    DBG_ASSERT(nuLibs == 2);
    if (toPlay == defender)
      return false;
    GoBoard::LibertyIterator it(bd, target);
    defensePoint = *it;
    ++it;
    attackPoint = *it;
  }
  if (bd.Line(defensePoint) != 1) {
    if (bd.Line(attackPoint) != 1)
      return false;
    std::swap(defensePoint, attackPoint);
  }
  int col = GoPointUtil::Col(defensePoint);
  int delta;
  switch (defensePoint - attackPoint) {
    case GO_NORTH_SOUTH + GO_WEST_EAST:delta = (col == bd.Size() ? GO_NORTH_SOUTH : GO_WEST_EAST);
      break;
    case GO_NORTH_SOUTH - GO_WEST_EAST:delta = (col == 1 ? GO_NORTH_SOUTH : -GO_WEST_EAST);
      break;
    case -GO_NORTH_SOUTH + GO_WEST_EAST:delta = (col == bd.Size() ? -GO_NORTH_SOUTH : GO_WEST_EAST);
      break;
    case -GO_NORTH_SOUTH - GO_WEST_EAST:delta = (col == 1 ? -GO_NORTH_SOUTH : -GO_WEST_EAST);
      break;
    default:return false;
  }
  while (true) {
    DBG_ASSERT(bd.IsEmpty(defensePoint));
    DBG_ASSERT(bd.IsEmpty(attackPoint));
    int nuNeighborsDefender = bd.NumNeighbors(defensePoint, defender);
    if (nuNeighborsDefender > 1)
      return false;
    if (nuNeighborsDefender == 0
        && bd.NumEmptyNeighbors(defensePoint) < 3)
      return true;
    defensePoint += delta;
    attackPoint += delta;
    if (!bd.IsEmpty(defensePoint) || !bd.IsEmpty(attackPoint))
      return false;
  }

  return true;
}

bool GoStaticLadder::IsLadder(const GoBoard& bd, GoPoint target,
                              SgBlackWhite toPlay) {
  return IsEdgeLadder(bd, target, toPlay);
}
