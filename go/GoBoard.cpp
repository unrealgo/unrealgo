

#include "platform/SgSystem.h"
#include "GoBoard.h"

#include <boost/static_assert.hpp>
#include <algorithm>
#include "GoInit.h"
#include "board/GoNbIterator.h"
#include "lib/SgStack.h"

namespace {

const bool CONSISTENCY = false;

void UpdateChanges(GoPoint p, GoArrayList<GoPoint, GO_MAXPOINT>& changes,
                   int& nuChanges) {
  if (!changes.Exclude(p))
    changes.PushBack(p);
  ++nuChanges;
}

}

GoBoard::GoBoard(int size, const GoSetup& setup, const GoRules& rules)
    : m_snapshot(new Snapshot()),
      m_const(size),
      m_blockList(new GoArrayList<Block, GO_MAX_NUM_MOVES>()),
      m_moves(new GoArrayList<StackEntry, GO_MAX_NUM_MOVES>()) {
  GoInitCheck();
  Init(size, rules, setup);
}

GoBoard::~GoBoard() {
  delete m_blockList;
  m_blockList = 0;
  delete m_moves;
  m_moves = 0;
}

void GoBoard::CheckConsistency() const {
  if (!CONSISTENCY)
    return;
  int numberBlack = 0;
  int numberWhite = 0;
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    if (IsBorder(p))
      continue;
    int c = m_state.m_color[p];
    DBG_ASSERT_EBW(c);
    int n = 0;
    for (SgNb4Iterator it(p); it; ++it)
      if (m_state.m_color[*it] == SG_EMPTY)
        ++n;
    DBG_ASSERT(n == NumEmptyNeighbors(p));
    n = 0;
    for (SgNb4Iterator it(p); it; ++it)
      if (m_state.m_color[*it] == SG_BLACK)
        ++n;
    DBG_ASSERT(n == NumNeighbors(p, SG_BLACK));
    n = 0;
    for (SgNb4Iterator it(p); it; ++it)
      if (m_state.m_color[*it] == SG_WHITE)
        ++n;
    DBG_ASSERT(n == NumNeighbors(p, SG_WHITE));
    if (c == SG_BLACK || c == SG_WHITE) {
      DBG_ASSERT(m_state.m_all[c].Contains(p));
      CheckConsistencyBlock(p);
    }
    if (c == SG_BLACK)
      ++numberBlack;
    if (c == SG_WHITE)
      ++numberWhite;
    if (c == SG_EMPTY)
      DBG_ASSERT(m_state.m_block[p] == 0);
  }
  DBG_ASSERT(m_state.m_all[SG_BLACK].Size() == numberBlack);
  DBG_ASSERT(m_state.m_all[SG_WHITE].Size() == numberWhite);
}

void GoBoard::CheckConsistencyBlock(GoPoint point) const {
  DBG_ASSERT(Occupied(point));
  SgBlackWhite color = GetColor(point);
  GoPointList stones;
  Block::LibertyList liberties;
  SgMarker mark;
  SgStack<GoPoint, GO_MAXPOINT> stack;
  stack.Push(point);
  while (!stack.IsEmpty()) {
    GoPoint p = stack.Pop();
    if (IsBorder(p) || !mark.NewMark(p))
      continue;
    if (GetColor(p) == color) {
      stones.PushBack(p);
      stack.Push(p - GO_NORTH_SOUTH);
      stack.Push(p - GO_WEST_EAST);
      stack.Push(p + GO_WEST_EAST);
      stack.Push(p + GO_NORTH_SOUTH);
    } else if (GetColor(p) == SG_EMPTY)
      liberties.PushBack(p);
  }
  const Block* block = m_state.m_block[point];
  SG_DEBUG_ONLY(block);
  DBG_ASSERT(stones.Contains(block->Anchor()));
  DBG_ASSERT(color == block->Color());
  DBG_ASSERT(stones.SameElements(block->Stones()));
  DBG_ASSERT(liberties.SameElements(block->Liberties()));
  DBG_ASSERT(stones.Length() == NumStones(point));
}

