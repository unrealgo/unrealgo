
#ifndef GOUCT_BOARD_H
#define GOUCT_BOARD_H

#include <bitset>
#include <cstring>
#include <stdint.h>
#include <boost/static_assert.hpp>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoPlayerMove.h"
#include "lib/Array.h"
#include "lib/SgArrayList.h"
#include "board/GoBoardConst.h"
#include "board/GoBoardColor.h"
#include "SgMarker.h"
#include "board/GoBWArray.h"
#include "board/GoNbIterator.h"
#include "board/GoPoint.h"
#include "board/GoPointArray.h"
#include "board/GoPointIterator.h"

class GoUctBoard;
typedef GoNb4Iterator<GoUctBoard> GoUctNbIterator;

class GoUctBoard {
 public:
  explicit GoUctBoard(const GoBoard &bd);
  ~GoUctBoard();
  const GoBoardConst &BoardConst() const;
  void Init(const GoBoard &bd);
  GoGrid Size() const;
  bool Occupied(GoPoint p) const;
  bool IsEmpty(GoPoint p) const;
  bool IsBorder(GoPoint p) const;
  bool IsColor(GoPoint p, int c) const;
  SgBoardColor GetColor(GoPoint p) const;
  SgBlackWhite GetStone(GoPoint p) const;
  SgBlackWhite ToPlay() const;
  SgBlackWhite Opponent() const;
  GoGrid Line(GoPoint p) const;
  GoGrid Pos(GoPoint p) const;
  int Up(GoPoint p) const;
  int Left(GoPoint p) const;
  int Right(GoPoint p) const;
  int Side(GoPoint p, int index) const;
  bool IsSuicide(GoPoint p, SgBlackWhite toPlay) const;
  bool IsValidPoint(GoPoint p) const;
  bool HasEmptyNeighbors(GoPoint p) const;
  int NumEmptyNeighbors(GoPoint p) const;
  int Num8EmptyNeighbors(GoPoint p) const;
  bool HasNeighbors(GoPoint p, SgBlackWhite c) const;
  int NumNeighbors(GoPoint p, SgBlackWhite c) const;
  int Num8Neighbors(GoPoint p, SgBlackWhite c) const;
  bool HasDiagonals(GoPoint p, SgBoardColor c) const;
  int NumDiagonals(GoPoint p, SgBoardColor c) const;
  int NumEmptyDiagonals(GoPoint p) const;
  bool HasNeighborsOrDiags(GoPoint p, SgBlackWhite c) const;
  bool InCorner(GoPoint p) const;
  bool OnEdge(GoPoint p) const;
  bool InCenter(GoPoint p) const;
  int FirstBoardPoint() const;
  int LastBoardPoint() const;
  void Play(GoPoint p);
  bool IsLegal(int p, SgBlackWhite player) const;
  bool IsLegal(int p) const;
  bool IsSuicide(GoPoint p) const;
  bool CapturingMove() const;
  const GoPointList &CapturedStones() const;
  int NuCapturedStones() const;
  int NumPrisoners(SgBlackWhite color) const;
  GoPoint GetLastMove() const;
  GoPoint Get2ndLastMove() const;
  int NumStones(GoPoint p) const;
  bool IsSingleStone(GoPoint p) const;
  bool AreInSameBlock(GoPoint stone1, GoPoint stone2) const;
  GoPoint Anchor(GoPoint p) const;
  bool IsInBlock(GoPoint p, GoPoint anchor) const;
  bool IsLibertyOfBlock(GoPoint p, GoPoint anchor) const;
  int AdjacentBlocks(GoPoint p, int maxLib, GoPoint anchors[],
                     int maxAnchors) const;
  void NeighborBlocks(GoPoint p, SgBlackWhite c, GoPoint anchors[]) const;
  void NeighborBlocks(GoPoint p, SgBlackWhite c, int maxLib,
                      GoPoint anchors[]) const;
  GoPoint TheLiberty(GoPoint blockInAtari) const;
  int NumLiberties(GoPoint p) const;
  bool AtMostNumLibs(GoPoint block, int n) const;
  bool AtLeastNumLibs(GoPoint block, int n) const;
  bool InAtari(GoPoint p) const;
  bool OccupiedInAtari(GoPoint p) const;
  bool CanCapture(GoPoint p, SgBlackWhite c) const;
  void CheckConsistency() const;

