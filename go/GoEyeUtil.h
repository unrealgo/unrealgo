

#ifndef GO_EYEUTIL_H
#define GO_EYEUTIL_H

#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "board/GoPoint.h"

namespace GoEyeUtil {

const int NAKADE_LIMIT = 6;
bool CanBecomeSinglePointEye(const GoBoard& bd, GoPoint p,
                             const GoPointSet& oppSafe);
template<class BOARD>
bool CanMakeEye(const BOARD& bd, SgBlackWhite color,
                GoPoint p, GoPoint& move);
int DegreeCode(const GoPointSet& points);
long DegreeCode8(const GoPointSet& points);
template<class BOARD>
GoPoint EmptyNeighbor(const BOARD& bd, GoPoint p);
bool IsNakadeShape(const GoPointSet& area, int nuPoints);
bool IsNakadeShape(const GoPointSet& area);
bool IsPossibleEye(const GoBoard& bd, SgBlackWhite color, GoPoint p);
bool IsSimpleEye(const GoBoard& bd, GoPoint p, SgBlackWhite c);
bool IsSinglePointEye(const GoBoard& bd, GoPoint p, SgBlackWhite c);
template<class BOARD>
bool MakesNakadeShape(const BOARD& bd, GoPoint p,
                      SgBlackWhite toPlay);
bool NumberOfMoveToEye(const GoBoard& bd, SgBlackWhite c, GoPoint p,
                       int& number);
bool IsSinglePointEye2(const GoBoard& bd, GoPoint p, SgBlackWhite c);
bool IsSinglePointEye2(const GoBoard& bd, GoPoint p,
                       SgBlackWhite c, SgVector<GoPoint>& eyes);
template<class BOARD>
bool IsTwoPointEye(const BOARD& bd, GoPoint p,
                   SgBlackWhite c);
bool NumberOfMoveToEye2(const GoBoard& bd, SgBlackWhite c,
                        GoPoint p, int& nummoves);
int CountSinglePointEyes2(const GoBoard& bd, GoPoint p);
bool SinglePointSafe2(const GoBoard& bd, GoPoint p);
bool IsSplitPt(GoPoint p, const GoPointSet& s);
bool IsLocalSplitPt(GoPoint p, const GoPointSet& set);
bool IsTreeShape(const GoPointSet& area);
bool IsVitalPt(const GoPointSet& points, GoPoint p, SgBlackWhite opp,
               const GoBoard& bd);
void TestNakade(const GoPointSet& points, const GoBoard& bd,
                SgBlackWhite color, bool isFullyEnclosed, bool& isNakade,
                bool& makeNakade, bool& makeFalse, bool& maybeSeki,
                bool& sureSeki, GoPoint* vital);
bool CheckInterior(const GoBoard& bd, const GoPointSet& area,
                   SgBlackWhite opp, bool checkBlocks);
}

inline bool AreSameBlocks(const GoPoint anchors1[], const GoPoint anchors2[]) {
  int i = 0;
  for (; anchors1[i] != GO_ENDPOINT; ++i) {
    if (anchors2[i] == GO_ENDPOINT)
      return false;
    if (!GoBoardUtil::ContainsAnchor(anchors2, anchors1[i]))
      return false;
  }
  return anchors2[i] == GO_ENDPOINT;
}

inline bool IsBulkyFour(const GoPointSet& points) {
  DBG_ASSERT(points.IsSize(4));
  SgSetIterator it(points);
  GoPoint p1 = *it;
  ++it;
  if (*it != p1 + GO_WEST_EAST)
    return false;
  ++it;
  if (*it != p1 + GO_NORTH_SOUTH)
    return false;
  ++it;
  if (*it != p1 + GO_WEST_EAST + GO_NORTH_SOUTH)
    return false;
  DBG_ASSERT(GoEyeUtil::DegreeCode(points) == 400);
  return true;
}

inline bool IsTShape(const GoPointSet& block) {
  return GoEyeUtil::DegreeCode(block) == 1030;
}

inline bool IsBulkyFive(const GoPointSet& block) {
  return GoEyeUtil::DegreeCode(block) == 1310;
}

inline bool IsCross(const GoPointSet& block) {
  return GoEyeUtil::DegreeCode(block) == 10040;
}

inline bool IsRabbitySix(const GoPointSet& block) {
  return GoEyeUtil::DegreeCode(block) == 10320;
}

inline bool Is2x3Area(const GoPointSet& area) {
  return GoEyeUtil::DegreeCode(area) == 2400;
}

template<class BOARD>
GoPoint GoEyeUtil::EmptyNeighbor(const BOARD& bd, GoPoint p) {
  if (bd.IsEmpty(p + GO_NORTH_SOUTH))
    return p + GO_NORTH_SOUTH;
  if (bd.IsEmpty(p - GO_NORTH_SOUTH))
    return p - GO_NORTH_SOUTH;
  if (bd.IsEmpty(p + GO_WEST_EAST))
    return p + GO_WEST_EAST;
  if (bd.IsEmpty(p - GO_WEST_EAST))
    return p - GO_WEST_EAST;
  DBG_ASSERT(false);
  return GO_NULLPOINT;
}