bool GoBoard::CheckKo(SgBlackWhite player) {
  if (!FullBoardRepetition())
    return true;
  m_moveInfo.set(GO_MOVEFLAG_REPETITION);
  if (AnyRepetitionAllowed()) {
    if (m_koLoser != SG_EMPTY && m_koLoser != player)
      ++m_state.m_koLevel;
    return true;
  }
  if (KoRepetitionAllowed()
      && (m_koLoser != player)
      && (!m_koModifiesHash || (m_state.m_koLevel < MAX_KOLEVEL))
      && (m_koColor != SgOppBW(player))) {
    ++m_state.m_koLevel;
    m_koColor = player;
    if (m_koModifiesHash)
      m_state.m_hash.XorWinKo(m_state.m_koLevel, m_koColor);
    return true;
  }
  return false;
}

void GoBoard::AddLibToAdjBlocks(GoPoint p) {
  if (NumNeighbors(p, SG_BLACK) + NumNeighbors(p, SG_WHITE) == 0)
    return;
  GoArrayList<Block*, 4> blocks = GetAdjacentBlocks(p);
  for (GoArrayList<Block*, 4>::Iterator it(blocks); it; ++it) {
    Block* block = *it;
    if (block != 0)
      block->AppendLiberty(p);
  }
}

void GoBoard::AddLibToAdjBlocks(GoPoint p, SgBlackWhite c) {
  if (NumNeighbors(p, c) == 0)
    return;
  GoArrayList<Block*, 4> blocks = GetAdjacentBlocks(p, c);
  for (GoArrayList<Block*, 4>::Iterator it(blocks); it; ++it) {
    Block* block = *it;
    if (block != 0)
      block->AppendLiberty(p);
  }
}

void GoBoard::AddStoneToBlock(GoPoint p, SgBlackWhite c, Block* block,
                              StackEntry& entry) {
  SG_DEBUG_ONLY(c);
  DBG_ASSERT(IsColor(p, c));
  block->AppendStone(p);
  entry.m_newLibs.Clear();
  if (IsEmpty(p - GO_NORTH_SOUTH) && !IsAdjacentTo(p - GO_NORTH_SOUTH, block)) {
    block->AppendLiberty(p - GO_NORTH_SOUTH);
    entry.m_newLibs.PushBack(p - GO_NORTH_SOUTH);
  }
  if (IsEmpty(p - GO_WEST_EAST) && !IsAdjacentTo(p - GO_WEST_EAST, block)) {
    block->AppendLiberty(p - GO_WEST_EAST);
    entry.m_newLibs.PushBack(p - GO_WEST_EAST);
  }
  if (IsEmpty(p + GO_WEST_EAST) && !IsAdjacentTo(p + GO_WEST_EAST, block)) {
    block->AppendLiberty(p + GO_WEST_EAST);
    entry.m_newLibs.PushBack(p + GO_WEST_EAST);
  }
  if (IsEmpty(p + GO_NORTH_SOUTH) && !IsAdjacentTo(p + GO_NORTH_SOUTH, block)) {
    block->AppendLiberty(p + GO_NORTH_SOUTH);
    entry.m_newLibs.PushBack(p + GO_NORTH_SOUTH);
  }
  entry.m_oldAnchor = block->Anchor();
  block->UpdateAnchor(p);
  m_state.m_block[p] = block;
}

GoBoard::Block& GoBoard::CreateNewBlock() {
  m_blockList->Resize(m_blockList->Length() + 1);
  Block& block = m_blockList->Last();
  return block;
}

void GoBoard::CreateSingleStoneBlock(GoPoint p, SgBlackWhite c) {
  DBG_ASSERT(IsColor(p, c));
  DBG_ASSERT(NumNeighbors(p, c) == 0);
  Block& block = CreateNewBlock();
  block.Init(c, p);
  if (IsEmpty(p - GO_NORTH_SOUTH))
    block.AppendLiberty(p - GO_NORTH_SOUTH);
  if (IsEmpty(p - GO_WEST_EAST))
    block.AppendLiberty(p - GO_WEST_EAST);
  if (IsEmpty(p + GO_WEST_EAST))
    block.AppendLiberty(p + GO_WEST_EAST);
  if (IsEmpty(p + GO_NORTH_SOUTH))
    block.AppendLiberty(p + GO_NORTH_SOUTH);
  m_state.m_block[p] = &block;
}

GoArrayList<GoBoard::Block*, 4> GoBoard::GetAdjacentBlocks(GoPoint p) const {
  GoArrayList<Block*, 4> result;
  if (NumNeighbors(p, SG_BLACK) > 0 || NumNeighbors(p, SG_WHITE) > 0) {
    Block* block;
    if ((block = m_state.m_block[p - GO_NORTH_SOUTH]) != 0)
      result.PushBack(block);
    if ((block = m_state.m_block[p - GO_WEST_EAST]) != 0
        && !result.Contains(block))
      result.PushBack(block);
    if ((block = m_state.m_block[p + GO_WEST_EAST]) != 0
        && !result.Contains(block))
      result.PushBack(block);
    if ((block = m_state.m_block[p + GO_NORTH_SOUTH]) != 0
        && !result.Contains(block))
      result.PushBack(block);
  }
  return result;
}