 private:

  struct Block {
   public:

    static const int MAX_LIBERTIES = (GO_MAX_SIZE / 3) * 2 * GO_MAX_SIZE;
    typedef GoArrayList<GoPoint, MAX_LIBERTIES> LibertyList;
    typedef LibertyList::Iterator LibertyIterator;
    typedef GoPointList::Iterator StoneIterator;
    GoPoint m_anchor;
    SgBlackWhite m_color;
    LibertyList m_liberties;
    GoPointList m_stones;

    void InitSingleStoneBlock(SgBlackWhite c, GoPoint anchor) {
      DBG_ASSERT_BW(c);
      m_color = c;
      m_anchor = anchor;
      m_stones.SetTo(anchor);
      m_liberties.Clear();
    }

    void InitNewBlock(SgBlackWhite c, GoPoint anchor) {
      DBG_ASSERT_BW(c);
      m_color = c;
      m_anchor = anchor;
      m_stones.Clear();
      m_liberties.Clear();
    }
  };
  GoPoint m_lastMove;
  GoPoint m_secondLastMove;
  GoPoint m_koPoint;
  SgBlackWhite m_toPlay;
  GoArray<Block *, GO_MAXPOINT> m_block;
  SgBWArray<int> m_prisoners;
  GoArray<int, GO_MAXPOINT> m_color;
  GoArray<int, GO_MAXPOINT> m_nuNeighborsEmpty;
  SgBWArray<GoArray<int, GO_MAXPOINT> > m_nuNeighbors;
  GoBoardConst m_const;
  GoGrid m_size;
  GoPointArray<Block> m_blockArray;
  mutable SgMarker m_marker;
  SgMarker m_marker2;
  GoPointList m_capturedStones;
  GoArray<bool, GO_MAXPOINT> m_isBorder;
  GoUctBoard(const GoUctBoard &);
  GoUctBoard &operator=(const GoUctBoard &);
  void AddLibToAdjBlocks(GoPoint p, SgBlackWhite c);
  void AddStoneToBlock(GoPoint p, Block *block);
  void CreateSingleStoneBlock(GoPoint p, SgBlackWhite c);
  void InitSize(const GoBoard &bd);
  bool IsAdjacentTo(GoPoint p, const Block *block) const;
  void MergeBlocks(GoPoint p, const GoArrayList<Block *, 4> &adjBlocks);
  void RemoveLibertyAndKill(GoPoint p, SgBlackWhite opp,
                            GoArrayList<Block *, 4> &ownAdjBlocks);
  void UpdateBlocksAfterAddStone(GoPoint p, SgBlackWhite c,
                                 const GoArrayList<Block *, 4> &adjBlocks);
  void CheckConsistencyBlock(GoPoint p) const;
  bool FullBoardRepetition() const;
  void AddStone(GoPoint p, SgBlackWhite c);
  void KillBlock(const Block *block);
  bool HasLiberties(GoPoint p) const;

 public:
  friend class LibertyIterator;
  friend class StoneIterator;

  class Iterator
      : public GoPointRangeIterator {
   public:
    explicit Iterator(const GoUctBoard &bd);
  };

  class LibertyIterator {
   public:
    LibertyIterator(const GoUctBoard &bd, GoPoint p);
    void operator++();
    GoPoint operator*() const;
    explicit operator bool() const;

   private:
    GoUctBoard::Block::LibertyList::Iterator m_it;
    const GoUctBoard &m_board;
    operator int() const;
    LibertyIterator(const LibertyIterator &);
    LibertyIterator &operator=(const LibertyIterator &);
  };

