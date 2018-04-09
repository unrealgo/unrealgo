
#include "platform/SgSystem.h"
#include "GoUctBoard.h"

#include <boost/static_assert.hpp>
#include <algorithm>
#include "GoBoardUtil.h"
#include "board/GoNbIterator.h"
#include "lib/SgStack.h"

namespace {
const bool CONSISTENCY = false;
}

GoUctBoard::GoUctBoard(const GoBoard &bd)
    : m_const(bd.Size()) {
  m_size = -1;
  Init(bd);
}

GoUctBoard::~GoUctBoard() {}

void GoUctBoard::CheckConsistency() const {
  if (!CONSISTENCY)
    return;
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    if (IsBorder(p))
      continue;
    int c = m_color[p];
    DBG_ASSERT_EBW(c);
    int n = 0;
    for (SgNb4Iterator it(p); it; ++it)
      if (m_color[*it] == SG_EMPTY)
        ++n;
    DBG_ASSERT(n == NumEmptyNeighbors(p));
    n = 0;

    for (SgNb4Iterator it(p); it; ++it)
      if (m_color[*it] == SG_BLACK)
        ++n;

    DBG_ASSERT(n == NumNeighbors(p, SG_BLACK));
    n = 0;

    for (SgNb4Iterator it(p); it; ++it)
      if (m_color[*it] == SG_WHITE)
        ++n;
    DBG_ASSERT(n == NumNeighbors(p, SG_WHITE));

    if (c == SG_BLACK || c == SG_WHITE)
      CheckConsistencyBlock(p);
    if (c == SG_EMPTY)
      DBG_ASSERT(m_block[p] == 0);
  }
}

void GoUctBoard::CheckConsistencyBlock(GoPoint point) const {
  DBG_ASSERT(Occupied(point));
  SgBlackWhite color = GetColor(point);
  GoPointList stones;
  Block::LibertyList liberties;
  SgMarker mark;
  SgStack<GoPoint, GO_MAXPOINT> stack;
  stack.Push(point);
  bool anchorFound = false;
  SG_DEBUG_ONLY(anchorFound);
  const Block *block = m_block[point];
  while (!stack.IsEmpty()) {
    GoPoint p = stack.Pop();
    if (IsBorder(p) || !mark.NewMark(p))
      continue;
    if (GetColor(p) == color) {
      stones.PushBack(p);
      if (p == block->m_anchor)
        anchorFound = true;
      stack.Push(p - GO_NORTH_SOUTH);
      stack.Push(p - GO_WEST_EAST);
      stack.Push(p + GO_WEST_EAST);
      stack.Push(p + GO_NORTH_SOUTH);
    } else if (GetColor(p) == SG_EMPTY)
      liberties.PushBack(p);
  }
  DBG_ASSERT(anchorFound);
  DBG_ASSERT(color == block->m_color);
  DBG_ASSERT(stones.SameElements(block->m_stones));
  DBG_ASSERT(liberties.SameElements(block->m_liberties));
  DBG_ASSERT(stones.Length() == NumStones(point));
}

void GoUctBoard::AddLibToAdjBlocks(GoPoint p, SgBlackWhite c) {
  if (NumNeighbors(p, c) == 0)
    return;
  SgReserveMarker reserve(m_marker2);
  m_marker2.Clear();
  Block *b;
  if (m_color[p - GO_NORTH_SOUTH] == c && (b = m_block[p - GO_NORTH_SOUTH]) != 0) {
    m_marker2.Include(b->m_anchor);
    b->m_liberties.PushBack(p);
  }
  if (m_color[p + GO_NORTH_SOUTH] == c && (b = m_block[p + GO_NORTH_SOUTH]) != 0
      && m_marker2.NewMark(b->m_anchor))
    b->m_liberties.PushBack(p);
  if (m_color[p - GO_WEST_EAST] == c && (b = m_block[p - GO_WEST_EAST]) != 0
      && m_marker2.NewMark(b->m_anchor))
    b->m_liberties.PushBack(p);
  if (m_color[p + GO_WEST_EAST] == c && (b = m_block[p + GO_WEST_EAST]) != 0
      && !m_marker2.Contains(b->m_anchor))
    b->m_liberties.PushBack(p);
}