GoArrayList<GoBoard::Block*, 4> GoBoard::GetAdjacentBlocks(GoPoint p,
                                                           SgBlackWhite c) const {
  GoArrayList<Block*, 4> result;
  if (NumNeighbors(p, c) > 0) {
    Block* block;
    if (IsColor(p - GO_NORTH_SOUTH, c))
      result.PushBack(m_state.m_block[p - GO_NORTH_SOUTH]);
    if (IsColor(p - GO_WEST_EAST, c)
        && !result.Contains((block = m_state.m_block[p - GO_WEST_EAST])))
      result.PushBack(block);
    if (IsColor(p + GO_WEST_EAST, c)
        && !result.Contains((block = m_state.m_block[p + GO_WEST_EAST])))
      result.PushBack(block);
    if (IsColor(p + GO_NORTH_SOUTH, c)
        && !result.Contains((block = m_state.m_block[p + GO_NORTH_SOUTH])))
      result.PushBack(block);
  }
  return result;
}

bool GoBoard::IsAdjacentTo(GoPoint p, const GoBoard::Block* block) const {
  return m_state.m_block[p - GO_NORTH_SOUTH] == block
      || m_state.m_block[p - GO_WEST_EAST] == block
      || m_state.m_block[p + GO_WEST_EAST] == block
      || m_state.m_block[p + GO_NORTH_SOUTH] == block;
}

void GoBoard::MergeBlocks(GoPoint p, SgBlackWhite c,
                          const GoArrayList<Block*, 4>& adjBlocks) {
  DBG_ASSERT(IsColor(p, c));
  DBG_ASSERT(NumNeighbors(p, c) > 1);
  Block& block = CreateNewBlock();
  block.Init(c, p);
  SgReserveMarker reserve(m_marker);
  SuppressUnused(reserve);
  m_marker.Clear();
  for (GoArrayList<Block*, 4>::Iterator it(adjBlocks); it; ++it) {
    Block* adjBlock = *it;
    for (Block::StoneIterator stn(adjBlock->Stones()); stn; ++stn) {
      block.AppendStone(*stn);
      m_state.m_block[*stn] = &block;
    }
    for (Block::LibertyIterator lib(adjBlock->Liberties()); lib; ++lib)
      if (m_marker.NewMark(*lib))
        block.AppendLiberty(*lib);
    block.UpdateAnchor(adjBlock->Anchor());
  }
  m_state.m_block[p] = &block;
  if (IsEmpty(p - GO_NORTH_SOUTH) && m_marker.NewMark(p - GO_NORTH_SOUTH))
    block.AppendLiberty(p - GO_NORTH_SOUTH);
  if (IsEmpty(p - GO_WEST_EAST) && m_marker.NewMark(p - GO_WEST_EAST))
    block.AppendLiberty(p - GO_WEST_EAST);
  if (IsEmpty(p + GO_WEST_EAST) && m_marker.NewMark(p + GO_WEST_EAST))
    block.AppendLiberty(p + GO_WEST_EAST);
  if (IsEmpty(p + GO_NORTH_SOUTH) && m_marker.NewMark(p + GO_NORTH_SOUTH))
    block.AppendLiberty(p + GO_NORTH_SOUTH);
}

void GoBoard::RemoveLibFromAdjBlocks(GoPoint p, SgBlackWhite c) {
  if (NumNeighbors(p, c) == 0)
    return;
  GoArrayList<Block*, 4> blocks = GetAdjacentBlocks(p, c);
  for (GoArrayList<Block*, 4>::Iterator it(blocks); it; ++it)
    (*it)->ExcludeLiberty(p);
}

void GoBoard::RestoreKill(Block* block, SgBlackWhite c) {
  SgBlackWhite opp = SgOppBW(c);
  for (Block::StoneIterator it(block->Stones()); it; ++it) {
    GoPoint stn = *it;
    AddStone(stn, c);
    m_state.m_block[stn] = block;
    RemoveLibFromAdjBlocks(stn, opp);
  }
  int nuStones = block->Stones().Length();
  m_state.m_numStones[c] += nuStones;
  m_state.m_prisoners[c] -= nuStones;
  DBG_ASSERT(m_state.m_prisoners[c] >= 0);
}

