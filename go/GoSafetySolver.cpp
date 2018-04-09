

#include "platform/SgSystem.h"
#include "GoSafetySolver.h"

#include "GoBlock.h"
#include "GoChain.h"
#include "GoSafetyUtil.h"
#include "board/GoConnCompIterator.h"

namespace {

const bool DEBUG_MERGE_CHAINS = false;
const bool DEBUG_SAFETY_SOLVER = false;

bool HaveSharedUnsafe(const SgVectorOf<GoBlock>& list1,
                      const SgVectorOf<GoBlock>& list2) {
  for (SgVectorIteratorOf<GoBlock> it(list1); it; ++it)
    if (!(*it)->IsSafe() && list2.Contains(*it))
      return true;
  return false;
}

}

void GoSafetySolver::FindHealthy() {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion>
             it(Regions()->AllRegions(color)); it; ++it)
      (*it)->ComputeFlag(GO_REGION_STATIC_1VITAL);
  }

  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion>
             it(Regions()->AllRegions(color)); it; ++it) {
      GoRegion* r = *it;
      for (SgVectorIteratorOf<GoChain> it2(r->Chains()); it2; ++it2) {
        if (RegionHealthyForBlock(*r, **it2))
          (*it2)->AddHealthy(r);
      }
    }
  }
}

void GoSafetySolver::FindClosure(SgVectorOf<GoBlock>* blocks) const {
  SgVectorOf<GoBlock> toTest(*blocks);
  while (toTest.NonEmpty()) {
    const GoBlock* b = toTest.Back();
    toTest.PopBack();
    for (SgVectorIteratorOf<GoRegion> rit(b->Healthy()); rit; ++rit) {
      GoRegion* r = *rit;
      for (SgVectorIteratorOf<GoChain> it(r->Chains()); it; ++it) {
        GoBlock* b2 = *it;
        if (!blocks->Contains(b2)) {
          blocks->PushBack(b2);
          toTest.PushBack(b2);
        }
      }
    }
  }
}

void GoSafetySolver::FindTestSets(SgVectorOf<SgVectorOf<GoBlock> >* sets,
                                  SgBlackWhite color) const {
  DBG_ASSERT(sets->IsEmpty());
  SgVectorOf<GoBlock> doneSoFar;
  for (SgVectorIteratorOf<GoChain>
           it(Regions()->AllChains(color)); it; ++it) {
    GoBlock* block = *it;
    if (!doneSoFar.Contains(block)) {
      SgVectorOf<GoBlock>* blocks = new SgVectorOf<GoBlock>;
      blocks->PushBack(block);

      FindClosure(blocks);
      doneSoFar.PushBackList(*blocks);
      sets->PushBack(blocks);
    }
  }
}

bool GoSafetySolver::RegionHealthyForBlock(const GoRegion& r,
                                           const GoBlock& b) const {
  return GoStaticSafetySolver::RegionHealthyForBlock(r, b)
      || r.GetFlag(GO_REGION_STATIC_1VITAL);
}

void GoSafetySolver::Test2Vital(GoRegion* r, GoBWSet* safe) {
  if (r->ComputeAndGetFlag(GO_REGION_STATIC_2V))
    GoSafetyUtil::AddToSafe(Board(), r->Points(), r->Color(),
                            safe, "2-vital:", 0, true);
}

void GoSafetySolver::Find2VitalAreas(GoBWSet* safe) {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion>
             it(Regions()->AllRegions(color)); it; ++it)
      if ((*it)->Points().Disjoint((*safe)[SG_BLACK])
          && (*it)->Points().Disjoint((*safe)[SG_WHITE])
          ) {
        Test2Vital(*it, safe);
        safe->AssertDisjoint();
      }
  }
}

void GoSafetySolver::FindSurroundedSafeAreas(GoBWSet* safe,
                                             SgBlackWhite color) {
  Regions()->SetSafeFlags(*safe);
  while (FindSurroundedSingleRegion(safe, color)) {}
  while (FindSurroundedRegionPair(safe, color))
    while (FindSurroundedSingleRegion(safe, color)) {}
}

bool GoSafetySolver::FindSafePair(GoBWSet* safe,
                                  SgBlackWhite color,
                                  const GoPointSet& anySafe,
                                  const GoRegion* r1) {
  for (SgVectorIteratorOf<GoRegion>
           it(Regions()->AllRegions(color)); it; ++it) {
    const GoRegion* r2 = *it;
    if (r2 != r1
        && !r2->Points().Overlaps(anySafe)
        && HaveSharedUnsafe(r1->Blocks(), r2->Blocks())
        ) {
      const GoPointSet unionSet(r1->Points() | r2->Points());
      std::string reason;
      if (GoSafetyUtil::IsTerritory(Board(), unionSet,
                                    (*safe)[color], color, &reason)) {
        std::string fullReason = "surr-safe-2-" + reason;
        GoSafetyUtil::AddToSafe(Board(), unionSet, color, safe,
                                fullReason, 0, true);
        Regions()->SetSafeFlags(*safe);
        safe->AssertDisjoint();
        return true;
      }
    }
  }
  return false;
}

bool GoSafetySolver::FindSurroundedRegionPair(GoBWSet* safe,
                                              SgBlackWhite color) {
  GoPointSet anySafe(safe->Both());
  for (SgVectorIteratorOf<GoRegion>
           it(Regions()->AllRegions(color)); it; ++it) {
    GoRegion* r1 = *it;
    if (!r1->GetFlag(GO_REGION_SAFE)
        && r1->SomeBlockIsSafe()
        && !r1->Points().Overlaps(anySafe)
        && FindSafePair(safe, color, anySafe, r1)
        )
      return true;
  }
  return false;
}

