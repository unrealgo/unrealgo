

#include "platform/SgSystem.h"
#include "GoRegion.h"

#include <iostream>
#include "GoBlock.h"
#include "GoBoard.h"
#include "GoChain.h"
#include "GoEyeUtil.h"
#include "GoRegionBoard.h"
#include "GoRegionUtil.h"
#include "GoSafetyUtil.h"
#include "board/GoConnCompIterator.h"
#include "platform/SgDebug.h"
#include "lib/SgVector.h"
#include "board/GoNbIterator.h"
#include "board/GoPointArray.h"
#include "SgStrategy.h"
#include "board/SgWrite.h"

using GoBoardUtil::ExpandToBlocks;
using GoEyeUtil::IsSplitPt;
using GoEyeUtil::TestNakade;
using GoRegionUtil::IsSmallRegion;
using GoSafetyUtil::Find2BestLibs;
using GoSafetyUtil::Find2Libs;
using GoSafetyUtil::MightMakeLife;

static const bool CHECK = SG_CHECK && true;
static const bool HEAVYCHECK = SG_HEAVYCHECK && CHECK && false;
static const bool WRITEDEBUG = false;
namespace {

inline bool IsAdjacentToAll(const GoBoard& board, GoPoint p,
                            const SgVectorOf<GoBlock>& blocks) {
  for (SgVectorIteratorOf<GoBlock> it(blocks); it; ++it)
    if (!board.IsLibertyOfBlock(p, (*it)->Anchor()))
      return false;
  return true;
}

inline bool AdjacentToAll(GoPoint p, const SgVector<GoPoint>& points) {
  if (points.IsEmpty())
    return true;

  for (SgVectorIterator<GoPoint> it(points); it; ++it)
    if (!GoPointUtil::AreAdjacent(p, *it))
      return false;

  return true;
}

}

GoRegion::GoRegion(const GoBoard& board, const GoPointSet& points,
                   SgBlackWhite color)
    : m_bd(board),
      m_points(points),
      m_color(color),
      m_eyes(),
      m_vitalPoint(GO_NULLMOVE),
      m_1vcDepth(0),
      m_miaiStrategy(color) {
#ifndef NDEBUG
  ++s_allocated;
#endif
}

bool GoRegion::StaticIs1VitalAndConnected() const {
  bool is1Vital = false;
  if (GetFlag(GO_REGION_SMALL)) {
    if (GetFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY))
      return true;
    else if (m_blocks.MinLength(5))
      return false;

    int nuConn = 0;
    for (SgSetIterator it(Points()); it; ++it) {
      GoPoint p(*it);
      if (m_bd.IsEmpty(p) &&
          IsAdjacentToAll(m_bd, p, m_blocks)) {
        if (++nuConn >= 2) {
          is1Vital = true;
          break;
        }
      }
    }
  }
  return is1Vital;
}

bool GoRegion::AllEmptyAreLibs() const {
  for (SgSetIterator it(Points()); it; ++it) {
    GoPoint p(*it);
    if (m_bd.IsEmpty(p) && !m_bd.HasNeighbors(p, Color()))
      return false;
  }
  return true;
}

SgVectorOf<GoBlock> GoRegion::InteriorBlocks() const {
  SgVectorOf<GoBlock> interior;
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it)
    if (IsInteriorBlock(*it))
      interior.PushBack(*it);
  return interior;
}

bool GoRegion::IsInteriorBlock(const GoBlock* block) const {
  DBG_ASSERT(m_blocks.Contains(block));
  const SgBlackWhite opp = SgOppBW(block->Color());
  for (GoBoard::StoneIterator it(m_bd, block->Anchor()); it; ++it)
    for (GoNbIterator nb(m_bd, *it); nb; ++nb) {
      const GoPoint p = *nb;
      if ((m_bd.IsEmpty(p) || m_bd.IsColor(p, opp))
          && !m_points.Contains(p))
        return false;
    }
  return true;
}

GoPointSet GoRegion::PointsPlusInteriorBlocks() const {
  GoPointSet area = m_points;
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it)
    if (IsInteriorBlock(*it))
      area |= (*it)->Stones();
  return area;
}

void GoRegion::InteriorEmpty(SgVector<GoPoint>* interiorEmpty,
                             int maxNu) const {
  for (SgSetIterator it(Points()); it; ++it) {
    GoPoint p(*it);
    if (m_bd.IsEmpty(p) && !m_bd.HasNeighbors(p, Color())) {
      interiorEmpty->Include(p);
      if (--maxNu < 0)
        return;
    }
  }
}