void GoBoard::UpdateBlocksAfterAddStone(GoPoint p, SgBlackWhite c,
                                        StackEntry& entry) {
  DBG_ASSERT(IsColor(p, c));
  if (NumNeighbors(p, c) == 0) {
    entry.m_stoneAddedTo = 0;
    CreateSingleStoneBlock(p, c);
    entry.m_merged.Clear();
  } else {
    GoArrayList<Block*, 4> adjBlocks = GetAdjacentBlocks(p, c);
    if (adjBlocks.Length() == 1) {
      Block* block = adjBlocks[0];
      AddStoneToBlock(p, c, block, entry);
      entry.m_stoneAddedTo = block;
    } else {
      entry.m_stoneAddedTo = 0;
      MergeBlocks(p, c, adjBlocks);
      entry.m_merged = adjBlocks;
    }
  }
}

void GoBoard::UpdateBlocksAfterUndo(const StackEntry& entry) {
  GoPoint p = entry.m_point;
  if (IsPass(p))
    return;
  SgBlackWhite c = entry.m_color;
  {
    Block* block = entry.m_suicide;
    if (block != 0)
      RestoreKill(block, c);
  }
  RemoveStone(p);
  --m_state.m_numStones[c];
  m_state.m_block[p] = 0;
  Block* stoneAddedTo = entry.m_stoneAddedTo;
  if (stoneAddedTo != 0) {
    stoneAddedTo->PopStone();
    for (GoArrayList<GoPoint, 4>::Iterator it(entry.m_newLibs); it; ++it)
      stoneAddedTo->ExcludeLiberty(*it);
    stoneAddedTo->SetAnchor(entry.m_oldAnchor);
  } else {
    for (GoArrayList<Block*, 4>::Iterator it(entry.m_merged); it; ++it) {
      Block* block = *it;
      for (Block::StoneIterator stn(block->Stones()); stn; ++stn)
        m_state.m_block[*stn] = block;
    }
    m_blockList->PopBack();
  }
  for (GoArrayList<Block*, 4>::Iterator it(entry.m_killed); it; ++it)
    RestoreKill(*it, SgOppBW(entry.m_color));
  AddLibToAdjBlocks(p);
}

void GoBoard::Init(int size, const GoRules& rules, const GoSetup& setup) {
  m_rules = rules;
  m_size = size;
  DBG_ASSERTRANGE(m_size, GO_MIN_SIZE, GO_MAX_SIZE);
  m_state.m_hash.Clear();
  m_moves->Clear();
  m_state.m_prisoners[SG_BLACK] = 0;
  m_state.m_prisoners[SG_WHITE] = 0;
  m_state.m_numStones[SG_BLACK] = 0;
  m_state.m_numStones[SG_WHITE] = 0;
  m_countPlay = 0;
  m_state.m_koPoint = GO_NULLPOINT;
  m_allowAnyRepetition = false;
  m_allowKoRepetition = false;
  m_koColor = SG_EMPTY;
  m_koLoser = SG_EMPTY;
  m_state.m_koLevel = 0;
  m_koModifiesHash = true;
  m_const.ChangeSize(m_size);
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    m_state.m_color[p] = SG_BORDER;
    m_isBorder[p] = true;
  }
  m_state.m_isFirst.Fill(true);
  m_state.m_isNewPosition = true;
  for (GoGrid row = 1; row <= m_size; ++row)
    for (GoGrid col = 1; col <= m_size; ++col) {
      GoPoint p = GoPointUtil::Pt(col, row);
      m_state.m_color[p] = SG_EMPTY;
      m_isBorder[p] = false;
    }
  m_state.m_all.Clear();
  m_state.m_empty.Clear();
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    m_state.m_nuNeighbors[SG_BLACK][p] = 0;
    m_state.m_nuNeighbors[SG_WHITE][p] = 0;
    if (IsBorder(p))
      m_state.m_nuNeighborsEmpty[p] = 4;
    else {
      m_state.m_empty.Include(p);
      m_state.m_nuNeighborsEmpty[p] = 0;
      for (SgNb4Iterator it(p); it; ++it)
        if (IsEmpty(*it))
          ++m_state.m_nuNeighborsEmpty[p];
    }
  }
  m_setup = setup;
  for (SgBWIterator c; c; ++c)
    for (SgSetIterator it(setup.m_stones[*c]); it; ++it) {
      GoPoint p = *it;
      DBG_ASSERT(IsValidPoint(p));
      DBG_ASSERT(IsEmpty(p));
      AddStone(p, *c);
      ++m_state.m_numStones[*c];
      m_state.m_hash.XorStone(p, *c);
      m_state.m_isFirst[p] = false;
    }
  m_state.m_toPlay = setup.m_player;
  m_blockList->Clear();
  m_state.m_block.Fill(0);
  for (GoBoard::Iterator it(*this); it; ++it) {
    SgBoardColor c = m_state.m_color[*it];
    if (c != SG_EMPTY && m_state.m_block[*it] == 0) {
      GoBoard::Block& block = CreateNewBlock();
      InitBlock(block, c, *it);
    }
  }
  m_snapshot->m_moveNumber = -1;
  CheckConsistency();
}

