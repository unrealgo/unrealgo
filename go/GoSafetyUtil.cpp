

#include "platform/SgSystem.h"
#include "GoSafetyUtil.h"

#include <math.h>
#include "GoBlock.h"
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoEyeUtil.h"
#include "GoModBoard.h"
#include "GoRegion.h"
#include "GoRegionBoard.h"
#include "GoSafetySolver.h"
#include "board/GoBoardColor.h"
#include "board/GoBWSet.h"
#include "lib/SgVector.h"
#include "board/GoPointSet.h"
#include "board/GoPointSetUtil.h"
#include "board/SgWrite.h"

namespace {
const bool DEBUG_SAFETY = false;
const bool DEBUG_MIGHT_MAKE_LIFE = false;
const bool DEBUG_EXTENDED_MIGHT_MAKE_LIFE = false;

void AddLibertiesAsMoves(const GoBoard& bd,
                         const GoBWSet& safe,
                         SgBWArray<int>& nuSafe) {
  for (SgBWIterator it; it; ++it) {
    const GoPointSet adj = safe[*it].BorderNoClip() & bd.AllEmpty();
    DBG_ASSERT(adj.Disjoint(safe.Both()));
    int nu = adj.Size();
    if (bd.ToPlay() == *it)
      ++nu;
    nuSafe[*it] += nu / 2;
  }
}

inline SgEmptyBlackWhite CheckWinner(
    const SgBWArray<int>& winThreshold,
    const SgBWArray<int>& nuSafe) {
  for (SgBWIterator it; it; ++it)
    if (nuSafe[*it] >= winThreshold[*it])
      return *it;
  return SG_EMPTY;
}

bool Find2Connections(const GoBoard& bd, GoPoint block, GoPointSet* libs,
                      GoPointSet* usedLibs, GoPointSet* safe) {
  DBG_ASSERT(libs->Disjoint(*usedLibs));

  int nuLibs(0);
  SgVector<GoPoint> blockLibs;
  for (GoBoard::LibertyIterator it(bd, block); it; ++it) {
    if (libs->Contains(*it)) {
      blockLibs.PushBack(*it);
      ++nuLibs;
      if (nuLibs >= 2)
        break;
    }
  }
  if (nuLibs >= 2) {
    for (GoBoard::StoneIterator it(bd, block); it; ++it)
      safe->Include(*it);
    for (GoBoard::LibertyIterator it(bd, block); it; ++it)
      libs->Include(*it);
    for (SgVectorIterator<GoPoint> it(blockLibs); it; ++it)
      usedLibs->Include(*it);
    *libs -= *usedLibs;
  }

  return nuLibs >= 2;
}

bool Find2ConnectionsForAll(const GoBoard& bd, const GoPointSet& pts,
                            const GoPointSet& inSafe, SgBlackWhite color,
                            int maxNuOmissions = 0) {
  if (DEBUG_SAFETY)
    SgDebug() << "Find2ConnectionsForAll " << pts
              << "safe points - input: " << inSafe;
  GoPointSet safe(inSafe);
  SgVector<GoPoint> unsafe;
  const int size = bd.Size();
  GoSafetyUtil::ReduceToAnchors(bd, pts.Border(size) - safe, &unsafe);

  if (DEBUG_SAFETY)
    SgDebug() << SgWritePointList(unsafe, "unsafe anchors: ");

  GoPointSet libs = pts & bd.AllEmpty() & safe.Border(size);
  GoPointSet interior = pts - libs;
  interior -= bd.All(SgOppBW(color));
  SgVector<GoPoint> unsafeInterior;
  GoSafetyUtil::ReduceToAnchors(bd, interior & bd.All(color),
                                &unsafeInterior);
  unsafe.Concat(&unsafeInterior);

  GoPointSet usedLibs;
  bool change = true;
  while (change && unsafe.NonEmpty() && libs.MinSetSize(2)) {
    SgVector<GoPoint> newSafe;
    for (SgVectorIterator<GoPoint> it(unsafe); it; ++it)
      if (Find2Connections(bd, *it, &libs, &usedLibs, &safe)) {
        newSafe.PushBack(*it);
      }

    unsafe.Exclude(newSafe);
    change = newSafe.NonEmpty();
  }

  if (unsafe.NonEmpty()) {
    if (DEBUG_SAFETY)
      SgDebug()
          << SgWritePointList(unsafe, "could not connect unsafe: ");
    return false;
  }

  interior = (pts & bd.AllEmpty()) - safe.Border(size);
  if (maxNuOmissions == 1) {
    SgBlackWhite opp(SgOppBW(color));
    if (!GoSafetyUtil::MightMakeLife(bd, interior, safe, opp))
      return true;
  }

  for (SgSetIterator it(interior); it; ++it) {
    if (!GoSafetyUtil::Find2Libs(*it, &libs)) {
      if (--maxNuOmissions < 0)
        return false;
    }
  }

  return true;
}

SgEmptyBlackWhite GetWinner(const GoBoard& bd,
                            const GoBWSet& safe,
                            float komi) {

  const float EPSILON = 0.1f;
  const int nuPoints = bd.Size() * bd.Size();
  int winThresholdBlack =
      int(ceil((float(nuPoints) + komi + EPSILON) / 2.f));
  int winThresholdWhite =
      int(ceil((float(nuPoints) - komi + EPSILON) / 2.f));
  const SgBWArray<int> winThreshold(winThresholdBlack, winThresholdWhite);
  SgBWArray<int> nuSafe;
  for (SgBWIterator it; it; ++it)
    nuSafe[*it] = safe[*it].Size();

  SgEmptyBlackWhite winner = CheckWinner(winThreshold, nuSafe);
  if (winner != SG_EMPTY)
    return winner;

  AddLibertiesAsMoves(bd, safe, nuSafe);
  winner = CheckWinner(winThreshold, nuSafe);
  if (winner != SG_EMPTY)
    return winner;
  if (nuSafe[SG_BLACK] + nuSafe[SG_WHITE] == nuPoints)
    SgDebug() << "draw: B = " << nuSafe[SG_BLACK]
              << ", W = " << nuSafe[SG_WHITE]
              << std::endl;

  return SG_EMPTY;
}

void TestLiberty(GoPoint lib, const GoPointSet& libs,
                 SgVector<GoPoint>* foundLibs,
                 int* nuLibs) {
  if (libs.Contains(lib)) {
    foundLibs->PushBack(lib);
    ++(*nuLibs);
  }
}

void WriteSafeTotal(std::ostream& stream, std::string text,
                    int partCount, int totalCount) {
  stream << partCount << " / " << totalCount
         << " ("
         << (partCount * 100 + totalCount / 2) / totalCount
         << "%) " << text << '\n';
}

}