void GoUctBoard::AddStoneToBlock(GoPoint p, Block *block) {
  DBG_ASSERT(IsColor(p, block->m_color));
  block->m_stones.PushBack(p);
  if (IsEmpty(p - GO_NORTH_SOUTH) && !IsAdjacentTo(p - GO_NORTH_SOUTH, block))
    block->m_liberties.PushBack(p - GO_NORTH_SOUTH);
  if (IsEmpty(p - GO_WEST_EAST) && !IsAdjacentTo(p - GO_WEST_EAST, block))
    block->m_liberties.PushBack(p - GO_WEST_EAST);
  if (IsEmpty(p + GO_WEST_EAST) && !IsAdjacentTo(p + GO_WEST_EAST, block))
    block->m_liberties.PushBack(p + GO_WEST_EAST);
  if (IsEmpty(p + GO_NORTH_SOUTH) && !IsAdjacentTo(p + GO_NORTH_SOUTH, block))
    block->m_liberties.PushBack(p + GO_NORTH_SOUTH);
  m_block[p] = block;
}

void GoUctBoard::CreateSingleStoneBlock(GoPoint p, SgBlackWhite c) {
  DBG_ASSERT(IsColor(p, c));
  DBG_ASSERT(NumNeighbors(p, c) == 0);
  Block &block = m_blockArray[p];
  block.InitSingleStoneBlock(c, p);
  if (IsEmpty(p - GO_NORTH_SOUTH))
    block.m_liberties.PushBack(p - GO_NORTH_SOUTH);
  if (IsEmpty(p - GO_WEST_EAST))
    block.m_liberties.PushBack(p - GO_WEST_EAST);
  if (IsEmpty(p + GO_WEST_EAST))
    block.m_liberties.PushBack(p + GO_WEST_EAST);
  if (IsEmpty(p + GO_NORTH_SOUTH))
    block.m_liberties.PushBack(p + GO_NORTH_SOUTH);
  m_block[p] = &block;
}

bool GoUctBoard::IsAdjacentTo(GoPoint p,
                              const GoUctBoard::Block *block) const {
  return m_block[p - GO_NORTH_SOUTH] == block
      || m_block[p - GO_WEST_EAST] == block
      || m_block[p + GO_WEST_EAST] == block
      || m_block[p + GO_NORTH_SOUTH] == block;
}

void GoUctBoard::MergeBlocks(GoPoint p, const GoArrayList<Block *, 4> &adjBlocks) {

  DBG_ASSERT(IsColor(p, adjBlocks[0]->m_color));
  DBG_ASSERT(NumNeighbors(p, adjBlocks[0]->m_color) > 1);
  Block *largestBlock = nullptr;
  int largestBlockStones = 0;
  for (GoArrayList<Block *, 4>::Iterator it(adjBlocks); it; ++it) {
    Block *adjBlock = *it;
    int numStones = adjBlock->m_stones.Length();
    if (numStones > largestBlockStones) {
      largestBlockStones = numStones;
      largestBlock = adjBlock;
    }
  }
  if (largestBlock != nullptr) {
    largestBlock->m_stones.PushBack(p);
    SgReserveMarker reserve(m_marker);
    m_marker.Clear();
    for (Block::LibertyIterator lib(largestBlock->m_liberties); lib; ++lib)
      m_marker.Include(*lib);
    for (GoArrayList<Block *, 4>::Iterator it(adjBlocks); it; ++it) {
      Block *adjBlock = *it;
      if (adjBlock == largestBlock)
        continue;
      for (Block::StoneIterator stn(adjBlock->m_stones); stn; ++stn) {
        largestBlock->m_stones.PushBack(*stn);
        m_block[*stn] = largestBlock;
      }
      for (Block::LibertyIterator lib(adjBlock->m_liberties); lib; ++lib)
        if (m_marker.NewMark(*lib))
          largestBlock->m_liberties.PushBack(*lib);
    }
    m_block[p] = largestBlock;
    if (IsEmpty(p - GO_NORTH_SOUTH) && m_marker.NewMark(p - GO_NORTH_SOUTH))
      largestBlock->m_liberties.PushBack(p - GO_NORTH_SOUTH);
    if (IsEmpty(p - GO_WEST_EAST) && m_marker.NewMark(p - GO_WEST_EAST))
      largestBlock->m_liberties.PushBack(p - GO_WEST_EAST);
    if (IsEmpty(p + GO_WEST_EAST) && m_marker.NewMark(p + GO_WEST_EAST))
      largestBlock->m_liberties.PushBack(p + GO_WEST_EAST);
    if (IsEmpty(p + GO_NORTH_SOUTH) && m_marker.NewMark(p + GO_NORTH_SOUTH))
      largestBlock->m_liberties.PushBack(p + GO_NORTH_SOUTH);
  }
}