void GoBoard::InitBlock(GoBoard::Block& block, SgBlackWhite c, GoPoint anchor) {
  DBG_ASSERT_BW(c);
  Block::LibertyList liberties;
  GoPointList stones;
  SgReserveMarker reserve(m_marker);
  m_marker.Clear();
  SgStack<GoPoint, GO_MAX_ONBOARD> stack;
  stack.Push(anchor);
  m_marker.NewMark(anchor);
  while (!stack.IsEmpty()) {
    GoPoint p = stack.Pop();
    if (m_state.m_color[p] == SG_EMPTY) {
      if (!liberties.Contains(p))
        liberties.PushBack(p);
    } else if (m_state.m_color[p] == c) {
      stones.PushBack(p);
      m_state.m_block[p] = &block;
      for (GoNbIterator it(*this, p); it; ++it)
        if (m_marker.NewMark(*it))
          stack.Push(*it);
    }
  }
  block.Init(c, anchor, stones, liberties);
}

GoPlayerMove GoBoard::Move(int i) const {
  const StackEntry& entry = (*m_moves)[i];
  GoPoint p = entry.m_point;
  SgBlackWhite c = entry.m_color;
  return GoPlayerMove(c, p);
}

bool GoBoard::StackOverflowLikely() const {
  return (MoveNumber() > GO_MAX_NUM_MOVES - 50);
}

int GoBoard::AdjacentBlocks(GoPoint point, int maxLib, GoPoint anchors[],
                            int maxAnchors) const {
  SG_DEBUG_ONLY(maxAnchors);
  DBG_ASSERT(Occupied(point));
  const SgBlackWhite other = SgOppBW(GetStone(point));
  int n = 0;
  SgReserveMarker reserve(m_marker);
  SuppressUnused(reserve);
  m_marker.Clear();
  for (GoBoard::StoneIterator it(*this, point); it; ++it) {
    if (NumNeighbors(*it, other) > 0) {
      GoPoint p = *it;
      if (IsColor(p - GO_NORTH_SOUTH, other)
          && m_marker.NewMark(Anchor(p - GO_NORTH_SOUTH))
          && AtMostNumLibs(p - GO_NORTH_SOUTH, maxLib))
        anchors[n++] = Anchor(p - GO_NORTH_SOUTH);
      if (IsColor(p - GO_WEST_EAST, other)
          && m_marker.NewMark(Anchor(p - GO_WEST_EAST))
          && AtMostNumLibs(p - GO_WEST_EAST, maxLib))
        anchors[n++] = Anchor(p - GO_WEST_EAST);
      if (IsColor(p + GO_WEST_EAST, other)
          && m_marker.NewMark(Anchor(p + GO_WEST_EAST))
          && AtMostNumLibs(p + GO_WEST_EAST, maxLib))
        anchors[n++] = Anchor(p + GO_WEST_EAST);
      if (IsColor(p + GO_NORTH_SOUTH, other)
          && m_marker.NewMark(Anchor(p + GO_NORTH_SOUTH))
          && AtMostNumLibs(p + GO_NORTH_SOUTH, maxLib))
        anchors[n++] = Anchor(p + GO_NORTH_SOUTH);
    }
  };
  DBG_ASSERT(n < maxAnchors);
  anchors[n] = GO_ENDPOINT;
  return n;
}