void GoSafetyUtil::AddToSafe(const GoBoard& board, const GoPointSet& pts,
                             SgBlackWhite color, GoBWSet* safe,
                             const std::string& reason,
                             int depth, bool addBoundary) {
  SG_DEBUG_ONLY(reason);
  SG_DEBUG_ONLY(depth);

  (*safe)[color] |= pts;
  safe->AssertDisjoint();
  GoPointSet empty = board.AllEmpty();
  const int size = board.Size();
  if (DEBUG_SAFETY)
    SgDebug() << "AddToSafe " << reason
              << " depth = " << depth << " points = "
              << PointSetIDWriter(pts) << '\n';

  if (addBoundary) {
    GoPointSet dep(pts.Border(size) - empty);
    GoBoardUtil::ExpandToBlocks(board, dep);
    DBG_ASSERT(dep.SubsetOf(board.All(color)));
    if (DEBUG_SAFETY) {
      const GoPointSet newPts = dep - (*safe)[color];
      SgDebug() << "Also AddBoundary "
                << PointSetIDWriter(newPts) << '\n';
    }
    (*safe)[color] |= dep;
    safe->AssertDisjoint();
  }

}

bool GoSafetyUtil::ExtendedMightMakeLife(const GoBoard& board,
                                         GoRegionBoard* regions,
                                         const GoPointSet& area,
                                         const GoPointSet& safe,
                                         SgBlackWhite color) {
  const GoRegion* nakadeRegion = 0;

  if (DEBUG_EXTENDED_MIGHT_MAKE_LIFE)
    SgDebug() << "ExtendedMightMakeLife for " << SgBW(color)
              << " area " << area << '\n';
  for (SgVectorIteratorOf<GoRegion> it(regions->AllRegions(color));
       it; ++it) {
    if (area.SupersetOf((*it)->Points())
        && area.SupersetOf((*it)->BlocksPoints())
        ) {
      if (DEBUG_EXTENDED_MIGHT_MAKE_LIFE) {
        SgDebug() << "contains region ";
        (*it)->WriteID(SgDebug());
        SgDebug() << '\n';
      }

      if (!(*it)->ComputedFlag(GO_REGION_COMPUTED_NAKADE))
        (*it)->DoComputeFlag(GO_REGION_COMPUTED_NAKADE);
      if ((*it)->MaxPotEyes() > 1)
        return true;
      else if (nakadeRegion == 0)
        nakadeRegion = *it;
      else
        return true;
    }
  }

  if (DEBUG_EXTENDED_MIGHT_MAKE_LIFE)
    SgDebug() << "case 2\n";
  GoPointSet rest = area;
  if (nakadeRegion == 0)
    return GoSafetyUtil::MightMakeLife(board, area, safe, color);
  else {
    if (DEBUG_EXTENDED_MIGHT_MAKE_LIFE)
      SgDebug() << "ExtendedMightMakeLife for " << area
                << ": inside opp region "
                << *nakadeRegion << '\n';
    if (nakadeRegion->MaxPotEyes() <= 1) {
      rest -= nakadeRegion->Points();
      rest -= nakadeRegion->BlocksPoints();
    }
  }

  const int size = board.Size();
  rest -= safe.Border(size);
  rest -= board.All(color);

  if (DEBUG_EXTENDED_MIGHT_MAKE_LIFE)
    SgDebug() << "rest = " << rest << "\n";
  for (SgSetIterator it(rest); it; ++it) {
    GoPoint p(*it);
    if (GoEyeUtil::CanBecomeSinglePointEye(board, p, safe))
      return true;
  }

  return false;
}

