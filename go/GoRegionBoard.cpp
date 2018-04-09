

#include "platform/SgSystem.h"
#include "GoRegionBoard.h"

#include <iostream>
#include "GoBlock.h"
#include "GoChain.h"
#include "GoEyeUtil.h"
#include "GoRegion.h"
#include "platform/SgDebug.h"
#include "board/GoConnCompIterator.h"
#include "board/SgWrite.h"

namespace {
static const bool CHECK = SG_CHECK && true;
static const bool HEAVYCHECK = SG_HEAVYCHECK && CHECK && false;
static const bool DEBUG_REGION_BOARD = false;
const int REGION_CODE_BASE = 2000;
const int REGION_REMOVE = REGION_CODE_BASE + 0;
const int REGION_ADD = REGION_CODE_BASE + 1;
const int REGION_REMOVE_BLOCK = REGION_CODE_BASE + 2;
const int REGION_ADD_BLOCK = REGION_CODE_BASE + 3;
const int REGION_ADD_STONE = REGION_CODE_BASE + 4;
const int REGION_ADD_STONE_TO_BLOCK = REGION_CODE_BASE + 5;
}

bool GoRegionBoard::IsSafeBlock(GoPoint p) const {
  return BlockAt(p)->IsSafe();
}

void GoRegionBoard::SetToSafe(GoPoint p) const {
  BlockAt(p)->SetToSafe();
}

void GoRegionBoard::Finish() {
  DBG_ASSERT(s_alloc == s_free);
}

void GoRegionBoard::SetSafeFlags(const GoBWSet& safe) {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
      if ((*it)->Points().Overlaps(safe[color]))
        (*it)->SetToSafe();
    for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it)
      if ((*it)->Stones().Overlaps(safe[color]))
        (*it)->SetToSafe();
  }
}

GoRegionBoard::GoRegionBoard(const GoBoard& board)
    : m_board(board),
      m_region(GoPointArray<GoRegion*>(0)),
      m_block(0),
      m_invalid(true),
      m_computedHealthy(false),
      m_boardSize(board.Size()) {
  m_code.Clear();
  m_chainsCode.Clear();
  GenBlocksRegions();
  ++s_alloc;
}

GoRegionBoard::~GoRegionBoard() {
  Clear();
  ++s_free;
}

void GoRegionBoard::Clear() {
  if (DEBUG_REGION_BOARD)
    SgDebug() << "Clear\n";
  for (SgBWIterator it; it; ++it) {
    SgBlackWhite color(*it);
    for (SgVectorIteratorOf<GoBlock> it1(AllBlocks(color)); it1; ++it1)
      delete *it1;
    for (SgVectorIteratorOf<GoRegion> it2(AllRegions(color)); it2; ++it2)
      delete *it2;
    for (SgVectorIteratorOf<GoChain> it3(AllChains(color)); it3; ++it3)
      delete *it3;
  }
  m_allBlocks[SG_BLACK].Clear();
  m_allBlocks[SG_WHITE].Clear();
  m_allRegions[SG_BLACK].Clear();
  m_allRegions[SG_WHITE].Clear();
  m_allChains[SG_BLACK].Clear();
  m_allChains[SG_WHITE].Clear();
  m_stack.Clear();
  m_code.Clear();
  m_invalid = true;
  m_computedHealthy = false;
  m_boardSize = m_board.Size();

  for (SgBWIterator it; it; ++it) {
    SgBlackWhite color(*it);
    for (GoPoint p = GoPointUtil::Pt(1, 1);
         p <= GoPointUtil::Pt(GO_MAX_SIZE, GO_MAX_SIZE);
         ++p) {
      m_region[color][p] = 0;
    }
  }
  for (GoPoint p = GoPointUtil::Pt(1, 1);
       p <= GoPointUtil::Pt(GO_MAX_SIZE, GO_MAX_SIZE); ++p)
    m_block[p] = 0;
}