bool GoRegion::Has2SureLibs(SgMiaiStrategy* miaiStrategy) const {
  if (m_blocks.IsEmpty())
    return false;

  DBG_ASSERT(!m_blocks.IsEmpty());
  SgVector<GoPoint> interiorEmpty;
  InteriorEmpty(&interiorEmpty, 3);
  SgMiaiPair ips;
  bool result1 = interiorEmpty.MaxLength(2)
      && Has2IPs(interiorEmpty, &ips);

  if (result1) {
    miaiStrategy->AddPair(ips);
    return true;
  }

  SgVector<GoPoint> usedLibs;
  bool result2 = Find2ConnForAllInterior(miaiStrategy, usedLibs)
      && Has2IntersectionPoints(usedLibs);
  return result2;
}

void GoRegion::InsideLibs(const GoBlock* b, SgVector<GoPoint>* libs) const {
  for (GoBoard::LibertyIterator it(m_bd, b->Anchor()); it; ++it)
    if (Points().Contains(*it))
      libs->PushBack(*it);
}

bool GoRegion::HasLibForAllBlocks() const {
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it)
    if (!HasBlockLibs(*it))
      return false;
  return true;
}

bool GoRegion::HasLibsForAllBlocks(int n) const {
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it)
    if (!(*it)->IsSafe() && !HasLibsForBlock(*it, n))
      return false;
  return true;
}

bool GoRegion::HasBlockLibs(const GoBlock* b) const {
  for (GoBoard::LibertyIterator it(m_bd, b->Anchor()); it; ++it)
    if (Points().Contains(*it))
      return true;
  return false;
}

bool GoRegion::HasLibsForBlock(const GoBlock* b, int n) const {
  int counter = 0;
  for (GoBoard::LibertyIterator it(m_bd, b->Anchor()); it; ++it)
    if (Points().Contains(*it)) {
      if (++counter >= n)
        return true;
    }
  return false;
}

void GoRegion::JointLibs(SgVector<GoPoint>* libs) const {
  GoBlock* first = m_blocks.Front();
  if (GetFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY)) {
    InsideLibs(first, libs);
    return;
  }

  DBG_ASSERT(m_blocks.MinLength(2));
  int minLib = INT_MAX;
  GoBlock* minB = 0;
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    int nu = (*it)->NuLiberties();
    if (nu < 2)
      return;
    else if (nu < minLib) {
      minLib = nu;
      minB = *it;
    }
  }
  DBG_ASSERT(minB != 0);

  for (GoBoard::LibertyIterator it(m_bd, minB->Anchor()); it; ++it) {
    GoPoint lib(*it);
    if (Points().Contains(lib)) {
      bool joint = true;
      for (SgVectorIteratorOf<GoBlock> itBlock(m_blocks); itBlock;
           ++itBlock) {
        if (!(*itBlock)->HasLiberty(lib)) {
          joint = false;
          break;
        }
      }
      if (joint)
        libs->PushBack(*it);
    }
  }
}

bool GoRegion::Has2IPs(const SgVector<GoPoint>& interiorEmpty,
                       SgMiaiPair* ips) const {
  SgVector<GoPoint> jointLibs;
  JointLibs(&jointLibs);
  if (jointLibs.MinLength(2)) {
    int nuIPs = 0;
    GoPoint ip1 = GO_NULLPOINT;
    for (SgVectorIterator<GoPoint> it(jointLibs); it; ++it) {
      if (AdjacentToAll(*it, interiorEmpty) && IsSplitPt(*it, Points())) {
        ++nuIPs;
        if (ip1 == GO_NULLPOINT)
          ip1 = *it;
        else {
          ips->first = ip1;
          ips->second = *it;
          return true;
        }
      }
    }
  }
  return false;
}

bool GoRegion::Has2IntersectionPoints(const SgVector<GoPoint>& usedLibs) const {
  SgVector<GoPoint> jointLibs;
  JointLibs(&jointLibs);
  if (jointLibs.MinLength(2)) {
    int nuIPs = 0;
    for (SgVectorIterator<GoPoint> it(jointLibs); it; ++it) {
      if (IsSplitPt(*it, Points()) && !usedLibs.Contains(*it)) {
        if (++nuIPs >= 2)
          return true;
      }
    }
  }
  return false;
}