GoPointSet GoSafetyUtil::FindDamePoints(const GoBoard& bd,
                                        const GoPointSet& empty,
                                        const GoBWSet& safe) {
  GoPointSet dame, unsurroundable;
  FindDameAndUnsurroundablePoints(bd, empty, safe, &dame, &unsurroundable);
  return dame;
}

void GoSafetyUtil::FindDameAndUnsurroundablePoints(const GoBoard& bd,
                                                   const GoPointSet& empty,
                                                   const GoBWSet& safe,
                                                   GoPointSet* dame,
                                                   GoPointSet* unsurroundable) {
  DBG_ASSERT(dame->IsEmpty());
  DBG_ASSERT(unsurroundable->IsEmpty());
  const int size = bd.Size();
  *unsurroundable = safe[SG_BLACK].Border(size)
      & safe[SG_WHITE].Border(size)
      & empty;
  GoPointSet maybeDame(*unsurroundable);
  GoBWSet unsafe;
  unsafe[SG_BLACK] = bd.All(SG_BLACK) - safe[SG_BLACK];
  unsafe[SG_WHITE] = bd.All(SG_WHITE) - safe[SG_WHITE];
  maybeDame -= unsafe[SG_BLACK].Border(size);
  maybeDame -= unsafe[SG_WHITE].Border(size);
  for (SgSetIterator it(maybeDame); it; ++it) {
    GoPoint p(*it);
    bool isDame = true;
    for (GoNbIterator nit(bd, p); nit; ++nit) {
      GoPoint nb(*nit);
      if (empty[nb] && !unsurroundable->Contains(nb)) {
        isDame = false;
        break;
      }
    }
    if (isDame)
      dame->Include(p);
  }
}