void GoBoard::NeighborBlocks(GoPoint p, SgBlackWhite c,
                             GoPoint anchors[]) const {
  DBG_ASSERT(IsEmpty(p));
  SgReserveMarker reserve(m_marker);
  SuppressUnused(reserve);
  m_marker.Clear();
  int i = 0;
  if (NumNeighbors(p, c) > 0) {
    if (IsColor(p - GO_NORTH_SOUTH, c) && m_marker.NewMark(Anchor(p - GO_NORTH_SOUTH)))
      anchors[i++] = Anchor(p - GO_NORTH_SOUTH);
    if (IsColor(p - GO_WEST_EAST, c) && m_marker.NewMark(Anchor(p - GO_WEST_EAST)))
      anchors[i++] = Anchor(p - GO_WEST_EAST);
    if (IsColor(p + GO_WEST_EAST, c) && m_marker.NewMark(Anchor(p + GO_WEST_EAST)))
      anchors[i++] = Anchor(p + GO_WEST_EAST);
    if (IsColor(p + GO_NORTH_SOUTH, c) && m_marker.NewMark(Anchor(p + GO_NORTH_SOUTH)))
      anchors[i++] = Anchor(p + GO_NORTH_SOUTH);
  }
  anchors[i] = GO_ENDPOINT;
}

void GoBoard::NeighborBlocks(GoPoint p, SgBlackWhite c, int maxLib,
                             GoPoint anchors[]) const {
  DBG_ASSERT(IsEmpty(p));
  SgReserveMarker reserve(m_marker);
  SuppressUnused(reserve);
  m_marker.Clear();
  int i = 0;
  if (NumNeighbors(p, c) > 0) {
    if (IsColor(p - GO_NORTH_SOUTH, c) && m_marker.NewMark(Anchor(p - GO_NORTH_SOUTH))
        && AtMostNumLibs(p - GO_NORTH_SOUTH, maxLib))
      anchors[i++] = Anchor(p - GO_NORTH_SOUTH);
    if (IsColor(p - GO_WEST_EAST, c) && m_marker.NewMark(Anchor(p - GO_WEST_EAST))
        && AtMostNumLibs(p - GO_WEST_EAST, maxLib))
      anchors[i++] = Anchor(p - GO_WEST_EAST);
    if (IsColor(p + GO_WEST_EAST, c) && m_marker.NewMark(Anchor(p + GO_WEST_EAST))
        && AtMostNumLibs(p + GO_WEST_EAST, maxLib))
      anchors[i++] = Anchor(p + GO_WEST_EAST);
    if (IsColor(p + GO_NORTH_SOUTH, c) && m_marker.NewMark(Anchor(p + GO_NORTH_SOUTH))
        && AtMostNumLibs(p + GO_NORTH_SOUTH, maxLib))
      anchors[i++] = Anchor(p + GO_NORTH_SOUTH);
  }
  anchors[i] = GO_ENDPOINT;
}

void GoBoard::AddStone(GoPoint p, SgBlackWhite c) {
  DBG_ASSERT(IsEmpty(p));
  DBG_ASSERT_BW(c);
  m_state.m_color[p] = c;
  m_state.m_empty.Exclude(p);
  m_state.m_all[c].Include(p);
  --m_state.m_nuNeighborsEmpty[p - GO_NORTH_SOUTH];
  --m_state.m_nuNeighborsEmpty[p - GO_WEST_EAST];
  --m_state.m_nuNeighborsEmpty[p + GO_WEST_EAST];
  --m_state.m_nuNeighborsEmpty[p + GO_NORTH_SOUTH];
  GoArray<int, GO_MAXPOINT>& nuNeighbors = m_state.m_nuNeighbors[c];
  ++nuNeighbors[p - GO_NORTH_SOUTH];
  ++nuNeighbors[p - GO_WEST_EAST];
  ++nuNeighbors[p + GO_WEST_EAST];
  ++nuNeighbors[p + GO_NORTH_SOUTH];
}

void GoBoard::RemoveStone(GoPoint p) {
  SgBlackWhite c = GetStone(p);
  DBG_ASSERT_BW(c);
  m_state.m_color[p] = SG_EMPTY;
  m_state.m_empty.Include(p);
  m_state.m_all[c].Exclude(p);
  ++m_state.m_nuNeighborsEmpty[p - GO_NORTH_SOUTH];
  ++m_state.m_nuNeighborsEmpty[p - GO_WEST_EAST];
  ++m_state.m_nuNeighborsEmpty[p + GO_WEST_EAST];
  ++m_state.m_nuNeighborsEmpty[p + GO_NORTH_SOUTH];
  GoArray<int, GO_MAXPOINT>& nuNeighbors = m_state.m_nuNeighbors[c];
  --nuNeighbors[p - GO_NORTH_SOUTH];
  --nuNeighbors[p - GO_WEST_EAST];
  --nuNeighbors[p + GO_WEST_EAST];
  --nuNeighbors[p + GO_NORTH_SOUTH];
}