void GoRegion::GetIPs(SgVector<GoPoint>* ips) const {
  SgVector<GoPoint> jointLibs;
  JointLibs(&jointLibs);
  for (SgVectorIterator<GoPoint> it(jointLibs); it; ++it)
    if (IsSplitPt(*it, Points()))
      ips->PushBack(*it);
}

void GoRegion::GetDivideMiaiPairs(SgVector<SgMiaiPair>& pairs) const {
  SgVector<GoPoint> divPs;

  for (SgVectorIteratorOf<GoBlock> it(Blocks()); it; ++it) {
    SgVector<GoPoint> libs, temp;
    InsideLibs(*it, &libs);
    SgMiaiPair p1;
    GoPoint a = -1;
    for (SgVectorIterator<GoPoint> it2(libs); it2; ++it2) {
      if (IsSplitPt(*it2, Points()))
        temp.PushBack(*it2);
    }
    temp.Sort();
    divPs.PushBackList(temp);

    for (SgVectorIterator<GoPoint> it2(temp); it2; ++it2) {
      if (a == -1)
        a = (*it2);
      else {
        if (GoPointUtil::AreAdjacent(a, *it2)) {
          p1.first = a;
          p1.second = *it2;
          pairs.PushBack(p1);
        }
        a = *it2;
      }
    }

  }

  if (WRITEDEBUG) {
    SgDebug() << SgWritePointList(divPs, "divPs: ", true);
    for (SgVectorIterator<SgMiaiPair> it(pairs); it; ++it) {
      SgDebug() << "Pair(1: " << GoWritePoint((*it).first)
                << " 2: " << GoWritePoint((*it).second) << ")\n";
    }
  }
}

GoPointSet GoRegion::AllInsideLibs() const {
  const int size = m_bd.Size();
  return (m_points - Dep().Border(size)) & m_bd.AllEmpty();
}

bool GoRegion::Find2ConnForAll() const {
  if (GetFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY)) {
    const GoPointSet interior = AllInsideLibs();
    GoPointSet libs = (Points() & m_bd.AllEmpty()) - AllInsideLibs();
    for (SgSetIterator it(interior); it; ++it) {
      if (!Find2Libs(*it, &libs))
        return false;
    }
    return true;
  }

  return false;

#if UNUSED
  single = GetFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY);
  if (! single)
  {
      GoBlock* first = Blocks().Front();
      if (Find2ConnForAll(m_bd, Points(), first->Stones(), Color()))
          twoLibs = true;
      else
          is1Vital = false;
  }
  else if (Find2ConnForAll(m_bd, Points(), bd, Color()))
      twoLibs = true;
  else
      is1Vital = false;

#endif
}

bool GoRegion::Find2ConnForAllInterior(SgMiaiStrategy* miaiStrategy,
                                       SgVector<GoPoint>& usedLibs) const {
  SgVector<SgMiaiPair> myStrategy;
  const int size = m_bd.Size();
  GoPointSet interior = AllInsideLibs();
  if (interior.IsEmpty()) {
    return true;
  }
  {
    GoPointSet testSet = interior;
    GoPointSet originalLibs = testSet.Border(size) & Dep().Border(size)
        & m_bd.AllEmpty() & Points();
    GoPointSet updateLibs = originalLibs;
    bool changed = true;
    while (changed) {
      changed = false;
      if (testSet.IsEmpty()) {
        SgVector<GoPoint> jlibs;
        JointLibs(&jlibs);
        SgVector<GoPoint> ips;
        GetIPs(&ips);
        SgVector<SgMiaiPair> updateStrg;

        for (SgSetIterator it(interior); it; ++it) {
          GoPoint p = *it;
          GoPointSet s1;
          s1.Include(p);
          GoPointSet rest = s1.Border(size) & updateLibs;
          if (!rest.IsEmpty()) {
            for (SgVectorIterator<SgMiaiPair> it2(myStrategy);
                 it2; ++it2) {
              SgMiaiPair x = *it2;
              if (GoPointUtil::AreAdjacent(p, x.first)
                  && GoPointUtil::AreAdjacent(p, x.second)
                  ) {
                if (ips.Contains(x.first)) {
                  updateLibs.Include(x.first);
                  usedLibs.Exclude(x.first);
                  GoPoint t = rest.PointOf();
                  x.first = t;
                  updateLibs.Exclude(t);
                  rest.Exclude(t);
                  usedLibs.Include(t);
                }
                if (ips.Contains(x.second)
                    && !rest.IsEmpty()
                    ) {
                  updateLibs.Include(x.second);
                  usedLibs.Exclude(x.second);
                  GoPoint t = rest.PointOf();
                  x.second = t;
                  updateLibs.Exclude(t);
                  rest.Exclude(t);
                  usedLibs.Include(t);
                }
                updateStrg.Include(x);
              }
            }
          }
        }
        miaiStrategy->SetStrategy(updateStrg);
        return true;
      }
      for (SgSetIterator it(interior); it; ++it) {
        SgMiaiPair miaiPair;
        if (Find2BestLibs(*it, updateLibs, testSet, &miaiPair)) {
          if (miaiPair.first == miaiPair.second) {
            SgDebug() << "\nmiaipair are same: "
                      << GoWritePoint(miaiPair.first)
                      << GoWritePoint(miaiPair.second);
            SgDebug() << "\ncurrent region is:\n";
            Points().Write(SgDebug(), size);
            DBG_ASSERT(false);
          }
          myStrategy.PushBack(miaiPair);
          usedLibs.PushBack(miaiPair.first);
          usedLibs.PushBack(miaiPair.second);
          updateLibs.Exclude(miaiPair.first);
          updateLibs.Exclude(miaiPair.second);
          updateLibs.Include(*it);
          testSet.Exclude(*it);
          changed = true;
        }
      }
    }
  }
  miaiStrategy->Clear();
  return false;
}