bool GoSafetyUtil::MightMakeLife(const GoBoard& board,
                                 const GoPointSet& area,
                                 const GoPointSet& safe, SgBlackWhite color) {
  const int size = board.Size();
  GoPointSet eyePts = (area - safe.Border(size)) - board.All(color);
  if (eyePts.MaxSetSize(1))
    return false;

  if (DEBUG_MIGHT_MAKE_LIFE)
    SgDebug() << "GoSafetyUtil::MightMakeLife\n";

  GoPoint eye(GO_NULLPOINT), adjToEye(GO_NULLPOINT);
  for (SgSetIterator it(eyePts); it; ++it) {
    const GoPoint p(*it);
    if (GoEyeUtil::CanBecomeSinglePointEye(board, p, safe)) {
      if (eye == GO_NULLPOINT) {
        eye = p;
        if (DEBUG_MIGHT_MAKE_LIFE)
          SgDebug() << "eye = " << GoWritePoint(eye) << "\n";
      } else if (adjToEye == GO_NULLPOINT
          && GoPointUtil::AreAdjacent(eye, p)
          )
        adjToEye = p;
      else {
        if (DEBUG_MIGHT_MAKE_LIFE)
          SgDebug() << "second eye = " << GoWritePoint(p) << "\n";
        return true;
      }
    }
  }

  return false;
}

bool GoSafetyUtil::Find2Libs(GoPoint p, GoPointSet* libs) {
  int nuLibs = 0;
  SgVector<GoPoint> foundLibs;
  TestLiberty(p + GO_NORTH_SOUTH, *libs, &foundLibs, &nuLibs);
  TestLiberty(p + GO_WEST_EAST, *libs, &foundLibs, &nuLibs);
  if (nuLibs < 2) {
    TestLiberty(p - GO_NORTH_SOUTH, *libs, &foundLibs, &nuLibs);
    if (nuLibs < 2)
      TestLiberty(p - GO_WEST_EAST, *libs, &foundLibs, &nuLibs);
  }
  if (nuLibs >= 2) {
    DBG_ASSERT(nuLibs == 2 && foundLibs.IsLength(2));
    libs->Exclude(foundLibs.Front());
    libs->Exclude(foundLibs.Back());
  }

  return nuLibs >= 2;
}

bool GoSafetyUtil::Find2BestLibs(GoPoint p, const GoPointSet& libs,
                                 GoPointSet interior, SgMiaiPair* miaiPair) {
  int nuLibs = 0;
  SgVector<GoPoint> allLibs;

  TestLiberty(p + GO_NORTH_SOUTH, libs, &allLibs, &nuLibs);
  TestLiberty(p + GO_WEST_EAST, libs, &allLibs, &nuLibs);
  TestLiberty(p - GO_NORTH_SOUTH, libs, &allLibs, &nuLibs);
  TestLiberty(p - GO_WEST_EAST, libs, &allLibs, &nuLibs);

  if (allLibs.MaxLength(1))
    return false;
  else if (allLibs.IsLength(2)) {
    DBG_ASSERT(nuLibs == 2 && allLibs.IsLength(2));
    miaiPair->first = allLibs[0];
    miaiPair->second = allLibs[1];
    return true;
  } else {
    SgVector<GoPoint> shared, not_shared;
    GoPointSet others = interior;
    others.Exclude(p);

    for (SgVectorIterator<GoPoint> it(allLibs); it; ++it) {
      bool share = false;
      for (SgSetIterator it2(others); it2; ++it2) {
        if (GoPointUtil::AreAdjacent(*it, *it2)) {
          share = true;
          break;
        }
      }
      if (share)
        shared.PushBack(*it);
      else
        not_shared.PushBack(*it);
    }
    if (not_shared.MinLength(2)) {
      miaiPair->first = not_shared[0];
      miaiPair->second = not_shared[1];
    } else if (not_shared.IsLength(1)) {
      miaiPair->first = not_shared[0];
      miaiPair->second = shared[0];
    } else {
      miaiPair->first = shared[0];
      miaiPair->second = shared[1];
    }
    return true;
  }
}