void GoRegionBoard::UpdateBlock(int move, SgBlackWhite moveColor) {
  GoPoint anchor = Board().Anchor(move);
  bool done = false;
  const int size = Board().Size();
  if (!Board().IsSingleStone(move)) {
    SgVectorOf<GoBlock> old;
    for (GoNbIterator it(Board(), move); it; ++it) {
      if (IsColor(*it, moveColor))
        old.Include(BlockAt(*it));
    }
    if (old.IsLength(1)) {
      GoBlock* b = old.Front();
      AppendStone(b, move);
      done = true;
    } else {
      for (SgVectorIteratorOf<GoBlock> it(old); it; ++it)
        RemoveBlock(*it, true, true);
    }
  }

  if (!done) {
    GoBlock* b = GenBlock(anchor, moveColor);
    GoPointSet area(b->Stones().Border(size));
    SgVectorOf<GoRegion> regions;
    RegionsAt(area, moveColor, &regions);
    for (SgVectorIteratorOf<GoRegion> it(regions); it; ++it)
      (*it)->BlocksNonConst().PushBack(b);
  }
}

void GoRegionBoard::AppendStone(GoBlock* b, GoPoint move) {
  m_block[move] = b;
  b->AddStone(move);
  m_stack.PushInt(move);
  m_stack.PushPtrEvent(REGION_ADD_STONE_TO_BLOCK, b);
}

void GoRegionBoard::OnExecutedMove(GoPlayerMove move) {
  OnExecutedUncodedMove(move.Point(), move.Color());
}

void GoRegionBoard::ExecuteMovePrologue() {
  if (!UpToDate()) {
    if (DEBUG_REGION_BOARD)
      SgDebug() << "recompute everything\n";
    GenBlocksRegions();
  }
}

void GoRegionBoard::OnExecutedUncodedMove(int move, SgBlackWhite moveColor) {
  if (DEBUG_REGION_BOARD)
    SgDebug() << "OnExecutedUncodedMove " << GoWritePoint(move) << '\n';
  {
    m_stack.StartMoveInfo();
    if (move != GO_PASS) {
      DBG_ASSERT(!Board().LastMoveInfo(GO_MOVEFLAG_SUICIDE));
      bool fWasCapture = Board().LastMoveInfo(GO_MOVEFLAG_CAPTURING);

      UpdateBlock(move, moveColor);

      {
        GoRegion* r = PreviousRegionAt(move, moveColor);
        bool split = GoEyeUtil::IsSplitPt(move, r->Points());

        r->OnAddStone(move);
        PushStone(r, move);
        GoPointSet points = r->Points();
        if (split || points.IsEmpty())
          RemoveRegion(r);

        if (split) {
          for (GoConnCompIterator it(points, Board().Size());
               it; ++it)
            GenRegion(*it, moveColor);
        }
      }

      if (fWasCapture) {
        MergeAdjacentAndAddBlock(move, SgOppBW(moveColor));
      }

      m_code = Board().GetHashCode();
      if (HEAVYCHECK)
        CheckConsistency();
    }
  }

  {
    for (SgBWIterator cit; cit; ++cit) {
      SgBlackWhite color(*cit);
      for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it) {
        GoRegion* r1 = *it;
        if (!r1->IsValid())
          r1->ComputeBasicFlags();
      }
    }
  }
}

void GoRegionBoard::CheckConsistency() const {
  DBG_ASSERT(CHECK && UpToDate());
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    GoPointSet blockArea, regionArea;
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it) {
      DBG_ASSERT((*it)->Points().Disjoint(regionArea));
      DBG_ASSERT((*it)->Points().Disjoint(blockArea));
      (*it)->CheckConsistency();
      regionArea |= (*it)->Points();
    }
    for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it) {
      DBG_ASSERT((*it)->Stones().Disjoint(regionArea));
      DBG_ASSERT((*it)->Stones().Disjoint(blockArea));
      for (SgSetIterator it2((*it)->Stones()); it2; ++it2)
        DBG_ASSERT(m_block[*it2] == *it);
      (*it)->CheckConsistency();
      blockArea |= (*it)->Stones();
    }
    DBG_ASSERT(blockArea.Disjoint(regionArea));
    DBG_ASSERT((blockArea | regionArea) == Board().AllPoints());
    for (GoPoint p = GoPointUtil::Pt(1, 1);
         p <= GoPointUtil::Pt(GO_MAX_SIZE, GO_MAX_SIZE);
         ++p) {
      DBG_ASSERT((m_region[color][p] == 0) != regionArea.Contains(p));
    }

  }
  for (GoPoint p = GoPointUtil::Pt(1, 1);
       p <= GoPointUtil::Pt(GO_MAX_SIZE, GO_MAX_SIZE); ++p) {
    GoBlock* block = m_block[p];
    SG_DEBUG_ONLY(block);
    DBG_ASSERT((block != 0) == Board().Occupied().Contains(p));
  }
}