  class StoneIterator {
   public:
    StoneIterator(const GoUctBoard &bd, GoPoint p);
    void operator++();
    GoPoint operator*() const;
    explicit operator bool() const;

   private:
    GoUctBoard::Block::StoneIterator m_it;
    const GoUctBoard &m_board;
    operator int() const;
    StoneIterator(const StoneIterator &);
    StoneIterator &operator=(const StoneIterator &);
  };
};

inline std::ostream &operator<<(std::ostream &out, const GoUctBoard &bd) {
  return GoWriteBoard(out, bd);
}

inline GoUctBoard::Iterator::Iterator(const GoUctBoard &bd)
    : GoPointRangeIterator(bd.BoardConst().BoardIterAddress(),
                           bd.BoardConst().BoardIterEnd()) {}

inline GoUctBoard::LibertyIterator::LibertyIterator(const GoUctBoard &bd,
                                                    GoPoint p)
    : m_it(bd.m_block[p]->m_liberties),
      m_board(bd) {
  DBG_ASSERT(m_board.Occupied(p));
}

inline void GoUctBoard::LibertyIterator::operator++() {
  ++m_it;
}

inline GoPoint GoUctBoard::LibertyIterator::operator*() const {
  return *m_it;
}

inline GoUctBoard::LibertyIterator::operator bool() const {
  return m_it;
}

inline GoUctBoard::StoneIterator::StoneIterator(const GoUctBoard &bd,
                                                GoPoint p)
    : m_it(bd.m_block[p]->m_stones),
      m_board(bd) {
  DBG_ASSERT(m_board.Occupied(p));
}

inline void GoUctBoard::StoneIterator::operator++() {
  ++m_it;
}

inline GoPoint GoUctBoard::StoneIterator::operator*() const {
  return *m_it;
}

inline GoUctBoard::StoneIterator::operator bool() const {
  return m_it;
}