bool GoRegion::ComputeIs1Vital() const {
  if (GetFlag(GO_REGION_STATIC_1VC))
    return true;
  else if (ComputedFlag(GO_REGION_STATIC_1VITAL))
    return GetFlag(GO_REGION_STATIC_1VITAL);

  bool is1Vital = true;

  if (!HasLibForAllBlocks())
    is1Vital = false;
  else {
    if (const_cast<GoRegion*>(this)
        ->ComputeAndGetFlag(GO_REGION_PROTECTED_CUTS)) {
      is1Vital = true;
    } else if (!Find2ConnForAll())
      is1Vital = false;
  }

  return is1Vital;
}

bool GoRegion::IsCorridor() const {
  DBG_ASSERT(!m_computedFlags.test(GO_REGION_CORRIDOR));
  for (SgSetIterator it(Points()); it; ++it) {
    if ((m_bd.NumNeighbors(*it, SgOppBW(Color()))
        + m_bd.NumEmptyNeighbors(*it)) > 2)
      return false;
    if (m_bd.NumNeighbors(*it, Color()) == 0)
      return false;
  }
  return true;
}

bool GoRegion::ReplaceChain(const GoChain* old, const GoChain* newChain) {
  DBG_ASSERT(old != newChain);
  DBG_ASSERT(Color() == old->Color());
  if (m_chains.Contains(old)) {
    m_computedFlags.reset();
    m_chains.Exclude(old);
    m_chains.Include(newChain);
    if (HEAVYCHECK)
      DBG_ASSERT(m_chains.UniqueElements());

    return true;
  }

  return false;
}

bool GoRegion::Find2Mergable(GoChain** c1, GoChain** c2) const {
  GoChain* test1;
  GoChain* test2;
  for (SgVectorPairIteratorOf<GoChain> it(m_chains);
       it.NextPair(test1, test2);) {
    if (Has2ConnForChains(test1, test2)) {
      *c1 = test1;
      *c2 = test2;
      return true;
    }
  }
  return false;
}

void GoRegion::Find2FreeLibs(const GoChain* c1, const GoChain* c2,
                             GoPoint* lib1, GoPoint* lib2) const {
  GoPointSet libs = Points() & c1->FreeLiberties() & c2->FreeLiberties();
  if (CHECK)
    DBG_ASSERT(libs.MinSetSize(2));
  SgSetIterator it(libs);
  *lib1 = *it;
  ++it;
  *lib2 = *it;
}

void GoRegion::ReInitialize() {
  m_computedFlags.reset();
  m_computedFlags.set(GO_REGION_COMPUTED_BLOCKS);
  m_flags.reset();
  m_miaiStrategy.Clear();
  ComputeBasicFlags();
}