bool GoSafetyUtil::ExtendedIsTerritory(const GoBoard& board,
                                       GoRegionBoard* regions,
                                       const GoPointSet& pts,
                                       const GoPointSet& safe,
                                       SgBlackWhite color,
                                       std::string& reason) {
  DBG_ASSERT(!pts.Overlaps(safe));
  const int size = board.Size();
  GoPointSet boundary(pts.Border(size));
  if (boundary.SubsetOf(safe)) {
    SgBlackWhite opp = SgOppBW(color);
    if (!ExtendedMightMakeLife(board, regions, pts, safe, opp)) {
      reason = "Boundary-safe-opp-cannot-live";
      return true;
    }
  }

  return IsTerritory(board, pts, safe, color, &reason);
}

SgEmptyBlackWhite GoSafetyUtil::GetWinner(const GoBoard& bd) {
  GoRegionBoard regionAttachment(bd);
  GoSafetySolver solver(bd, &regionAttachment);
  GoBWSet safe;
  solver.FindSafePoints(&safe);
  const float komi = bd.Rules().Komi().ToFloat();
  return ::GetWinner(bd, safe, komi);
}

bool GoSafetyUtil::IsTerritory(const GoBoard& board, const GoPointSet& pts,
                               const GoPointSet& safe, SgBlackWhite color,
                               std::string* reason) {
  DBG_ASSERT(!pts.Overlaps(safe));
  const int size = board.Size();
  GoPointSet boundary(pts.Border(size));
  if (boundary.SubsetOf(safe)) {
    SgBlackWhite opp = SgOppBW(color);
    if (!GoSafetyUtil::MightMakeLife(board, pts, safe, opp)) {
      if (reason)
        *reason = "IsTerritory-opp-cannot-live";
      return true;
    }
  }

  if (boundary.SubsetOf(board.All(color))
      && Find2ConnectionsForAll(board, pts, safe, color, 1)
      ) {
    if (reason)
      *reason = "IsTerritory-Find2ConnectionsForAll";
    return true;
  }
  return false;
}

void GoSafetyUtil::ReduceToAnchors(const GoBoard& board,
                                   const GoPointSet& stones,
                                   SgVector<GoPoint>* anchors) {
  DBG_ASSERT(anchors->IsEmpty());
  for (SgSetIterator it(stones); it; ++it) {
    DBG_ASSERT(board.Occupied(*it));
    anchors->Insert(board.Anchor(*it));
  }
}

void GoSafetyUtil::WriteStatistics(const std::string& heading,
                                   const GoRegionBoard* regions,
                                   const GoBWSet* safe) {
  const GoPointSet allSafe = safe->Both();
  int totalRegions = 0;
  int safeRegions = 0;
  int totalBlocks = 0;
  int safeBlocks = 0;

  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion> it(regions->AllRegions(color));
         it; ++it) {
      ++totalRegions;
      if ((*it)->Points().SubsetOf(allSafe))
        ++safeRegions;
    }
    for (SgVectorIteratorOf<GoBlock> it(regions->AllBlocks(color));
         it; ++it) {
      ++totalBlocks;
      if (allSafe.Overlaps((*it)->Stones()))
        ++safeBlocks;
    }
  }

  const int bdSize = regions->Board().Size();
  SgDebug() << heading << "\n";
  WriteSafeTotal(SgDebug(), "points", allSafe.Size(), bdSize * bdSize);
  WriteSafeTotal(SgDebug(), "regions", safeRegions, totalRegions);
  WriteSafeTotal(SgDebug(), "blocks", safeBlocks, totalBlocks);
}