void GoUctBoard::UpdateBlocksAfterAddStone(GoPoint p, SgBlackWhite c,
                                           const GoArrayList<Block *, 4> &adjBlocks) {

  DBG_ASSERT(IsColor(p, c));
  int n = adjBlocks.Length();
  if (n == 0)
    CreateSingleStoneBlock(p, c);
  else {
    if (n == 1)
      AddStoneToBlock(p, adjBlocks[0]);
    else
      MergeBlocks(p, adjBlocks);
  }
}

void GoUctBoard::Init(const GoBoard &bd) {
  if (bd.Size() != m_size)
    InitSize(bd);
  m_prisoners[SG_BLACK] = bd.NumPrisoners(SG_BLACK);
  m_prisoners[SG_WHITE] = bd.NumPrisoners(SG_WHITE);
  m_koPoint = bd.KoPoint();
  m_lastMove = bd.GetLastMove();
  m_secondLastMove = bd.Get2ndLastMove();
  m_toPlay = bd.ToPlay();
  for (GoBoard::Iterator it(bd); it; ++it) {
    const GoPoint p = *it;
    const SgBoardColor c = bd.GetColor(p);
    m_color[p] = c;
    m_nuNeighbors[SG_BLACK][p] = bd.NumNeighbors(p, SG_BLACK);
    m_nuNeighbors[SG_WHITE][p] = bd.NumNeighbors(p, SG_WHITE);
    m_nuNeighborsEmpty[p] = bd.NumEmptyNeighbors(p);
    if (bd.IsEmpty(p))
      m_block[p] = 0;
    else if (bd.Anchor(p) == p) {
      DBG_ASSERT(c == m_color[p]);
      Block &block = m_blockArray[p];
      block.InitNewBlock(c, p);
      for (GoBoard::StoneIterator it2(bd, p); it2; ++it2) {
        block.m_stones.PushBack(*it2);
        m_block[*it2] = &block;
      }
      for (GoBoard::LibertyIterator it2(bd, p); it2; ++it2)
        block.m_liberties.PushBack(*it2);
    }
  }
  CheckConsistency();
}

void GoUctBoard::InitSize(const GoBoard &bd) {
  m_size = bd.Size();
  m_nuNeighbors[SG_BLACK].Fill(0);
  m_nuNeighbors[SG_WHITE].Fill(0);
  m_nuNeighborsEmpty.Fill(0);
  m_block.Fill(0);
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    if (bd.IsBorder(p)) {
      m_color[p] = SG_BORDER;
      m_isBorder[p] = true;
    } else
      m_isBorder[p] = false;
  }
  m_const.ChangeSize(m_size);
}