void GoRegion::WriteID(std::ostream& stream) const {
  stream << SgBW(Color()) << " Region "
         << GoWritePoint(Points().Center());
}

const char* kRegionFlagStrings[_GO_REGION_FLAG_COUNT + 1] =
    {
        "isSmall", "GO_REGION_CORRIDOR", "GO_REGION_STATIC_1VC",
        "GO_REGION_1VC", "GO_REGION_STATIC_2V", "GO_REGION_2V",
        "GO_REGION_SINGLE_BLOCK_BOUNDARY",
        "GO_REGION_OPP_CAN_LIVE_INSIDE",
        "GO_REGION_AT_LEAST_SEKI",
        "isSafe",
        "GO_REGION_PROTECTED_CUTS", "GO_REGION_STATIC_1VITAL",
        "is1Vital",
        "GO_REGION_USED_FOR_MERGE",
        "GO_REGION_VALID",
        "GO_REGION_COMPUTED_BLOCKS",
        "GO_REGION_COMPUTED_CHAINS",
        "GO_REGION_COMPUTED_NAKADE",
        "_GO_REGION_FLAG_COUNT"
    };

void GoRegion::Write(std::ostream& stream) const {
  WriteID(stream);
  stream << ", " << Points().Size()
         << " Points\nBlocks:";
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    (*it)->WriteID(stream);
    if ((*it)->ContainsHealthy(this))
      stream << ":Healthy";
  }

  stream << "\nInterior Blocks: ";
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    if (IsInteriorBlock(*it))
      (*it)->WriteID(stream);
  }
  stream << "\nChains: ";
  for (SgVectorIteratorOf<GoChain> it(m_chains); it; ++it) {
    (*it)->WriteID(stream);
    if ((*it)->ContainsHealthy(this))
      stream << ":Healthy";
  }

  stream << "\ncomputed region flags:\n";

  for (int f = 0; f < _GO_REGION_FLAG_COUNT; ++f) {
    if (m_computedFlags.test(f)) {
      stream << SgWriteLabel(kRegionFlagStrings[f])
             << SgWriteBoolean(m_flags.test(f))
             << '\n';
    }
  }
  stream << SgWriteLabel("not computed:");
  bool first = true;
  for (int f = 0; f < _GO_REGION_FLAG_COUNT; ++f) {
    if (!m_computedFlags.test(f)) {
      if (first)
        first = false;
      else
        stream << ", ";

      stream << kRegionFlagStrings[f];
    }
  }
  stream << '\n'
         << SgWriteLabel("eyes:")
         << m_eyes
         << SgWriteLabel("Miai strategy:")
         << m_miaiStrategy
         << '\n';
}

bool GoRegion::GetFlag(GoRegionFlag flag) const {
  DBG_ASSERT(IsValid());
  DBG_ASSERT(m_computedFlags.test(flag));
  return m_flags.test(flag);
}

bool GoRegion::ComputeAndGetFlag(GoRegionFlag flag) {
  DBG_ASSERT(IsValid());
  ComputeFlag(flag);
  return m_flags.test(flag);
}

bool GoRegion::ComputedFlag(GoRegionFlag flag) const {
  return m_computedFlags.test(flag);
}

void GoRegion::ComputeFlag(GoRegionFlag flag) {
  if (!m_computedFlags.test(flag))
    DoComputeFlag(flag);
}