void GoBoard::KillBlock(const Block* block) {
  SgBlackWhite c = block->Color();
  SgBlackWhite opp = SgOppBW(c);
  for (Block::StoneIterator it(block->Stones()); it; ++it) {
    GoPoint stn = *it;
    AddLibToAdjBlocks(stn, opp);
    m_state.m_hash.XorStone(stn, c);
    RemoveStone(stn);
    m_capturedStones.PushBack(stn);
    m_state.m_block[stn] = 0;
  }
  int nuStones = block->Stones().Length();
  m_state.m_numStones[c] -= nuStones;
  m_state.m_prisoners[c] += nuStones;
  if (nuStones == 1)
    m_state.m_koPoint = block->Anchor();
}

bool GoBoard::FullBoardRepetition() const {
  GoRules::KoRule koRule = Rules().GetKoRule();
  if (koRule == GoRules::SIMPLEKO) {
    int nuMoves = MoveNumber();
    if (nuMoves == 0)
      return false;
    const StackEntry& entry = (*m_moves)[nuMoves - 1];
    return (entry.m_point == entry.m_koPoint);
  }
  SgBWArray<GoArrayList<GoPoint, GO_MAXPOINT> > changes;
  int nuChanges = 0;
  int moveNumber = m_moves->Length() - 1;
  bool requireSameToPlay = (koRule == GoRules::SUPERKO);
  while (moveNumber >= 0) {
    const StackEntry& entry = (*m_moves)[moveNumber];
    GoPoint p = entry.m_point;
    if (!IsPass(p) && entry.m_color != SG_EMPTY
        && entry.m_color == GetColor(p)
        && entry.m_isFirst)
      return false;

    if (!IsPass(entry.m_point)) {
      UpdateChanges(entry.m_point, changes[entry.m_color], nuChanges);
      for (GoArrayList<Block*, 4>::Iterator it(entry.m_killed); it; ++it) {
        Block* killed = *it;
        for (GoPointList::Iterator stn(killed->Stones()); stn; ++stn)
          UpdateChanges(*stn, changes[killed->Color()], nuChanges);
      }
      Block* suicide = entry.m_suicide;
      if (suicide != 0)
        for (GoPointList::Iterator stn(suicide->Stones()); stn;
             ++stn)
          UpdateChanges(*stn, changes[suicide->Color()], nuChanges);
    }

    if (nuChanges > 0
        && (!requireSameToPlay || entry.m_toPlay == m_state.m_toPlay)
        && changes[SG_BLACK].IsEmpty() && changes[SG_WHITE].IsEmpty())
      return true;
    --moveNumber;
  }
  return false;
}

bool GoBoard::CheckSuicide(GoPoint p, StackEntry& entry) {
  if (!HasLiberties(p)) {
    entry.m_suicide = m_state.m_block[p];
    KillBlock(entry.m_suicide);
    m_moveInfo.set(GO_MOVEFLAG_SUICIDE);
    return m_rules.AllowSuicide();
  }
  return true;
}

void GoBoard::Play(GoPoint p, SgBlackWhite player) {
  DBG_ASSERT(p != GO_NULLMOVE);
  DBG_ASSERT_BW(player);
  DBG_ASSERT(IsPass(p) || (IsValidPoint(p) && IsEmpty(p)));
  CheckConsistency();
  ++m_countPlay;
  m_moves->Resize(m_moves->Length() + 1);
  StackEntry& entry = m_moves->Last();
  entry.m_point = p;
  entry.m_color = player;
  SaveState(entry);
  m_state.m_koPoint = GO_NULLPOINT;
  m_capturedStones.Clear();
  m_moveInfo.reset();
  SgBlackWhite opp = SgOppBW(player);
  if (IsPass(p)) {
    m_state.m_toPlay = opp;
    return;
  }
  bool isLegal = true;
  bool wasFirstStone = IsFirst(p);
  m_state.m_isFirst[p] = false;
  m_state.m_hash.XorStone(p, player);
  AddStone(p, player);
  ++m_state.m_numStones[player];
  RemoveLibAndKill(p, opp, entry);
  if (!entry.m_killed.IsEmpty()) {
    m_moveInfo.set(GO_MOVEFLAG_CAPTURING);
    m_state.m_isNewPosition = m_state.m_isNewPosition && wasFirstStone;
  }
  UpdateBlocksAfterAddStone(p, player, entry);
  entry.m_suicide = 0;
  if (m_state.m_koPoint != GO_NULLPOINT)
    if (NumStones(p) > 1 || NumLiberties(p) > 1)
      m_state.m_koPoint = GO_NULLPOINT;
  isLegal = CheckSuicide(p, entry);
  m_state.m_toPlay = opp;
  if (!wasFirstStone && !IsNewPosition() && !CheckKo(player))
    isLegal = false;
  if (!isLegal)
    m_moveInfo.set(GO_MOVEFLAG_ILLEGAL);
  if (!m_capturedStones.IsEmpty() && m_koModifiesHash) {
    GoPoint firstCapturedStone = m_capturedStones[0];
    m_state.m_hash.XorCaptured(MoveNumber(), firstCapturedStone);
  }
  CheckConsistency();
}

