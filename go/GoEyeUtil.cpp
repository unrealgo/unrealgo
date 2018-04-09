

#include "platform/SgSystem.h"
#include "GoEyeUtil.h"

#include "GoBoardUtil.h"
#include "GoEyeCount.h"
#include "board/GoNbIterator.h"
namespace {

static const bool WRITE_NAKADE_WARNING_MESSAGES = false;

int NuEdgePoints(const GoBoard& bd, const GoPointSet& points) {
  return (points & bd.LineSet(1)).Size();
}

bool IsAliveBlock(const GoPointSet& block) {
  const int code = GoEyeUtil::DegreeCode(block);

  return code == 220
      || code == 320
      || code == 1130
      || code == 2400
      || code == 2220;
}

bool CheckAlwaysAlive8Code(long code) {
  return code == 11210
      || code == 3110;
}

bool IsAlwaysAliveBlock(const GoPointSet& block) {
  const int code = GoEyeUtil::DegreeCode(block);

  return code == 320
      || (code == 1130
          && CheckAlwaysAlive8Code(GoEyeUtil::DegreeCode8(block))
      );
}

bool IsNakadeBlock(const GoBoard& bd,
                   const GoPointSet& block) {
  if (!GoEyeUtil::IsNakadeShape(block))
    return false;
  return bd.NumStones(block.PointOf()) == block.Size();
}

bool AlmostFilledByLivingShape(const GoBoard& bd,
                               const GoPointSet& points,
                               SgBlackWhite stoneColor) {
  DBG_ASSERT(points.Border(bd.Size()).SubsetOf(bd.All(SgOppBW(stoneColor))));

  return (points & bd.AllEmpty()).IsSize(1)
      && IsAliveBlock(points & bd.All(stoneColor));
}

bool ContainsLivingShape(const GoBoard& bd,
                         const GoPointSet& points,
                         SgBlackWhite stoneColor) {
  DBG_ASSERT(points.Border(bd.Size()).SubsetOf(bd.All(SgOppBW(stoneColor))));

  return IsAlwaysAliveBlock(points & bd.All(stoneColor));
}

bool AlmostFilledByNakade(const GoBoard& bd,
                          const GoPointSet& points,
                          SgBlackWhite stoneColor) {
  DBG_ASSERT(points.Border(bd.Size()).SubsetOf(bd.All(SgOppBW(stoneColor))));

  if ((points & bd.AllEmpty()).IsSize(1)) {
    return IsNakadeBlock(bd, points & bd.All(stoneColor));
  }
  return false;
}

GoEyeStatus BulkyFiveNakade(const GoBoard& bd,
                            const GoPointSet& points,
                            SgBlackWhite stoneColor) {
  const GoPointSet stones = points & bd.All(stoneColor);
  if (IsBulkyFive(points)
      && stones.IsSize(3)
      ) {
    const GoPoint p = stones.PointOf();
    if (bd.NumStones(p) == 3) {
      DBG_ASSERT((points & bd.AllEmpty()).IsSize(2));
      for (SgSetIterator it(points & bd.AllEmpty()); it; ++it) {
        if (bd.NumNeighbors(*it, stoneColor) == 0) {
          return EYE_ONE_AND_HALF;
        }
      }
      return EYE_ONE;
    } else {
      return GoEyeUtil::DegreeCode(stones) == 3 ?
             EYE_ONE_AND_HALF :
             EYE_TWO;
    }

  }
  return EYE_UNKNOWN;
}

GoEyeStatus Special2x3Cases(const GoBoard& bd,
                            const GoPointSet& points,
                            SgBlackWhite stoneColor) {
  const GoPointSet stones = points & bd.All(stoneColor);
  const int nuStones = stones.Size();
  if (nuStones > 0) {
    const int code = GoEyeUtil::DegreeCode(stones);
    const long code8 = GoEyeUtil::DegreeCode8(stones);
    if (code == 220 && code8 == 2200)
      return EYE_ONE_AND_HALF;
    if (code == 21 && code8 == 120)
      return EYE_TWO;
    if (NuEdgePoints(bd, points) == 3) {
      const GoPoint p = stones.PointOf();
      switch (nuStones) {
        case 1:
          if (bd.Line(p) == 1
              && bd.HasNeighbors(p, SgOppBW(stoneColor)))
            return EYE_ONE_AND_HALF;
          break;
        case 2:
          if (bd.Line(p) == 1
              && code == 2
              && NuEdgePoints(bd, stones) == 2
              )
            return EYE_ONE_AND_HALF;
          if (code == 2
              && code8 == 20
              ) {
            const GoPoint p1 = (stones & bd.LineSet(1)).PointOf();
            if (bd.HasNeighbors(p1, SgOppBW(stoneColor)))
              return EYE_ONE;
          }
          break;
        case 3 :
          if (code == 3
              && code8 == 120
              ) {
            const GoPoint p1 = (stones & bd.LineSet(1)).PointOf();
            if (bd.HasNeighbors(p1, SgOppBW(stoneColor)))
              return EYE_ONE;
          }
          break;
      }
    }
  }
  return EYE_UNKNOWN;
}

inline bool TestDiagonal(const GoPointSet& set, GoPoint p, int ns, int we) {
  return !set[p + ns + we]
      && !(set[p + ns - we]
          && set[p - we]
          && set[p - ns - we]
          && set[p - ns]
          && set[p - ns + we]
      );
}

inline bool TestOpposite(const GoPointSet& set, GoPoint p, int ns, int we) {
  return
      !(set[p - ns - we] && set[p - we] && set[p + ns - we])
          && !(set[p - ns + we] && set[p + we] && set[p + ns + we]);
}

bool IsBentFour(const GoPointSet& points, int boardSize, GoPoint* vital) {
  DBG_ASSERT(points.IsSize(4));

  if (points.Contains(GoPointUtil::Pt(1, 1))
      && points.Contains(GoPointUtil::Pt(2, 1))
      && points.Contains(GoPointUtil::Pt(1, 2))
      ) {
    if (points.Contains(GoPointUtil::Pt(3, 1))) {
      *vital = GoPointUtil::Pt(2, 1);
      return true;
    } else if (points.Contains(GoPointUtil::Pt(1, 3))) {
      *vital = GoPointUtil::Pt(1, 2);
      return true;
    }
  }
  if (points.Contains(GoPointUtil::Pt(1, boardSize))
      && points.Contains(GoPointUtil::Pt(2, boardSize))
      && points.Contains(GoPointUtil::Pt(1, boardSize - 1))
      ) {
    if (points.Contains(GoPointUtil::Pt(3, boardSize))) {
      *vital = GoPointUtil::Pt(2, boardSize);
      return true;
    } else if (points.Contains(GoPointUtil::Pt(1, boardSize - 2))) {
      *vital = GoPointUtil::Pt(1, boardSize - 1);
      return true;
    }
  }
  if (points.Contains(GoPointUtil::Pt(boardSize, 1))
      && points.Contains(GoPointUtil::Pt(boardSize - 1, 1))
      && points.Contains(GoPointUtil::Pt(boardSize, 2))
      ) {
    if (points.Contains(GoPointUtil::Pt(boardSize - 2, 1))) {
      *vital = GoPointUtil::Pt(boardSize - 1, 1);
      return true;
    } else if (points.Contains(GoPointUtil::Pt(boardSize, 3))) {
      *vital = GoPointUtil::Pt(boardSize, 2);
      return true;
    }
  }
  if (points.Contains(GoPointUtil::Pt(boardSize, boardSize))
      && points.Contains(GoPointUtil::Pt(boardSize - 1, boardSize))
      && points.Contains(GoPointUtil::Pt(boardSize, boardSize - 1))
      ) {
    if (points.Contains(GoPointUtil::Pt(boardSize - 2, boardSize))) {
      *vital = GoPointUtil::Pt(boardSize - 1, boardSize);
      return true;
    } else if (points.Contains(GoPointUtil::Pt(boardSize, boardSize - 2))) {
      *vital = GoPointUtil::Pt(boardSize, boardSize - 1);
      return true;
    }
  }

  return false;
}

bool TwoDiagonalStonesInBulkyFour(const GoBoard& bd,
                                  const GoPointSet& points,
                                  SgBlackWhite stoneColor) {
  DBG_ASSERT(points.Border(bd.Size()).SubsetOf(bd.All(SgOppBW(stoneColor))));

  if ((points & bd.All(stoneColor)).IsSize(2)) {
    DBG_ASSERT((points & bd.AllEmpty()).IsSize(2));

    const GoPoint p = points.PointOf();
    if (bd.IsEmpty(p))
      return bd.NumNeighbors(p, stoneColor) == 2;
    else
      return bd.NumEmptyNeighbors(p) == 2;
  }
  return false;
}

inline bool ProcessStatus(GoEyeStatus status,
                          bool& isNakade,
                          bool& makeNakade) {
  if (status == EYE_ONE)
    isNakade = true;
  else if (status == EYE_ONE_AND_HALF)
    makeNakade = true;
  return status != EYE_UNKNOWN;
}
}