void GoUctBoard::NeighborBlocks(GoPoint p, SgBlackWhite c,
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

void GoUctBoard::AddStone(GoPoint p, SgBlackWhite c) {
  DBG_ASSERT(IsEmpty(p));
  DBG_ASSERT_BW(c);
  m_color[p] = c;
  --m_nuNeighborsEmpty[p - GO_NORTH_SOUTH];
  --m_nuNeighborsEmpty[p - GO_WEST_EAST];
  --m_nuNeighborsEmpty[p + GO_WEST_EAST];
  --m_nuNeighborsEmpty[p + GO_NORTH_SOUTH];
  GoArray<int, GO_MAXPOINT> &nuNeighbors = m_nuNeighbors[c];
  ++nuNeighbors[p - GO_NORTH_SOUTH];
  ++nuNeighbors[p - GO_WEST_EAST];
  ++nuNeighbors[p + GO_WEST_EAST];
  ++nuNeighbors[p + GO_NORTH_SOUTH];
}

void GoUctBoard::RemoveLibertyAndKill(GoPoint p, SgBlackWhite opp,
                                      GoArrayList<Block *, 4> &ownAdjBlocks) {
  SgReserveMarker reserve(m_marker);
  m_marker.Clear();
  Block *b;
  if ((b = m_block[p - GO_NORTH_SOUTH]) != 0) {
    m_marker.Include(b->m_anchor);
    b->m_liberties.Exclude(p);
    if (b->m_color == opp) {
      if (b->m_liberties.Length() == 0)
        KillBlock(b);
    } else
      ownAdjBlocks.PushBack(b);
  }
  if ((b = m_block[p - GO_WEST_EAST]) != 0 && m_marker.NewMark(b->m_anchor)) {
    b->m_liberties.Exclude(p);
    if (b->m_color == opp) {
      if (b->m_liberties.Length() == 0)
        KillBlock(b);
    } else
      ownAdjBlocks.PushBack(b);
  }
  if ((b = m_block[p + GO_WEST_EAST]) != 0 && m_marker.NewMark(b->m_anchor)) {
    b->m_liberties.Exclude(p);
    if (b->m_color == opp) {
      if (b->m_liberties.Length() == 0)
        KillBlock(b);
    } else
      ownAdjBlocks.PushBack(b);
  }
  if ((b = m_block[p + GO_NORTH_SOUTH]) != 0 && !m_marker.Contains(b->m_anchor)) {
    b->m_liberties.Exclude(p);
    if (b->m_color == opp) {
      if (b->m_liberties.Length() == 0)
        KillBlock(b);
    } else
      ownAdjBlocks.PushBack(b);
  }
}

void GoUctBoard::KillBlock(const Block *block) {
  SgBlackWhite c = block->m_color;
  SgBlackWhite opp = SgOppBW(c);
  GoArray<int, GO_MAXPOINT> &nuNeighbors = m_nuNeighbors[c];
  for (Block::StoneIterator it(block->m_stones); it; ++it) {
    GoPoint p = *it;
    AddLibToAdjBlocks(p, opp);
    m_color[p] = SG_EMPTY;
    ++m_nuNeighborsEmpty[p - GO_NORTH_SOUTH];
    ++m_nuNeighborsEmpty[p - GO_WEST_EAST];
    ++m_nuNeighborsEmpty[p + GO_WEST_EAST];
    ++m_nuNeighborsEmpty[p + GO_NORTH_SOUTH];
    --nuNeighbors[p - GO_NORTH_SOUTH];
    --nuNeighbors[p - GO_WEST_EAST];
    --nuNeighbors[p + GO_WEST_EAST];
    --nuNeighbors[p + GO_NORTH_SOUTH];
    m_capturedStones.PushBack(p);
    m_block[p] = 0;
  }
  int nuStones = block->m_stones.Length();
  m_prisoners[c] += nuStones;
  if (nuStones == 1)
    m_koPoint = block->m_anchor;
}

void GoUctBoard::Play(GoPoint p) {
  DBG_ASSERT(p >= 0);
  DBG_ASSERT(p == GO_PASS || (IsValidPoint(p) && IsEmpty(p)));
  CheckConsistency();
  m_koPoint = GO_NULLPOINT;
  m_capturedStones.Clear();
  SgBlackWhite opp = SgOppBW(m_toPlay);
  if (p != GO_PASS) {
    AddStone(p, m_toPlay);
    GoArrayList<Block *, 4> adjBlocks;
    if (NumNeighbors(p, SG_BLACK) > 0 || NumNeighbors(p, SG_WHITE) > 0)
      RemoveLibertyAndKill(p, opp, adjBlocks);

    UpdateBlocksAfterAddStone(p, m_toPlay, adjBlocks);
    if (m_koPoint != GO_NULLPOINT)
      if (NumStones(p) > 1 || NumLiberties(p) > 1)
        m_koPoint = GO_NULLPOINT;
    DBG_ASSERT(HasLiberties(p));
  }
  m_secondLastMove = m_lastMove;
  m_lastMove = p;
  m_toPlay = opp;
  CheckConsistency();
}

