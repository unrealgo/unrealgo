

#include "platform/SgSystem.h"
#include "GoRegionUtil.h"

#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoEyeUtil.h"
#include "lib/SgVector.h"
#include "board/GoPointSet.h"

namespace {

bool Has2IntersectionPoints(const GoBoard& board, const GoPointSet& region,
                            const GoPoint boundaryAnchor) {
  int nuIPs = 0;
  for (GoBoard::LibertyIterator it(board, boundaryAnchor); it; ++it) {
    if (region.Contains(*it)
        && GoEyeUtil::IsSplitPt(*it, region)
        && ++nuIPs >= 2
        )
      return true;
  }
  return false;
}

bool Has2IntersectionPoints(const GoBoard& board, const GoPointSet& region,
                            const SgVector<GoPoint>& boundaryAnchors) {
  if (boundaryAnchors.IsLength(1))
    return Has2IntersectionPoints(board, region, boundaryAnchors.Back());
  else {
    SgVector<GoPoint> sharedLibs;
    for (GoBoard::LibertyIterator it(board, boundaryAnchors.Front()); it;
         ++it)
      if (region.Contains(*it))
        sharedLibs.PushBack(*it);

    if (sharedLibs.MaxLength(1))
      return false;
    bool first = true;
    for (SgVectorIterator<GoPoint> bit(boundaryAnchors); bit; ++bit) {
      if (first)
        first = false;
      else {
        SgVector<GoPoint> newShared;
        const GoPoint block = *bit;
        for (GoBoard::LibertyIterator it(board, block); it; ++it)
          if (sharedLibs.Contains(*it))
            newShared.PushBack(*it);
        if (newShared.MaxLength(1))
          return false;
        else
          newShared.SwapWith(&sharedLibs);
      }
    }
    int nuIPs = 0;
    for (SgVectorIterator<GoPoint> it(sharedLibs); it; ++it) {
      if (GoEyeUtil::IsSplitPt(*it, region)
          && ++nuIPs >= 2
          )
        return true;
    }
    return false;
  }
}

inline bool IsAdjacentToAll(const GoBoard& board, GoPoint p,
                            const SgVector<GoPoint>& anchors) {
  for (SgVectorIterator<GoPoint> it(anchors); it; ++it)
    if (!board.IsLibertyOfBlock(p, *it))
      return false;
  return true;
}

bool TwoSeparateEyes(const GoBoard& bd, const GoPointSet& pts,
                     const GoPointSet& boundary, SgBlackWhite color) {
  DBG_ASSERT((pts & bd.AllEmpty()).SubsetOf(boundary.Border(bd.Size())));

  if (pts.Disjoint(bd.All(color)) && !pts.IsConnected()) {
    if (GoRegionUtil::IsSingleBlock(bd, boundary, color))
      return true;
    else {
      const GoPointSet area = pts & bd.AllEmpty();
      if (area.MinSetSize(2)) {
        for (SgSetIterator it(area); it; ++it)
          if (bd.IsLegal(*it, SgOppBW(color)))
            return false;
        return true;
      }
    }
  }
  return false;
}

}

void GoRegionUtil::FindCurrentAnchors(const GoBoard& board,
                                      const SgVector<GoPoint>& origAnchors,
                                      SgVector<GoPoint>* currentAnchors) {
  DBG_ASSERT(currentAnchors->IsEmpty());
  for (SgVectorIterator<GoPoint> it(origAnchors); it; ++it)
    currentAnchors->Insert(board.Anchor(*it));
}

bool GoRegionUtil::Has2IPorEyes(const GoBoard& board, const GoPointSet& pts,
                                SgBlackWhite color,
                                const SgVector<GoPoint>& boundaryAnchors) {
  return Has2IntersectionPoints(board, pts, boundaryAnchors)
      || (boundaryAnchors.IsLength(1)
          && pts.Disjoint(board.All(color))
          && !pts.IsConnected()
      );
}

bool GoRegionUtil::Has2SureLiberties(const GoBoard& board,
                                     const GoPointSet& pts,
                                     SgBlackWhite color,
                                     const SgVector<GoPoint>& boundaryAnchors) {
  const int size = board.Size();
  GoPointSet boundary(pts.Border(size));
  if (!boundary.SubsetOf(board.All(color))) {
    return false;
  } else {
    boundary -= board.AllEmpty();
    DBG_ASSERT(boundary.SubsetOf(board.All(color)));
  }

  if ((pts & board.AllEmpty()).SubsetOf(boundary.Border(size))
      && (Has2IntersectionPoints(board, pts, boundaryAnchors)
          || TwoSeparateEyes(board, pts, boundary, color)
      ))
    return true;

  return false;
}

bool GoRegionUtil::IsSingleBlock(const GoBoard& board, const GoPointSet& pts,
                                 SgBlackWhite color) {
  SG_DEBUG_ONLY(color);
  DBG_ASSERT(pts.NonEmpty()
                 || board.TotalNumEmpty() == board.Size() * board.Size());

  GoPoint firstAnchor(GO_NULLPOINT);
  for (SgSetIterator it(pts); it; ++it) {
    DBG_ASSERT(board.IsColor(*it, color));
    const GoPoint anchor = board.Anchor(*it);
    if (anchor != firstAnchor) {
      if (firstAnchor == GO_NULLPOINT)
        firstAnchor = anchor;
      else
        return false;
    }
  }
  return true;
}

bool GoRegionUtil::IsSmallRegion(const GoBoard& board, const GoPointSet& pts,
                                 SgBlackWhite opp) {
  const int size = board.Size();
  return pts.Kernel(size).SubsetOf(board.All(opp));
}

bool GoRegionUtil::StaticIs1VitalAndConnected(const GoBoard& board,
                                              const GoPointSet& pts,
                                              SgBlackWhite color) {
  SgVector<GoPoint> anchors;
  GoBoardUtil::BlocksAdjacentToPoints(board, pts, color, &anchors);

  bool is1Vital = false;

  if (IsSmallRegion(board, pts, SgOppBW(color))) {
    if (anchors.MaxLength(1))
      return true;
    else if (anchors.MinLength(5))
      return false;

    int nuConn = 0;
    for (SgSetIterator it(pts); it; ++it) {
      const GoPoint p(*it);
      if (board.IsEmpty(p) &&
          IsAdjacentToAll(board, p, anchors)) {
        if (++nuConn >= 2) {
          is1Vital = true;
          break;
        }
      }
    }
  }
  return is1Vital;
}