void GoRegionBoard::PushRegion(int type, GoRegion* r) {
  m_stack.PushPtrEvent(type, r);
}

void GoRegionBoard::PushBlock(int type, GoBlock* b) {
  m_stack.PushPtrEvent(type, b);
}

void GoRegionBoard::PushStone(GoRegion* r, GoPoint move) {
  m_stack.PushInt(move);
  m_stack.PushPtrEvent(REGION_ADD_STONE, r);
  m_region[r->Color()][move] = 0;
}

void GoRegionBoard::RemoveRegion(GoRegion* r, bool isExecute) {
  SgBlackWhite color = r->Color();

  for (SgSetIterator reg(r->Points()); reg; ++reg) {
    m_region[color][*reg] = 0;
  }

  bool found = m_allRegions[r->Color()].Exclude(r);
  SuppressUnused(found);
  DBG_ASSERT(found);
  for (SgVectorIteratorOf<GoBlock> it(r->Blocks()); it; ++it)
    (*it)->RemoveRegion(r);

  if (isExecute)
    PushRegion(REGION_REMOVE, r);
  else
    delete r;
}

void GoRegionBoard::SetRegionArrays(GoRegion* r) {
  SgBlackWhite color = r->Color();

  for (SgSetIterator reg(r->Points()); reg; ++reg) {
    m_region[color][*reg] = r;
  }
}

void GoRegionBoard::RemoveBlock(GoBlock* b, bool isExecute,
                                bool removeFromRegions) {
  SgBlackWhite color = b->Color();
  for (SgSetIterator it(b->Stones()); it; ++it)
    m_block[*it] = 0;

  bool found = m_allBlocks[color].Exclude(b);
  SuppressUnused(found);
  DBG_ASSERT(found);
  const int size = Board().Size();
  GoPointSet area(b->Stones().Border(size));
  SgVectorOf<GoRegion> regions;
  if (removeFromRegions) {
    RegionsAt(area, color, &regions);
    for (SgVectorIteratorOf<GoRegion> it(regions); it; ++it) {
      (*it)->RemoveBlock(b);
      if (isExecute)
        m_stack.PushPtr(*it);
    }
  }
  if (isExecute) {
    m_stack.PushInt(regions.Length());
    PushBlock(REGION_REMOVE_BLOCK, b);
  } else
    delete b;
}

void GoRegionBoard::AddBlock(GoBlock* b, bool isExecute) {
  SgBlackWhite color = b->Color();
  AllBlocks(color).PushBack(b);
  for (GoBoard::StoneIterator it(Board(), b->Anchor()); it; ++it)
    m_block[*it] = b;
  if (isExecute)
    PushBlock(REGION_ADD_BLOCK, b);
}

GoBlock* GoRegionBoard::GenBlock(GoPoint anchor, SgBlackWhite color) {
  auto* b = new GoBlock(color, anchor, Board());
  AddBlock(b);
  return b;
}

void
GoRegionBoard::PreviousBlocksAt(const SgVector<GoPoint>& area,
                                SgBlackWhite color,
                                SgVectorOf<GoBlock>* captures) const {
  SuppressUnused(color);

  for (SgVectorIterator<GoPoint> it(area); it; ++it) {
    GoBlock* b = m_block[*it];
    if (b)
      captures->Include(b);
  }
}

#if UNUSED
void GoRegionBoard::BlockToRegion(GoBlock* b)
{
    GoRegion* r = new GoRegion(Board(), b->Stones(), OppBW(b->Color()));
    SetRegionArrays(r);
}

void GoRegionBoard::FindNewNeighborRegions(GoPoint move,
                                                BlackWhite moveColor)
{

    SgVector<GoPoint> nb;
    for (Nb4Iterator it(move); it; ++it)
        if (Board().IsEmpty(*it))
            nb.PushBack(*it);

    SgVectorOf<GoBlock> captures;
    PreviousBlocksAt(nb, OppBW(moveColor), &captures);
    DBG_ASSERT(captures.NonEmpty());

    for (SgVectorIteratorOf<GoBlock> it(captures); it; ++it)
        BlockToRegion(*it);
}
#endif