void GoRegion::DoComputeFlag(GoRegionFlag flag) {
  DBG_ASSERT(!m_computedFlags.test(flag));
  switch (flag) {
    case GO_REGION_SMALL:
      SetFlag(GO_REGION_SMALL,
              IsSmallRegion(m_bd, Points(), SgOppBW(Color())));
      break;
    case GO_REGION_CORRIDOR:SetFlag(GO_REGION_CORRIDOR, IsCorridor());
      break;
    case GO_REGION_1VC:DBG_ASSERT(false);
      break;
    case GO_REGION_1VITAL:DBG_ASSERT(false);
      break;
    case GO_REGION_STATIC_1VITAL: {
      bool is = ComputeIs1Vital();
      SetFlag(GO_REGION_STATIC_1VITAL, is);
      if (is)
        SetFlag(GO_REGION_1VITAL, true);
    }
      break;
    case GO_REGION_STATIC_1VC: {
      bool is = StaticIs1VitalAndConnected();
      SetFlag(GO_REGION_STATIC_1VC, is);
      if (is)
        SetFlag(GO_REGION_1VC, true);
    }
      break;
    case GO_REGION_2V:DBG_ASSERT(false);
      break;
    case GO_REGION_STATIC_2V: {
      bool is = Has2SureLibs(&m_miaiStrategy);
      SetFlag(GO_REGION_STATIC_2V, is);
      if (is) {
        SetFlag(GO_REGION_2V, true);
      }
    }
      break;
    case GO_REGION_SINGLE_BLOCK_BOUNDARY:DBG_ASSERT(m_computedFlags.test(GO_REGION_COMPUTED_BLOCKS));
      SetFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY, m_blocks.MaxLength(1));
      break;
    case GO_REGION_OPP_CAN_LIVE_INSIDE:
      SetFlag(GO_REGION_OPP_CAN_LIVE_INSIDE,
              MightMakeLife(m_bd, Points(), Dep(), SgOppBW(Color())));
      break;
    case GO_REGION_AT_LEAST_SEKI:DBG_ASSERT(false);
      break;
    case GO_REGION_SAFE:DBG_ASSERT(false);
      break;
    case GO_REGION_PROTECTED_CUTS:SetFlag(GO_REGION_PROTECTED_CUTS, ProtectedCuts(m_bd));
      break;
      break;
    case GO_REGION_COMPUTED_NAKADE:ComputeNakade();
      SetFlag(GO_REGION_COMPUTED_NAKADE, true);
      break;
    default:DBG_ASSERT(false);
  }
  m_computedFlags.set(flag);
}

void GoRegion::ComputeSingleBlockEyeSpace() {
  DBG_ASSERT(m_blocks.IsLength(1));
  const int nu = Points().Size();
  DBG_ASSERT(nu > 0);

  if (nu <= 2) {
    m_eyes.SetEyes(1, 1);
  } else {
    bool isNakade = false, makeNakade = false, maybeSeki = false;
    bool sureSeki = false;
    bool makeFalse = false;
    if (nu <= 7) {
      GoPoint vitalP(GO_NULLPOINT);
      TestNakade(Points(), m_bd, m_color, true,
                 isNakade, makeNakade, makeFalse,
                 maybeSeki, sureSeki,
                 &vitalP);
      if (makeNakade || makeFalse)
        m_vitalPoint = vitalP;
    }
    if (sureSeki)
      m_eyes.SetLocalSeki();
    else if (maybeSeki) {
      m_eyes.SetMinEyes(0);
      m_eyes.SetMaxEyes(2);
      int potEyes = isNakade ? 1 : 2;
      m_eyes.SetExactPotEyes(potEyes);
      m_eyes.SetMaybeLocalSeki();
    } else if (isNakade || makeNakade) {
      int potEyes = isNakade ? 1 : 2;
      m_eyes.SetEyes(1, potEyes);
    } else if (makeFalse)
      m_eyes.SetEyes(0, 1);

    else {
      m_eyes.SetMinEyes(0);
      m_eyes.SetMaxEyes(2);
      m_eyes.SetExactPotEyes(2);
    }
  }
}

void GoRegion::ComputeMultipleBlockEyeSpace() {
  DBG_ASSERT(m_blocks.MinLength(2));
  const int nu = m_points.Size();
  DBG_ASSERT (nu > 0);

  int minNuEyes = 0;
  bool isNakade = false;
  bool makeNakade = false;
  bool makeFalse = false;

  if (nu <= 2) {
    if (minNuEyes == 1) {
      m_eyes.SetEyes(1, 1);
      return;
    }

    return;
  } else if (nu <= 7) {
    GoPoint vitalP(GO_NULLPOINT);
    bool maybeSeki = false, sureSeki = false;
    TestNakade(m_points, m_bd, m_color, true,
               isNakade, makeNakade,
               makeFalse,
               maybeSeki, sureSeki,
               &vitalP);
    if (makeNakade)
      m_vitalPoint = vitalP;
  }
  if (makeFalse)
    m_eyes.SetEyes(0, 1);
  else if (isNakade)
    m_eyes.SetEyes(1, 1);
  else if (makeNakade)
    m_eyes.SetEyes(1, 2);
  else {
    m_eyes.SetMinEyes(minNuEyes);
    m_eyes.SetMaxEyes(2);
    m_eyes.SetExactPotEyes(2);
  }

}