inline bool GoEyeUtil::IsNakadeShape(const GoPointSet& area, int nuPoints) {
  switch (nuPoints) {
    case 1:
    case 2:
    case 3:return true;
    case 4:return IsBulkyFour(area) || IsTShape(area);
    case 5:return IsBulkyFive(area) || IsCross(area);
    case 6:return IsRabbitySix(area);
    default:return false;
  }
}

inline bool GoEyeUtil::IsNakadeShape(const GoPointSet& area) {
  return IsNakadeShape(area, area.Size());
}

inline bool GoEyeUtil::IsSimpleEye(const GoBoard& bd, GoPoint p,
                                   SgBlackWhite c) {

  SgBlackWhite opp = SgOppBW(c);
  if (bd.HasEmptyNeighbors(p) || bd.HasNeighbors(p, opp))
    return false;
  GoArrayList<GoPoint, 2> anchors;
  for (GoNbIterator it(bd, p); it; ++it) {
    GoPoint nbPoint = *it;
    DBG_ASSERT(bd.IsColor(nbPoint, c));
    GoPoint nbAnchor = bd.Anchor(nbPoint);
    if (!anchors.Contains(nbAnchor)) {
      if (anchors.Length() > 1)
        return false;
      anchors.PushBack(nbAnchor);
    }
  }
  if (anchors.Length() == 1)
    return true;
  for (GoBoard::LibertyIterator it(bd, anchors[0]); it; ++it) {
    GoPoint lib = *it;
    if (lib == p)
      continue;
    bool isSecondSharedEye = true;
    GoArrayList<GoPoint, 2> foundAnchors;
    for (GoNbIterator it2(bd, lib); it2; ++it2) {
      GoPoint nbPoint = *it2;
      if (bd.GetColor(nbPoint) != c) {
        isSecondSharedEye = false;
        break;
      }
      GoPoint nbAnchor = bd.Anchor(nbPoint);
      if (!anchors.Contains(nbAnchor)) {
        isSecondSharedEye = false;
        break;
      }
      if (!foundAnchors.Contains(nbAnchor))
        foundAnchors.PushBack(nbAnchor);
    }
    if (isSecondSharedEye && foundAnchors.Length() == 2)
      return true;
  }
  return false;
}

template<class BOARD>
bool GoEyeUtil::IsTwoPointEye(const BOARD& bd, GoPoint p,
                              SgBlackWhite color) {
  const SgBlackWhite opp = SgOppBW(color);
  if (bd.NumEmptyNeighbors(p) == 1
      && bd.NumNeighbors(p, opp) == 0
      ) {
    const GoPoint p2 = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
    if (bd.NumEmptyNeighbors(p2) == 1
        && bd.NumNeighbors(p2, opp) == 0
        ) {
      GoPoint nbanchorp[4 + 1];
      GoPoint nbanchorp2[4 + 1];
      bd.NeighborBlocks(p, color, nbanchorp);
      bd.NeighborBlocks(p2, color, nbanchorp2);
      DBG_ASSERT(nbanchorp[0] != GO_ENDPOINT);
      DBG_ASSERT(nbanchorp2[0] != GO_ENDPOINT);
      return ::AreSameBlocks(nbanchorp, nbanchorp2);
    }
  }
  return false;
}

template<class BOARD>
bool GoEyeUtil::MakesNakadeShape(const BOARD& bd, GoPoint move,
                                 SgBlackWhite toPlay) {
  GoPointSet area;
  area.Include(move);
  int nu = 1;
  SgVector<GoPoint> toProcess;
  toProcess.PushBack(move);
  while (toProcess.NonEmpty()) {
    GoPoint p = toProcess.Back();
    toProcess.PopBack();
    for (GoNb4Iterator<BOARD> it(bd, p); it; ++it)
      if (bd.IsColor(*it, toPlay) && !area.Contains(*it)) {
        area.Include(*it);
        toProcess.PushBack(*it);
        if (++nu > NAKADE_LIMIT)
          return false;
      }
  }
  return IsNakadeShape(area, nu);
}

template<class BOARD>
bool GoEyeUtil::CanMakeEye(const BOARD& bd, SgBlackWhite color,
                           GoPoint p, GoPoint& move) {
  bool isPossibleEye = false;
  DBG_ASSERT(bd.GetColor(p) != color);
  const SgBlackWhite opp = SgOppBW(color);
  if (bd.Line(p) == 1) {
    const int nuOwn = (bd.Pos(p) == 1) ? 2 : 4;
    if (bd.Num8Neighbors(p, color) == nuOwn
        && bd.Num8EmptyNeighbors(p) == 1
        ) {
      isPossibleEye = true;
      move = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
    }
  } else {
    if (bd.NumNeighbors(p, color) == 4
        && bd.NumDiagonals(p, color) == 2
        && bd.NumEmptyDiagonals(p) > 0
        ) {
      isPossibleEye = true;
      move = GoBoardUtil::FindDiagNeighbor(bd, p, SG_EMPTY);
    } else if (bd.NumNeighbors(p, color) == 3
        && bd.NumNeighbors(p, opp) == 0
        && bd.NumDiagonals(p, color) >= 3
        ) {
      isPossibleEye = true;
      move = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
    }
  }

  return isPossibleEye;
}

#endif