int GoEyeUtil::DegreeCode(const GoPointSet& points) {
  int degrees[5] = {0, 0, 0, 0, 0};

  for (SgSetIterator pit(points); pit; ++pit) {
    const GoPoint p(*pit);
    int nbs = 0;
    for (SgNb4Iterator it(p); it; ++it) {
      if (points.Contains(*it))
        ++nbs;
    }
    ++(degrees[nbs]);
  }
  return degrees[0]
      + 10 * degrees[1]
      + 100 * degrees[2]
      + 1000 * degrees[3]
      + 10000 * degrees[4];
}

long GoEyeUtil::DegreeCode8(const GoPointSet& points) {
  int degrees[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (SgSetIterator pit(points); pit; ++pit) {
    const GoPoint p(*pit);
    int nbs = 0;
    for (SgNb8Iterator it(p); it; ++it) {
      if (points.Contains(*it))
        ++nbs;
    }
    ++(degrees[nbs]);
  }
  return degrees[0]
      + 10 * degrees[1]
      + 100 * degrees[2]
      + 1000 * degrees[3]
      + 10000 * degrees[4]
      + 100000 * degrees[5]
      + 1000000 * degrees[6]
      + 10000000 * degrees[7]
      + 100000000 * degrees[8];
}

bool GoEyeUtil::IsSinglePointEye(const GoBoard& bd, GoPoint p,
                                 SgBlackWhite color) {
  DBG_ASSERT(bd.IsEmpty(p));
  const SgBlackWhite opp = SgOppBW(color);
  if (bd.HasEmptyNeighbors(p) || bd.HasNeighbors(p, opp))
    return false;
  if (bd.Line(p) == 1)
    return !(bd.HasDiagonals(p, SG_EMPTY) || bd.HasDiagonals(p, opp));
  return (bd.NumDiagonals(p, SG_EMPTY) + bd.NumDiagonals(p, opp)) <= 1;
}

bool GoEyeUtil::IsPossibleEye(const GoBoard& board, SgBlackWhite color,
                              GoPoint p) {
  bool isPossibleEye = false;
  DBG_ASSERT(board.GetColor(p) != color);
  const SgBlackWhite opp = SgOppBW(color);
  if (board.Line(p) == 1) {
    const int nuOwn = (board.Pos(p) == 1) ? 2 : 4;
    if (board.Num8Neighbors(p, color) == nuOwn
        && board.Num8EmptyNeighbors(p) == 1
        ) {
      isPossibleEye = true;
    }
  } else {
    if (board.NumNeighbors(p, color) == 4
        && board.NumDiagonals(p, color) == 2
        && board.NumEmptyDiagonals(p) > 0
        ) {
      isPossibleEye = true;
    } else if (board.NumNeighbors(p, color) == 3
        && board.NumNeighbors(p, opp) == 0
        && board.NumDiagonals(p, color) >= 3
        ) {
      isPossibleEye = true;
    }
  }

  return isPossibleEye;
}

bool GoEyeUtil::NumberOfMoveToEye(const GoBoard& board, SgBlackWhite color,
                                  GoPoint p, int& number) {
  DBG_ASSERT(board.IsEmpty(p));
  SgBlackWhite att = SgOppBW(color);

  if (board.Line(p) == 1) {
    if (board.Num8Neighbors(p, att) > 0)
      return false;
    else {
      number = board.Num8EmptyNeighbors(p);
      return true;
    }
  } else {
    if (board.Num8Neighbors(p, att) >= 2)
      return false;
    else if (board.NumNeighbors(p, att) > 0)
      return false;
    else {
      number = board.Num8EmptyNeighbors(p);
      return true;
    }
  }

}

bool GoEyeUtil::IsSinglePointEye2(const GoBoard& board, GoPoint p,
                                  SgBlackWhite color, SgVector<GoPoint>& eyes) {
  if (!board.IsColor(p, SG_EMPTY))
    return false;
  SgBoardColor opp = SgOppBW(color);
  for (GoNbIterator adj(board, p); adj; ++adj) {
    SgBoardColor adjColor = board.GetColor(*adj);
    if (adjColor == opp || adjColor == SG_EMPTY)
      return false;
  }
  int baddiags = 0;
  int maxbaddiags = (board.Line(p) == 1 ? 0 : 1);
  for (SgNb4DiagIterator it(p); it; ++it) {
    if (board.IsColor(*it, opp))
      ++baddiags;
    if (board.IsColor(*it, SG_EMPTY) && !eyes.Contains(*it)) {
      eyes.PushBack(p);
      if (!IsSinglePointEye2(board, *it, color, eyes))
        ++baddiags;
      eyes.PopBack();
    }
    if (baddiags > maxbaddiags)
      return false;
  }
  return true;
}

bool GoEyeUtil::IsSinglePointEye2(const GoBoard& board, GoPoint p,
                                  SgBlackWhite color) {
  SgVector<GoPoint> emptylist;
  return IsSinglePointEye2(board, p, color, emptylist);
}

bool GoEyeUtil::NumberOfMoveToEye2(const GoBoard& board, SgBlackWhite color,
                                   GoPoint p, int& nummoves) {
  nummoves = 0;
  bool capturing = false;
  SgVector<GoPoint> usedpoints;
  usedpoints.PushBack(p);
  GoPointSet counted;
  if (board.IsColor(p, color))
    return false;
  if (board.IsColor(p, SgOppBW(color))) {
    capturing = true;
    if (SinglePointSafe2(board, p))
      return false;

    for (GoBoard::LibertyIterator libit(board, p); libit; ++libit)
      counted.Include(*libit);
  }
  for (GoNbIterator nb(board, p); nb; ++nb) {
    GoPoint adj = *nb;
    if (board.IsColor(adj, SG_EMPTY))
      counted.Include(adj);
    else if (board.IsColor(adj, SgOppBW(color))) {
      if (capturing)
        counted.Include(adj);
      else
        return false;
    }
  }
  GoPoint toignore = GO_NULLPOINT;
  int maxcost = 0;
  int infcost = 1000;
  if (board.Line(p) > 1) {
    for (SgNb4DiagIterator nbd(p); nbd; ++nbd) {
      GoPoint diag = *nbd;
      int cost = 0;

      if (board.IsColor(diag, SG_EMPTY)
          && !IsSinglePointEye2(board, diag, color, usedpoints)) {
        cost = 1;
      } else if (board.IsColor(diag, SgOppBW(color))) {
        if (SinglePointSafe2(board, diag))
          cost = infcost;
        else
          cost = board.NumLiberties(diag);
      }

      if (cost > maxcost) {
        maxcost = cost;
        toignore = diag;
      }
    }
  }
  for (SgNb4DiagIterator nbd(p); nbd; ++nbd) {
    GoPoint diag = *nbd;
    if (diag == toignore)
      continue;
    if (board.IsColor(diag, SG_EMPTY)
        && !IsSinglePointEye2(board, diag, color, usedpoints)) {
      counted.Include(diag);
    } else if (board.IsColor(diag, SgOppBW(color))) {
      if (SinglePointSafe2(board, diag))
        return false;
      else {
        counted.Include(diag);
        for (GoBoard::LibertyIterator libit(board, diag); libit;
             ++libit)
          counted.Include(*libit);
      }
    }
  }

  nummoves = counted.Size();
  return true;
}

int GoEyeUtil::CountSinglePointEyes2(const GoBoard& board, GoPoint p) {
  if (!board.Occupied(p))
    return 0;

  SgBlackWhite color = board.GetColor(p);
  int numeyes = 0;

  for (GoBoard::LibertyIterator lib(board, p); lib; ++lib) {
    if (IsSinglePointEye2(board, *lib, color))
      numeyes++;
  }

  return numeyes;
}

bool GoEyeUtil::SinglePointSafe2(const GoBoard& board, GoPoint p) {
  int numeyes = CountSinglePointEyes2(board, p);
  return numeyes >= 2;
}

bool GoEyeUtil::IsLocalSplitPt(GoPoint p, const GoPointSet& set) {
  int nuNb = 0;
  for (SgNb4Iterator it(p); it; ++it) {
    if (set.Contains(*it)) {
      ++nuNb;
      if (nuNb >= 2)
        break;
    }
  }
  if (nuNb >= 2) {
    if (set[p - GO_NORTH_SOUTH]) {
      if (set[p - GO_WEST_EAST] && TestDiagonal(set, p, -GO_NORTH_SOUTH, -GO_WEST_EAST))
        return true;
      if (set[p + GO_NORTH_SOUTH] && TestOpposite(set, p, GO_NORTH_SOUTH, GO_WEST_EAST))
        return true;
      if (set[p + GO_WEST_EAST] && TestDiagonal(set, p, -GO_NORTH_SOUTH, +GO_WEST_EAST))
        return true;
    }
    if (set[p + GO_NORTH_SOUTH]) {
      if (set[p - GO_WEST_EAST] && TestDiagonal(set, p, +GO_NORTH_SOUTH, -GO_WEST_EAST))
        return true;
      if (set[p + GO_WEST_EAST] && TestDiagonal(set, p, +GO_NORTH_SOUTH, +GO_WEST_EAST))
        return true;
    }
    if (set[p - GO_WEST_EAST] && set[p + GO_WEST_EAST]
        && TestOpposite(set, p, GO_WEST_EAST, GO_NORTH_SOUTH))
      return true;
  }
  return false;
}

bool GoEyeUtil::IsSplitPt(GoPoint p, const GoPointSet& points) {
  DBG_ASSERT(points[p]);
  if (!IsLocalSplitPt(p, points))
    return false;
  GoPointSet s(points);
  s.Exclude(p);
  return !s.IsConnected();
}

bool GoEyeUtil::CanBecomeSinglePointEye(const GoBoard& board, GoPoint p,
                                        const GoPointSet& oppSafe) {
  DBG_ASSERT(!oppSafe[p]);

  for (GoNbIterator it(board, p); it; ++it) {
    if (oppSafe[*it])
      return false;
  }

  int nu = 0;
  for (SgNb4DiagIterator dit(p); dit; ++dit) {
    if (oppSafe[*dit]) {
      if (board.Line(p) == 1)
        return false;
      ++nu;
      if (nu > 1)
        return false;
    }
  }

  return true;
}

void GoEyeUtil::TestNakade(const GoPointSet& points,
                           const GoBoard& bd,
                           SgBlackWhite color,
                           bool isFullyEnclosed,
                           bool& isNakade,
                           bool& makeNakade,
                           bool& makeFalse,
                           bool& maybeSeki,
                           bool& sureSeki,
                           GoPoint* vital) {

  SuppressUnused(makeFalse);
  isNakade = makeNakade = maybeSeki = sureSeki = false;
  GoPoint vitalP(GO_NULLPOINT);
  const SgBlackWhite opp = SgOppBW(color);
  const int nuPoints = points.Size();

  DBG_ASSERT(nuPoints >= 3);

  if (nuPoints == 4) {
    if (IsBulkyFour(points)) {
      if (isFullyEnclosed
          && TwoDiagonalStonesInBulkyFour(bd, points, opp)
          )
        makeNakade = true;
      else
        isNakade = true;
      return;
    } else if (isFullyEnclosed
        && points.SubsetOf(bd.AllEmpty())
        && IsBentFour(points, bd.Size(), vital)
        ) {
      makeNakade = true;
      return;
    }
  } else if (isFullyEnclosed && nuPoints == 5) {
    const GoEyeStatus status = BulkyFiveNakade(bd, points, opp);
    if (ProcessStatus(status, isNakade, makeNakade))
      return;
  } else if (isFullyEnclosed && nuPoints == 6) {
    if (Is2x3Area(points)) {
      GoEyeStatus status = Special2x3Cases(bd, points, opp);
      if (ProcessStatus(status, isNakade, makeNakade))
        return;
    }
  }
  if (isFullyEnclosed
      && AlmostFilledByNakade(bd, points, opp)
      ) {
    if (WRITE_NAKADE_WARNING_MESSAGES)
      SgDebug() << "AlmostFilledByNakade: isNakade = true\n";
    isNakade = true;
    return;
  }

  if (isFullyEnclosed
      && (AlmostFilledByLivingShape(bd, points, opp)
          || ContainsLivingShape(bd, points, opp)
      )
      ) {
    return;
  }

  int nuMakeNakade = 0;
  int nuVitalOccupied = 0;
  bool hasDivider = false;
  for (SgSetIterator it(points); it; ++it) {
    GoPoint p(*it);
    if (IsVitalPt(points, p, opp, bd)) {
      if (bd.IsEmpty(p)) {
        if (bd.IsLegal(p, opp)) {
          ++nuMakeNakade;
          vitalP = p;
        } else
          hasDivider = true;
      } else
        ++nuVitalOccupied;
    }
  }

  if (hasDivider) {
  } else if (nuMakeNakade == 1) {
    makeNakade = true;
    *vital = vitalP;
  } else if (nuMakeNakade > 0)
    isNakade = false;
  else if (nuVitalOccupied < 3 && nuPoints <= 6) {
    if (WRITE_NAKADE_WARNING_MESSAGES)
      SgDebug() << "nuVitalOccupied < 3: isNakade = true\n";
    isNakade = true;
  } else {
    maybeSeki = true;
  }
}

bool GoEyeUtil::IsVitalPt(const GoPointSet& points, GoPoint p,
                          SgBlackWhite opp,
                          const GoBoard& bd) {
  int numNb = bd.NumEmptyNeighbors(p) + bd.NumNeighbors(p, opp);
  if (numNb >= 2) {
    if (numNb >= 4)
      return true;
    int nu = IsTreeShape(points) ? 2 : 3;
    if (numNb >= nu) {
      if (numNb == 2 && bd.IsEmpty(p))
        return IsSplitPt(p, points);
      else
        return true;
    }
  }
  return false;
}

bool GoEyeUtil::CheckInterior(const GoBoard& bd, const GoPointSet& area,
                              SgBlackWhite opp, bool checkBlocks) {
  bool hasSingleNbPoint = false;
  int nuPoints = 0;
  for (SgSetIterator it(area); it; ++it) {
    const GoPoint p(*it);
    if (bd.IsEmpty(p)) {
      int nuNbs = 0;
      if (area.Contains(p + GO_NORTH_SOUTH))
        ++nuNbs;
      if (area.Contains(p - GO_NORTH_SOUTH))
        ++nuNbs;
      if (area.Contains(p + GO_WEST_EAST))
        ++nuNbs;
      if (area.Contains(p - GO_WEST_EAST))
        ++nuNbs;
      if (nuNbs == 1)
        hasSingleNbPoint = true;
      else if (nuNbs > 2)
        return false;
    } else if (p == bd.Anchor(p)) {
      if (bd.GetColor(p) != opp)
        return false;
      int nuLibs = bd.NumLiberties(p);
      if (nuLibs == 1)
        hasSingleNbPoint = true;
      else if (checkBlocks && nuLibs > 2)
        return false;
    }
    ++nuPoints;
  }
  return nuPoints == 1 || hasSingleNbPoint;
}

bool GoEyeUtil::IsTreeShape(const GoPointSet& area) {
  for (SgSetIterator it(area); it; ++it) {
    const GoPoint p(*it);
    if (area.Contains(p + GO_NORTH_SOUTH)
        && area.Contains(p + GO_WEST_EAST)
        && area.Contains(p + GO_NORTH_SOUTH + GO_WEST_EAST)
        )
      return false;
  }
  return true;
}