void GoRegionBoard::RegionsAt(const GoPointSet& area, SgBlackWhite color,
                              SgVectorOf<GoRegion>* regions) const {
  for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
    if ((*it)->Points().Overlaps(area))
      regions->PushBack(*it);
}

void GoRegionBoard::AdjacentRegions(const SgVector<GoPoint>& anchors,
                                    SgBlackWhite color,
                                    SgVectorOf<GoRegion>* regions) const {
  for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
    if ((*it)->AdjacentToSomeBlock(anchors))
      regions->PushBack(*it);
}

GoRegion* GoRegionBoard::MergeAll(const SgVectorOf<GoRegion>& regions,
                                  const GoPointSet& captured,
                                  SgBlackWhite color) {
  GoPointSet area(captured);
  for (SgVectorIteratorOf<GoRegion> it(regions); it; ++it)
    area |= (*it)->Points();
  {
    for (SgVectorIteratorOf<GoRegion> it(regions); it; ++it)
      RemoveRegion(*it);
  }
  GoRegion* r = GenRegion(area, color);

  return r;
}

void GoRegionBoard::MergeAdjacentAndAddBlock(GoPoint move,
                                             SgBlackWhite capturedColor) {
  SgVector<GoPoint> nb;
  for (GoNbIterator it(Board(), move); it; ++it)
    if (Board().IsEmpty(*it))
      nb.PushBack(*it);

  SgVectorOf<GoBlock> captures;
  PreviousBlocksAt(nb, capturedColor, &captures);
  DBG_ASSERT(captures.NonEmpty());

  GoPointSet captured;
  {
    for (SgVectorIteratorOf<GoBlock> it(captures); it; ++it)
      captured |= (*it)->Stones();
  }
  SgVectorOf<GoRegion> adj;
  const int size = Board().Size();
  RegionsAt(captured.Border(size), capturedColor, &adj);
  DBG_ASSERT(adj.NonEmpty());
  GoRegion* r = MergeAll(adj, captured, capturedColor);
  SuppressUnused(r);

  for (SgVectorIteratorOf<GoBlock> it(captures); it; ++it)
    RemoveBlock(*it, true, false);
}

void GoRegionBoard::OnUndoneMove() {
  if (DEBUG_REGION_BOARD)
    SgDebug() << "OnUndoneMove " << '\n';

  const bool IS_UNDO = false;
  SgVectorOf<GoRegion> changed;

  for (int val = m_stack.PopEvent(); val != SG_NEXTMOVE;
       val = m_stack.PopEvent()) {

    switch (val) {
      case REGION_REMOVE: {
        auto* r = static_cast<GoRegion*>(m_stack.PopPtr());
        AddRegion(r, IS_UNDO);
        changed.Insert(r);
      }
        break;
      case REGION_ADD: {
        auto* r = static_cast<GoRegion*>(m_stack.PopPtr());
        RemoveRegion(r, IS_UNDO);
      }
        break;
      case REGION_REMOVE_BLOCK: {
        auto* b = static_cast<GoBlock*>(m_stack.PopPtr());
        AddBlock(b, IS_UNDO);
        for (int nu = m_stack.PopInt(); nu > 0; --nu) {
          auto* r = static_cast<GoRegion*>(m_stack.PopPtr());
          if (CHECK)
            DBG_ASSERT(!r->Blocks().Contains(b));
          r->BlocksNonConst().PushBack(b);
          changed.Insert(r);
        }
      }
        break;
      case REGION_ADD_BLOCK: {
        auto* b = static_cast<GoBlock*>(m_stack.PopPtr());
        RemoveBlock(b, IS_UNDO, true);
      }
        break;
      case REGION_ADD_STONE: {
        auto* r = static_cast<GoRegion*>(m_stack.PopPtr());
        GoPoint p = m_stack.PopInt();
        r->OnRemoveStone(p);
        m_region[r->Color()][p] = r;
        changed.Insert(r);
      }
        break;
      case REGION_ADD_STONE_TO_BLOCK: {
        auto* b = static_cast<GoBlock*>(m_stack.PopPtr());
        GoPoint p = m_stack.PopInt();
        b->RemoveStone(p);
        m_block[p] = 0;
      }
        break;
      default:DBG_ASSERT(false);
    }
  }

  for (SgVectorIteratorOf<GoRegion> it(changed); it; ++it) {
    (*it)->ResetNonBlockFlags();
    (*it)->ComputeBasicFlags();
  }

  if (HEAVYCHECK) {
    for (SgBWIterator cit; cit; ++cit) {
      SgBlackWhite color(*cit);
      for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it) {
        const GoRegion* r = *it;
        SuppressUnused(r);
        DBG_ASSERT(r->IsValid());
      }
    }
  }

  m_code = Board().GetHashCode();
  if (HEAVYCHECK)
    CheckConsistency();
}