void GoRegion::ComputeEyeSpace() {
  if (m_blocks.IsLength(1))
    ComputeSingleBlockEyeSpace();
  else
    ComputeMultipleBlockEyeSpace();

  return;
}

void GoRegion::ComputeNakade() {

  if (GetFlag(GO_REGION_STATIC_2V)) {
    m_eyes.SetMinEyes(2);
  } else {
    m_eyes.SetUnknown();

    bool is1vital = ComputeAndGetFlag(GO_REGION_STATIC_1VITAL);
    if (is1vital)
      m_eyes.SetMinEyes(1);

    bool isNakade = false, makeNakade = false;
    bool maybeSeki = false, sureSeki = false;
    bool makeFalse = false;
    int nu = Points().Size();

    if (SgUtil::InRange(nu, 3, 7)) {
      GoPoint vitalP(GO_NULLPOINT);
      TestNakade(m_points, m_bd, m_color, true,
                 isNakade, makeNakade,
                 makeFalse,
                 maybeSeki, sureSeki, &vitalP);
      if (isNakade) {
        m_eyes.SetMaxEyes(1);
        m_eyes.SetMaxPotEyes(1);
      } else if (makeNakade) {
        m_vitalPoint = vitalP;
        m_eyes.SetMinPotEyes(2);
      }
    }
  }

  m_eyes.Normalize();
}

void GoRegion::Finish() {
  GoChain::Finish();
  GoBlock::Finish();
  GoRegionBoard::Finish();
#ifndef NDEBUG
  DBG_ASSERT(s_allocated == s_freed);
#endif
}

#ifndef NDEBUG
int GoRegion::s_allocated = 0;

int GoRegion::s_freed = 0;
#endif

void GoRegion::ComputeBasicFlags() {
  DBG_ASSERT(!IsValid());
  m_flags.set(GO_REGION_VALID);
  DoComputeFlag(GO_REGION_SMALL);
  DoComputeFlag(GO_REGION_CORRIDOR);
  DoComputeFlag(GO_REGION_SINGLE_BLOCK_BOUNDARY);
  DoComputeFlag(GO_REGION_STATIC_1VC);
  DoComputeFlag(GO_REGION_STATIC_2V);
  SetFlag(GO_REGION_USED_FOR_MERGE, false);
}

bool GoRegion::Has2Conn() const {
  DBG_ASSERT(m_chains.IsLength(2));
  const GoChain* c1 = m_chains.Front();
  const GoChain* c2 = m_chains.Back();
  return Has2ConnForChains(c1, c2);
}

bool GoRegion::Has2ConnForChains(const GoChain* c1,
                                 const GoChain* c2) const {
  return (Points()
      & c1->FreeLiberties()
      & c2->FreeLiberties()
  ).MinSetSize(2);
}

bool GoRegion::Safe2Cuts(const GoBoard& board) const {
  DBG_ASSERT(m_blocks.IsLength(2));
  const int size = board.Size();
  GoBlock* block1 = m_blocks.Front();
  GoBlock* block2 = m_blocks.Back();
  GoPointSet cuts(Points());
  cuts -= board.AllEmpty();
  if (cuts.IsEmpty())
    return true;
  cuts &= block1->Stones().Border(size);
  cuts &= block2->Stones().Border(size);
  return cuts.IsEmpty();
}

bool GoRegion::ProtectedCuts(const GoBoard& board) const {
  if (!GetFlag(GO_REGION_CORRIDOR))
    return false;
  if (m_blocks.IsLength(2))
    return Safe2Cuts(board);

  bool prot = true;
  GoPointSet allCuts;
  const int size = board.Size();
  GoBlock* block1, * block2;
  for (SgVectorPairIteratorOf<GoBlock> it(m_blocks);
       it.NextPair(block1, block2);) {
    GoPointSet lib1(block1->Stones().Border(size));
    GoPointSet lib2(block2->Stones().Border(size));
    GoPointSet cuts(lib1 & lib2 & Points());
    if (!cuts.SubsetOf(board.AllEmpty()))
      return false;
    else
      allCuts |= cuts;
  }

  return prot;
}

void GoRegion::FindBlocks(const GoRegionBoard& ra) {
  DBG_ASSERT(m_blocks.IsEmpty());
  const int size = m_bd.Size();
  GoPointSet area(Points().Border(size));

  for (SgVectorIteratorOf<GoBlock> it(ra.AllBlocks(Color())); it; ++it) {
    if ((*it)->Stones().Overlaps(area))
      m_blocks.PushBack(*it);
  }
  m_computedFlags.set(GO_REGION_COMPUTED_BLOCKS);
}