void GoBoard::Undo() {
  CheckConsistency();
  const StackEntry& entry = m_moves->Last();
  RestoreState(entry);
  UpdateBlocksAfterUndo(entry);
  m_moves->PopBack();
  CheckConsistency();
}

void GoBoard::RemoveLibAndKill(GoPoint p, SgBlackWhite opp,
                               StackEntry& entry) {
  entry.m_killed.Clear();
  if (NumNeighbors(p, SG_BLACK) == 0 && NumNeighbors(p, SG_WHITE) == 0)
    return;
  GoArrayList<Block*, 4> blocks = GetAdjacentBlocks(p);
  for (GoArrayList<Block*, 4>::Iterator it(blocks); it; ++it) {
    Block* b = *it;
    b->ExcludeLiberty(p);
    if (b->Color() == opp && b->NumLiberties() == 0) {
      entry.m_killed.PushBack(b);
      KillBlock(b);
    }
  }
}

void GoBoard::RestoreState(const StackEntry& entry) {
  m_state.m_hash = entry.m_hash;
  m_state.m_koPoint = entry.m_koPoint;
  if (!IsPass(entry.m_point)) {
    m_state.m_isFirst[entry.m_point] = entry.m_isFirst;
    m_state.m_isNewPosition = entry.m_isNewPosition;
  }
  m_state.m_toPlay = entry.m_toPlay;
  m_state.m_koLevel = entry.m_koLevel;
  m_koColor = entry.m_koColor;
  m_koLoser = entry.m_koLoser;
  m_koModifiesHash = entry.m_koModifiesHash;
}

void GoBoard::SaveState(StackEntry& entry) {
  entry.m_hash = m_state.m_hash;
  if (!IsPass(entry.m_point)) {
    entry.m_isFirst = m_state.m_isFirst[entry.m_point];
    entry.m_isNewPosition = m_state.m_isNewPosition;
  }
  entry.m_toPlay = m_state.m_toPlay;
  entry.m_koPoint = m_state.m_koPoint;
  entry.m_koLevel = m_state.m_koLevel;
  entry.m_koColor = m_koColor;
  entry.m_koLoser = m_koLoser;
  entry.m_koModifiesHash = m_koModifiesHash;
}

void GoBoard::TakeSnapshot() {
  m_snapshot->m_moveNumber = MoveNumber();
  m_snapshot->m_blockListSize = m_blockList->Length();
  m_snapshot->m_state = m_state;
  for (GoBoard::Iterator it(*this); it; ++it) {
    GoPoint p = *it;
    if (m_state.m_block[p] != 0)
      m_snapshot->m_blockArray[p] = *m_state.m_block[p];
  }
}

void GoBoard::RestoreSnapshot() {
  DBG_ASSERT(m_snapshot->m_moveNumber >= 0);
  DBG_ASSERT(m_snapshot->m_moveNumber <= MoveNumber());
  if (m_snapshot->m_moveNumber == MoveNumber())
    return;
  m_blockList->Resize(m_snapshot->m_blockListSize);
  m_moves->Resize(m_snapshot->m_moveNumber);
  m_state = m_snapshot->m_state;
  for (GoBoard::Iterator it(*this); it; ++it) {
    GoPoint p = *it;
    if (m_state.m_block[p] != 0)
      *m_state.m_block[p] = m_snapshot->m_blockArray[p];
  }
  CheckConsistency();
}