void GoRegionBoard::ReInitializeBlocksRegions() {
  DBG_ASSERT(UpToDate());

  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it)
      (*it)->ReInitialize();
    for (SgVectorIteratorOf<GoRegion> it2(AllRegions(color)); it2; ++it2)
      (*it2)->ReInitialize();
  }
}

GoRegion* GoRegionBoard::GenRegion(const GoPointSet& area,
                                   SgBlackWhite color) {
  auto* r = new GoRegion(Board(), area, color);
  AddRegion(r);
  return r;
}

void GoRegionBoard::AddRegion(GoRegion* r, bool isExecute) {
  SetRegionArrays(r);

  if (isExecute) {
    r->FindBlocks(*this);
    r->ComputeBasicFlags();
  }

  SgBlackWhite color = r->Color();
  if (HEAVYCHECK)
    DBG_ASSERT(!AllRegions(color).Contains(r));
  AllRegions(color).PushBack(r);

  if (isExecute)
    PushRegion(REGION_ADD, r);
}

void GoRegionBoard::FindBlocksWithEye() {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
      if ((*it)->Blocks().IsLength(1)) {
        GoBlock* b = (*it)->Blocks().Front();
        (b)->TestFor1Eye(*it);
      }
  }
}

void GoRegionBoard::GenBlocksRegions() {
  if (UpToDate())
    return;

  Clear();
  GenBlocks();

  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (GoConnCompIterator it(AllPoints() - All(color), Board().Size());
         it; ++it)
      GenRegion(*it, color);
  }

  FindBlocksWithEye();

  m_code = Board().GetHashCode();
  m_invalid = false;
  if (HEAVYCHECK)
    CheckConsistency();
}

void GoRegionBoard::GenChains() {
  if (ChainsUpToDate())
    return;

  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    DBG_ASSERT(AllChains(color).IsEmpty());

    for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it)
      AllChains(color).PushBack(new GoChain(*it, Board()));
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
      (*it)->FindChains(*this);
  }
  m_chainsCode = Board().GetHashCode();
}

void GoRegionBoard::WriteBlocks(std::ostream& stream) const {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it)
      stream << **it;
  }
}

void GoRegionBoard::WriteRegions(std::ostream& stream) const {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
      (*it)->Write(stream);
  }
}

GoBlock* GoRegionBoard::GetBlock(const GoPointSet& boundary,
                                 SgBlackWhite color) const {
  DBG_ASSERT(UpToDate());

  for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it) {
    if (boundary.SubsetOf((*it)->Stones()))
      return *it;
  }

  SgDebug() << "ERROR: no block on set";
  const int size = Board().Size();
  boundary.Write(SgDebug(), size);
  SgDebug() << "blocks:";
  for (SgVectorIteratorOf<GoBlock> it(AllBlocks(color)); it; ++it)
    (*it)->Stones().Write(SgDebug(), size);

  DBG_ASSERT(false);
  return 0;
}

GoChain* GoRegionBoard::ChainAt(GoPoint p) const {
  DBG_ASSERT(ChainsUpToDate());
  const SgBlackWhite color = Board().GetStone(p);

  for (SgVectorIteratorOf<GoChain> it(AllChains(color)); it; ++it)
    if ((*it)->Stones().Contains(p))
      return *it;

  DBG_ASSERT(false);
  return 0;
}

void GoRegionBoard::GenBlocks() {
  for (GoBlockIterator it(Board()); it; ++it)
    GenBlock(*it, Board().GetStone(*it));
}

void GoRegionBoard::SetComputedFlagForAll(GoRegionFlag flag) {
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite color(*cit);
    for (SgVectorIteratorOf<GoRegion> it(AllRegions(color)); it; ++it)
      (*it)->SetComputedFlag(flag);
  }
}

void GoRegionBoard::SetComputedHealthy() {
  m_computedHealthy = true;
}

int GoRegionBoard::s_alloc = 0;
int GoRegionBoard::s_free = 0;