GoPointSet GoRegion::BlocksPoints() const {
  GoPointSet points;
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it)
    points |= (*it)->Stones();
  return points;
}

void GoRegion::SetBlocks(const SgVectorOf<GoBlock>& blocks) {
  DBG_ASSERT(m_blocks.IsEmpty());
  DBG_ASSERT(!blocks.IsEmpty());
  const int size = m_bd.Size();
  GoPointSet area(Points().Border(size));
  for (SgVectorIteratorOf<GoBlock> it(blocks); it; ++it) {
    if ((*it)->Stones().Overlaps(area))
      m_blocks.PushBack(*it);
  }
  m_computedFlags.set(GO_REGION_COMPUTED_BLOCKS);
}

void GoRegion::FindChains(const GoRegionBoard& ra) {
  DBG_ASSERT(m_chains.IsEmpty());
  const int size = m_bd.Size();
  GoPointSet area(Points().Border(size));
  for (SgVectorIteratorOf<GoChain> it(ra.AllChains(Color())); it; ++it) {
    if ((*it)->Stones().Overlaps(area))
      m_chains.PushBack(*it);
  }
  m_computedFlags.set(GO_REGION_COMPUTED_CHAINS);
}

bool GoRegion::IsSurrounded(const SgVectorOf<GoBlock>& blocks) const {
  const int size = m_bd.Size();
  GoPointSet adj(Points().Border(size));
  for (SgVectorIteratorOf<GoBlock> it(blocks); it; ++it)
    adj -= (*it)->Stones();
  return adj.IsEmpty();
}

bool GoRegion::HealthyForSomeBlock(const SgVectorOf<GoBlock>& blocks) const {
  for (SgVectorIteratorOf<GoBlock> it(blocks); it; ++it)
    if ((*it)->ContainsHealthy(this))
      return true;
  return false;
}

bool GoRegion::SomeBlockIsSafe() const {
  for (SgVectorIteratorOf<GoBlock> it(Blocks()); it; ++it)
    if ((*it)->IsSafe())
      return true;
  return false;
}

bool GoRegion::AllBlockIsSafe() const {
  for (SgVectorIteratorOf<GoBlock> it(Blocks()); it; ++it)
    if (!(*it)->IsSafe())
      return false;
  return true;
}

bool GoRegion::ComputedVitalForDepth(int depth) const {
  return GetFlag(GO_REGION_STATIC_1VC)
      || (ComputedFlag(GO_REGION_1VC) && m_1vcDepth >= depth);
}

void GoRegion::CheckConsistency() const {
  DBG_ASSERT(Points().Disjoint(m_bd.All(Color())));
  DBG_ASSERT(Points().Border(m_bd.Size()).SubsetOf(m_bd.All(Color())));
  DBG_ASSERT(Points().IsConnected());
  GoPointSet blockPts;
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    DBG_ASSERT(AdjacentToBlock((*it)->Anchor()));
    blockPts |= (*it)->Stones();
  }
  DBG_ASSERT(Points().Border(m_bd.Size()).SubsetOf(blockPts));
}

void GoRegion::RemoveBlock(const GoBlock* b) {
  bool found = m_blocks.Exclude(b);
  SuppressUnused(found);
  DBG_ASSERT(found);
  ResetNonBlockFlags();
}

bool GoRegion::AdjacentToSomeBlock(const SgVector<GoPoint>& anchors) const {
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    if (anchors.Contains((*it)->Anchor()))
      return true;
  }
  return false;
}

bool GoRegion::AdjacentToBlock(GoPoint anchor) const {
  for (SgVectorIteratorOf<GoBlock> it(m_blocks); it; ++it) {
    if ((*it)->Anchor() == anchor)
      return true;
  }
  return false;
}

void GoRegion::OnAddStone(GoPoint p) {
  DBG_ASSERT(m_points.Contains(p));
  m_points.Exclude(p);
  ResetNonBlockFlags();
}

void GoRegion::OnRemoveStone(GoPoint p) {
  DBG_ASSERT(!m_points.Contains(p));
  m_points.Include(p);
  ResetNonBlockFlags();
}

std::ostream& operator<<(std::ostream& stream, const GoRegion& r) {
  r.Write(stream);
  return stream;
}