bool GoSafetySolver::FindSurroundedSingleRegion(GoBWSet* safe,
                                                SgBlackWhite color) {
  GoPointSet anySafe(safe->Both());
  for (SgVectorIteratorOf<GoRegion>
           it(Regions()->AllRegions(color)); it; ++it) {
    GoRegion* r = *it;
    std::string reason;
    if (!r->GetFlag(GO_REGION_SAFE)
        && r->SomeBlockIsSafe()
        && !r->PointsPlusInteriorBlocks().Overlaps(anySafe)
        && GoSafetyUtil::ExtendedIsTerritory(Board(), Regions(),
                                             r->PointsPlusInteriorBlocks(),
                                             (*safe)[color],
                                             color,
                                             reason)
        ) {
      std::string fullReason = "surr-safe-1-" + reason;
      GoSafetyUtil::AddToSafe(Board(), r->Points(), color, safe,
                              fullReason, 0, true);
      Regions()->SetSafeFlags(*safe);
      return true;
    }
  }
  return false;
}

void GoSafetySolver::FindSafePoints(GoBWSet* safe) {
  GoStaticSafetySolver::FindSafePoints(safe);
  safe->AssertDisjoint();
  if (DEBUG_SAFETY_SOLVER)
    GoSafetyUtil::WriteStatistics("Base Solver", Regions(), safe);
  Find2VitalAreas(safe);
  safe->AssertDisjoint();
  if (DEBUG_SAFETY_SOLVER)
    GoSafetyUtil::WriteStatistics("2Vital", Regions(), safe);
  for (SgBWIterator it; it; ++it) {
    FindSurroundedSafeAreas(safe, *it);
    safe->AssertDisjoint();
  }

  if (DEBUG_SAFETY_SOLVER)
    GoSafetyUtil::WriteStatistics("SurroundedSafe-Final",
                                  Regions(), safe);
}

void GoSafetySolver::Merge(GoChain* c1, GoChain* c2,
                           GoRegion* rg, bool bySearch) {
  DBG_ASSERT(!rg->GetFlag(GO_REGION_USED_FOR_MERGE));
  rg->SetFlag(GO_REGION_USED_FOR_MERGE, true);

  GoChainCondition* c = 0;
  if (bySearch)
    c = new GoChainCondition(GO_CHAIN_BY_SEARCH);
  else {
    GoPoint lib1, lib2;
    rg->Find2FreeLibs(c1, c2, &lib1, &lib2);
    c = new GoChainCondition(GO_CHAIN_TWO_LIBERTIES_IN_REGION,
                             lib1, lib2);
  }

  GoChain* m = new GoChain(c1, c2, c);
  SgBlackWhite color = c1->Color();
  bool found = Regions()->AllChains(color).Exclude(c1);
  SG_DEBUG_ONLY(found);
  DBG_ASSERT(found);
  found = Regions()->AllChains(color).Exclude(c2);
  DBG_ASSERT(found);
  Regions()->AllChains(color).Include(m);
  DBG_ASSERT(Regions()->AllChains(color).UniqueElements());

  for (SgVectorIteratorOf<GoRegion>
           it(Regions()->AllRegions(color)); it; ++it) {
    GoRegion* r = *it;
    bool replace1 = r->ReplaceChain(c1, m);
    bool replace2 = r->ReplaceChain(c2, m);
    if (replace1 || replace2) {
      r->ReInitialize();
      r->ComputeFlag(GO_REGION_STATIC_1VITAL);
    }
  }

  if (DEBUG_MERGE_CHAINS) {
    SgDebug() << "\nmerge:";
    c1->WriteID(SgDebug());
    SgDebug() << " + ";
    c2->WriteID(SgDebug());
    SgDebug() << " = ";
    m->WriteID(SgDebug());
    SgDebug() << '\n';
  }

  delete c1;
  delete c2;
}

void GoSafetySolver::GenBlocksRegions() {
  if (UpToDate())
    return;

  GoStaticSafetySolver::GenBlocksRegions();

  Regions()->GenChains();
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion>
             it(Regions()->AllRegions(color)); it; ++it) {
      GoRegion* r = *it;
      r->ComputeFlag(GO_REGION_STATIC_1VITAL);
    }

    bool changed = true;
    while (changed) {
      changed = false;
      for (SgVectorIteratorOf<GoRegion>
               it(Regions()->AllRegions(color)); it; ++it) {
        GoRegion* r = *it;
        if (r->GetFlag(GO_REGION_STATIC_1VC)
            && r->Chains().IsLength(2)
            && r->Has2Conn()
            ) {
          GoChain* c1 = r->Chains().Front();
          GoChain* c2 = r->Chains().Back();
          Merge(c1, c2, r, false);
          changed = true;
          break;
        } else if (r->GetFlag(GO_REGION_STATIC_1VITAL)
            && r->GetFlag(GO_REGION_CORRIDOR)
            && !r->GetFlag(GO_REGION_USED_FOR_MERGE)
            ) {
          GoChain* c1 = 0;
          GoChain* c2 = 0;
          if (r->Find2Mergable(&c1, &c2)) {
            Merge(c1, c2, r, false);
            changed = true;
            break;
          }
        }
      }
    }
  }

  m_code = Board().GetHashCode();
}

bool GoSafetySolver::PotentialCaptureMove(GoPoint p,
                                          SgBlackWhite regionColor) const {
  DBG_ASSERT(UpToDate());
  DBG_ASSERT(Board().ToPlay() == regionColor);
  const GoRegion* r = Regions()->RegionAt(p, regionColor);
  DBG_ASSERT(r);
  return r->Points().Overlaps(Board().All(SgOppBW(regionColor)));
}