inline int GoUctBoard::AdjacentBlocks(GoPoint point, int maxLib,
                                      GoPoint anchors[], int maxAnchors) const {
  SG_DEBUG_ONLY(maxAnchors);
  DBG_ASSERT(Occupied(point));
  const SgBlackWhite other = SgOppBW(GetStone(point));
  int n = 0;
  SgReserveMarker reserve(m_marker);
  SuppressUnused(reserve);
  m_marker.Clear();
  for (StoneIterator it(*this, point); it; ++it) {
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

inline GoPoint GoUctBoard::Anchor(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return m_block[p]->m_anchor;
}

inline bool GoUctBoard::AreInSameBlock(GoPoint p1, GoPoint p2) const {
  return Occupied(p1) && Occupied(p2) && Anchor(p1) == Anchor(p2);
}

inline bool GoUctBoard::AtLeastNumLibs(GoPoint block, int n) const {
  return NumLiberties(block) >= n;
}

inline bool GoUctBoard::AtMostNumLibs(GoPoint block, int n) const {
  return NumLiberties(block) <= n;
}

inline const GoPointList &GoUctBoard::CapturedStones() const {
  return m_capturedStones;
}

inline bool GoUctBoard::CapturingMove() const {
  return !m_capturedStones.IsEmpty();
}

inline int GoUctBoard::FirstBoardPoint() const {
  return m_const.FirstBoardPoint();
}

inline const GoBoardConst &GoUctBoard::BoardConst() const {
  return m_const;
}

inline GoPoint GoUctBoard::Get2ndLastMove() const {
  return m_secondLastMove;
}

inline SgBoardColor GoUctBoard::GetColor(GoPoint p) const {
  return m_color[p];
}

inline GoPoint GoUctBoard::GetLastMove() const {
  return m_lastMove;
}

inline SgBlackWhite GoUctBoard::GetStone(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return m_color[p];
}

inline bool GoUctBoard::HasDiagonals(GoPoint p, SgBoardColor c) const {
  return (IsColor(p - GO_NORTH_SOUTH - GO_WEST_EAST, c)
      || IsColor(p - GO_NORTH_SOUTH + GO_WEST_EAST, c)
      || IsColor(p + GO_NORTH_SOUTH - GO_WEST_EAST, c)
      || IsColor(p + GO_NORTH_SOUTH + GO_WEST_EAST, c));
}

inline bool GoUctBoard::HasEmptyNeighbors(GoPoint p) const {
  return m_nuNeighborsEmpty[p] != 0;
}

inline bool GoUctBoard::HasLiberties(GoPoint p) const {
  return NumLiberties(p) > 0;
}

inline bool GoUctBoard::HasNeighbors(GoPoint p, SgBlackWhite c) const {
  return (m_nuNeighbors[c][p] > 0);
}

inline bool GoUctBoard::HasNeighborsOrDiags(GoPoint p, SgBlackWhite c) const {
  return HasNeighbors(p, c) || HasDiagonals(p, c);
}

inline bool GoUctBoard::InAtari(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return AtMostNumLibs(p, 1);
}

inline bool GoUctBoard::IsInBlock(GoPoint p, GoPoint anchor) const {
  DBG_ASSERT(Occupied(anchor));
  const Block *b = m_block[p];
  return (b != 0 && b->m_anchor == anchor);
}

inline bool GoUctBoard::IsLibertyOfBlock(GoPoint p, GoPoint anchor) const {
  DBG_ASSERT(IsEmpty(p));
  DBG_ASSERT(Occupied(anchor));
  DBG_ASSERT(Anchor(anchor) == anchor);
  const Block *b = m_block[anchor];
  if (m_nuNeighbors[b->m_color][p] == 0)
    return false;
  return (m_block[p - GO_NORTH_SOUTH] == b
      || m_block[p - GO_WEST_EAST] == b
      || m_block[p + GO_WEST_EAST] == b
      || m_block[p + GO_NORTH_SOUTH] == b);
}

inline bool GoUctBoard::CanCapture(GoPoint p, SgBlackWhite c) const {
  SgBlackWhite opp = SgOppBW(c);
  for (GoUctNbIterator nb(*this, p); nb; ++nb)
    if (IsColor(*nb, opp) && AtMostNumLibs(*nb, 1))
      return true;
  return false;
}

inline bool GoUctBoard::IsSuicide(GoPoint p, SgBlackWhite toPlay) const {
  if (HasEmptyNeighbors(p))
    return false;
  SgBlackWhite opp = SgOppBW(toPlay);
  for (GoUctNbIterator it(*this, p); it; ++it) {
    SgEmptyBlackWhite c = GetColor(*it);
    if (c == toPlay && NumLiberties(*it) > 1)
      return false;
    if (c == opp && NumLiberties(*it) == 1)
      return false;
  }
  return true;
}

inline bool GoUctBoard::IsBorder(GoPoint p) const {
  DBG_ASSERT(p != GO_PASS);
  return m_isBorder[p];
}

inline bool GoUctBoard::IsColor(GoPoint p, int c) const {
  DBG_ASSERT(p != GO_PASS);
  DBG_ASSERT_EBW(c);
  return m_color[p] == c;
}

inline bool GoUctBoard::IsEmpty(GoPoint p) const {
  DBG_ASSERT(p != GO_PASS);
  return m_color[p] == SG_EMPTY;
}

inline bool GoUctBoard::IsLegal(int p, SgBlackWhite player) const {
  DBG_ASSERT_BW(player);
  if (p == GO_PASS)
    return true;
  DBG_ASSERT(GoPointUtil::InBoardRange(p));
  if (!IsEmpty(p))
    return false;
  if (IsSuicide(p, player))
    return false;
  if (p == m_koPoint && m_toPlay == player)
    return false;
  return true;
}

inline bool GoUctBoard::IsLegal(int p) const {
  return IsLegal(p, ToPlay());
}

inline bool GoUctBoard::IsSingleStone(GoPoint p) const {
  return (Occupied(p) && NumNeighbors(p, GetColor(p)) == 0);
}

inline bool GoUctBoard::IsSuicide(GoPoint p) const {
  return IsSuicide(p, ToPlay());
}

inline bool GoUctBoard::IsValidPoint(GoPoint p) const {
  return GoPointUtil::InBoardRange(p) && !IsBorder(p);
}

inline int GoUctBoard::LastBoardPoint() const {
  return m_const.LastBoardPoint();
}

inline int GoUctBoard::Left(GoPoint p) const {
  return m_const.Left(p);
}

inline GoGrid GoUctBoard::Line(GoPoint p) const {
  return m_const.Line(p);
}

inline void GoUctBoard::NeighborBlocks(GoPoint p, SgBlackWhite c, int maxLib,
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

inline int GoUctBoard::Num8Neighbors(GoPoint p, SgBlackWhite c) const {
  return NumNeighbors(p, c) + NumDiagonals(p, c);
}

inline int GoUctBoard::Num8EmptyNeighbors(GoPoint p) const {
  return NumEmptyNeighbors(p) + NumEmptyDiagonals(p);
}

inline int GoUctBoard::NuCapturedStones() const {
  return m_capturedStones.Length();
}

inline int GoUctBoard::NumDiagonals(GoPoint p, SgBoardColor c) const {
  int n = 0;
  if (IsColor(p - GO_NORTH_SOUTH - GO_WEST_EAST, c))
    ++n;
  if (IsColor(p - GO_NORTH_SOUTH + GO_WEST_EAST, c))
    ++n;
  if (IsColor(p + GO_NORTH_SOUTH - GO_WEST_EAST, c))
    ++n;
  if (IsColor(p + GO_NORTH_SOUTH + GO_WEST_EAST, c))
    ++n;
  return n;
}

inline int GoUctBoard::NumEmptyDiagonals(GoPoint p) const {
  return NumDiagonals(p, SG_EMPTY);
}

inline int GoUctBoard::NumEmptyNeighbors(GoPoint p) const {
  return m_nuNeighborsEmpty[p];
}

inline int GoUctBoard::NumLiberties(GoPoint p) const {
  DBG_ASSERT(IsValidPoint(p));
  DBG_ASSERT(Occupied(p));
  return m_block[p]->m_liberties.Length();
}

inline int GoUctBoard::NumNeighbors(GoPoint p, SgBlackWhite c) const {
  return m_nuNeighbors[c][p];
}

inline int GoUctBoard::NumPrisoners(SgBlackWhite color) const {
  return m_prisoners[color];
}

inline int GoUctBoard::NumStones(GoPoint block) const {
  DBG_ASSERT(Occupied(block));
  return m_block[block]->m_stones.Length();
}

inline bool GoUctBoard::Occupied(GoPoint p) const {
  return (m_block[p] != 0);
}

inline bool GoUctBoard::OccupiedInAtari(GoPoint p) const {
  const Block *b = m_block[p];
  return (b != 0 && b->m_liberties.Length() <= 1);
}

inline SgBlackWhite GoUctBoard::Opponent() const {
  return SgOppBW(m_toPlay);
}

inline GoGrid GoUctBoard::Pos(GoPoint p) const {
  return m_const.Pos(p);
}

inline int GoUctBoard::Right(GoPoint p) const {
  return m_const.Right(p);
}

inline int GoUctBoard::Side(GoPoint p, int index) const {
  return m_const.Side(p, index);
}

inline GoGrid GoUctBoard::Size() const {
  return m_size;
}

inline GoPoint GoUctBoard::TheLiberty(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  DBG_ASSERT(NumLiberties(p) == 1);
  return m_block[p]->m_liberties[0];
}

inline SgBlackWhite GoUctBoard::ToPlay() const {
  return m_toPlay;
}

inline int GoUctBoard::Up(GoPoint p) const {
  return m_const.Up(p);
}

#endif

