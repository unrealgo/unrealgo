
#ifndef GO_BOARD_H
#define GO_BOARD_H

#include <bitset>
#include <cstring>
#include <stdint.h>
#include <boost/static_assert.hpp>
#include "GoPlayerMove.h"
#include "GoRules.h"
#include "GoSetup.h"
#include "lib/Array.h"
#include "lib/SgArrayList.h"
#include "board/GoBoardConst.h"
#include "board/GoBoardColor.h"
#include "SgMarker.h"
#include "board/GoBWArray.h"
#include "board/GoBWSet.h"
#include "lib/SgHash.h"
#include "board/GoNbIterator.h"
#include "board/GoPoint.h"
#include "board/GoPointArray.h"
#include "board/GoPointIterator.h"
#include "board/GoPointSet.h"

const int GO_DEFAULT_SIZE = (GO_MAX_SIZE >= 19 ? 19 : GO_MAX_SIZE);
const int GO_MAX_NUM_MOVES = (4 * GO_MAX_SIZE * GO_MAX_SIZE);
enum GoMoveInfoFlag {
  GO_MOVEFLAG_REPETITION,
  GO_MOVEFLAG_SUICIDE,
  GO_MOVEFLAG_CAPTURING,
  GO_MOVEFLAG_ILLEGAL,
  _GO_NU_MOVEFLAG
};
typedef std::bitset<_GO_NU_MOVEFLAG> GoMoveInfo;
typedef GoArrayList<GoPoint, GO_MAX_ONBOARD + 1> GoPointList;
typedef GoArrayList<GoPoint, GO_MAX_NUM_MOVES> GoSequence;
class GoBoard {
 public:
  static const int MAX_KOLEVEL = 3;
  mutable SgMarker m_userMarker;
  explicit GoBoard(int size = GO_DEFAULT_SIZE,
                   const GoSetup& setup = GoSetup(),
                   const GoRules& rules = GoRules());
  ~GoBoard();
  const GoBoardConst& BoardConst() const;
  uint64_t CountPlay() const;
  void Reset();
  void Init(int size, const GoSetup& setup = GoSetup());
  void Init(int size, const GoRules& rules, const GoSetup& setup = GoSetup());
  GoRules& Rules();
  const GoRules& Rules() const;
  GoGrid Size() const;
  bool StackOverflowLikely() const;
  bool IsFirst(GoPoint p) const;
  bool IsNewPosition() const;
  bool Occupied(GoPoint p) const;
  bool IsEmpty(GoPoint p) const;
  bool IsBorder(GoPoint p) const;
  bool IsColor(GoPoint p, int c) const;
  SgBoardColor GetColor(GoPoint p) const;
  SgBlackWhite GetStone(GoPoint p) const;
  SgBlackWhite ToPlay() const;
  SgBlackWhite Opponent() const;
  void SetToPlay(SgBlackWhite player);
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
  GoPointSet Occupied() const;
  const GoPointSet& All(SgBlackWhite color) const;
  const GoPointSet& AllEmpty() const;
  const GoPointSet& AllPoints() const;
  const GoPointSet& Corners() const;
  const GoPointSet& Edges() const;
  const GoPointSet& Centers() const;
  const GoPointSet& SideExtensions() const;
  const GoPointSet& LineSet(GoGrid line) const;
  bool InCorner(GoPoint p) const;
  bool OnEdge(GoPoint p) const;
  bool InCenter(GoPoint p) const;
  int FirstBoardPoint() const;
  int LastBoardPoint() const;
  bool LastMoveInfo(GoMoveInfoFlag flag) const;
  GoMoveInfo GetLastMoveInfo() const;
  void AllowKoRepetition(bool allowKo);
  void AllowAnyRepetition(bool allowAny);
  void SetKoModifiesHash(bool modify);
  bool KoRepetitionAllowed() const;
  bool AnyRepetitionAllowed() const;
  bool KoModifiesHash() const;
  void Play(GoPoint p, SgBlackWhite player);
  void Play(GoPoint p);
  void Play(GoPlayerMove move);
  void Undo();
  bool CanUndo() const;
  bool IsLegal(int p, SgBlackWhite player) const;
  bool IsLegal(int p) const;
  bool IsSuicide(GoPoint p) const;
  bool CapturingMove() const;
  const GoPointList& CapturedStones() const;
  int NuCapturedStones() const;
  int NumPrisoners(SgBlackWhite color) const;
  const GoSetup& Setup() const;
  int MoveNumber() const;
  GoPlayerMove Move(int i) const;
  GoPoint GetLastMove() const;
  GoPoint Get2ndLastMove() const;
  GoPoint KoPoint() const;
  const SgHashCode& GetHashCode() const;
  SgHashCode GetHashCodeInclToPlay() const;
  int NumStones(GoPoint block) const;
  bool IsSingleStone(GoPoint p) const;
  bool AreInSameBlock(GoPoint p1, GoPoint p2) const;
  GoPoint Anchor(GoPoint p) const;
  bool IsInBlock(GoPoint p, GoPoint anchor) const;
  bool IsLibertyOfBlock(GoPoint p, GoPoint anchor) const;
  int AdjacentBlocks(GoPoint p, int maxLib, GoPoint anchors[],
                     int maxAnchors) const;
  void NeighborBlocks(GoPoint p, SgBlackWhite c, GoPoint anchors[]) const;
  void NeighborBlocks(GoPoint p, SgBlackWhite c, int maxLib,
                      GoPoint anchors[]) const;
  const SgBWArray<int>& TotalNumStones() const;
  int TotalNumStones(SgBlackWhite color) const;
  int TotalNumEmpty() const;
  GoPoint TheLiberty(GoPoint p) const;
  int NumLiberties(GoPoint p) const;
  bool AtMostNumLibs(GoPoint block, int n) const;
  bool AtLeastNumLibs(GoPoint block, int n) const;
  bool InAtari(GoPoint p) const;
  bool OccupiedInAtari(GoPoint p) const;
  bool CanCapture(GoPoint p, SgBlackWhite c) const;
  SgEmptyBlackWhite KoColor() const;
  int KoLevel() const;
  SgEmptyBlackWhite KoLoser() const;
  void SetKoLoser(SgEmptyBlackWhite color);
  void CheckConsistency() const;
  void TakeSnapshot();
  void RestoreSnapshot();

 private:
  class Block {
   public:
    static const int MAX_LIBERTIES = (GO_MAX_SIZE / 3) * 2 * GO_MAX_SIZE;
    typedef GoArrayList<GoPoint, MAX_LIBERTIES> LibertyList;
    typedef LibertyList::Iterator LibertyIterator;
    typedef GoPointList::Iterator StoneIterator;

    GoPoint Anchor() const {
      return m_anchor;
    }

    void UpdateAnchor(GoPoint p) {
      if (p < m_anchor)
        m_anchor = p;
    }

    void AppendLiberty(GoPoint p) {
      m_liberties.PushBack(p);
    }

    void AppendStone(GoPoint p) {
      m_stones.PushBack(p);
    }

    SgBlackWhite Color() const {
      return m_color;
    }

    void ExcludeLiberty(GoPoint p) {
      m_liberties.Exclude(p);
    }

    void Init(SgBlackWhite c, GoPoint anchor) {
      DBG_ASSERT_BW(c);
      m_color = c;
      m_anchor = anchor;
      m_stones.SetTo(anchor);
      m_liberties.Clear();
    }

    void Init(SgBlackWhite c, GoPoint anchor, GoPointList stones,
              LibertyList liberties) {
      DBG_ASSERT_BW(c);
      DBG_ASSERT(stones.Contains(anchor));
      m_color = c;
      m_anchor = anchor;
      m_stones = stones;
      m_liberties = liberties;
    }

    const LibertyList& Liberties() const {
      return m_liberties;
    }

    int NumLiberties() const {
      return m_liberties.Length();
    }

    int NumStones() const {
      return m_stones.Length();
    }

    void PopStone() {
      m_stones.PopBack();
    }

    void SetAnchor(GoPoint p) {
      m_anchor = p;
    }

    const GoPointList& Stones() const {
      return m_stones;
    }

   private:
    GoPoint m_anchor;
    SgBlackWhite m_color;
    LibertyList m_liberties;
    GoPointList m_stones;
  };
  class HashCode {
   public:
    void Clear();
    const SgHashCode& Get() const;
    SgHashCode GetInclToPlay(SgBlackWhite toPlay) const;
    void XorCaptured(int moveNumber, GoPoint firstCapturedStone);
    void XorStone(GoPoint p, SgBlackWhite c);
    void XorWinKo(int level, SgBlackWhite c);

   private:
    static const int START_INDEX_TOPLAY = 1;
    static const int END_INDEX_TOPLAY = 2;
    static const int START_INDEX_STONES = 3;
    static const int END_INDEX_STONES = 2 * GO_MAXPOINT;
    static const int START_INDEX_WINKO = 2 * GO_MAXPOINT + 1;
    static const int END_INDEX_WINKO = 2 * GO_MAXPOINT + GO_MAX_SIZE + 1;
    static const int START_INDEX_CAPTURES = 2 * GO_MAXPOINT + GO_MAX_SIZE + 2;
    static const int END_INDEX_CAPTURES = 3 * GO_MAXPOINT + 63;
    BOOST_STATIC_ASSERT(START_INDEX_TOPLAY >= 0);
    BOOST_STATIC_ASSERT(END_INDEX_TOPLAY > START_INDEX_TOPLAY);
    BOOST_STATIC_ASSERT(START_INDEX_STONES > END_INDEX_TOPLAY);
    BOOST_STATIC_ASSERT(END_INDEX_STONES > START_INDEX_STONES);
    BOOST_STATIC_ASSERT(END_INDEX_WINKO > START_INDEX_WINKO);
    BOOST_STATIC_ASSERT(START_INDEX_CAPTURES > END_INDEX_WINKO);
    BOOST_STATIC_ASSERT(END_INDEX_CAPTURES > START_INDEX_CAPTURES);
    BOOST_STATIC_ASSERT(START_INDEX_WINKO + MAX_KOLEVEL * 3 - 1
                            <= END_INDEX_WINKO);
    BOOST_STATIC_ASSERT(END_INDEX_CAPTURES
                            < SgHashZobristTable::MAX_HASH_INDEX);
    SgHashCode m_hash;
  };
  struct StackEntry {
    SgBlackWhite m_color;
    GoPoint m_point;
    bool m_isFirst;
    bool m_isNewPosition;
    Block* m_stoneAddedTo;
    GoPoint m_oldAnchor;
    GoArrayList<GoPoint, 4> m_newLibs;
    GoArrayList<Block*, 4> m_merged;
    SgBlackWhite m_toPlay;
    HashCode m_hash;
    GoPoint m_koPoint;
    int m_koLevel;
    SgEmptyBlackWhite m_koColor;
    SgEmptyBlackWhite m_koLoser;
    bool m_koModifiesHash;
    Block* m_suicide;
    GoArrayList<Block*, 4> m_killed;
  };
  struct State {
    GoPoint m_koPoint;
    SgBlackWhite m_toPlay;
    HashCode m_hash;
    GoBWSet m_all;
    GoPointSet m_empty;
    GoArray<Block*, GO_MAXPOINT> m_block;
    SgBWArray<int> m_prisoners;
    SgBWArray<int> m_numStones;
    int m_koLevel;
    GoArray<int, GO_MAXPOINT> m_color;
    GoArray<int, GO_MAXPOINT> m_nuNeighborsEmpty;
    SgBWArray<GoArray<int, GO_MAXPOINT> > m_nuNeighbors;
    GoArray<bool, GO_MAXPOINT> m_isFirst;
    bool m_isNewPosition;
  };
  struct Snapshot {
    int m_moveNumber;
    int m_blockListSize;
    State m_state;
    GoPointArray<Block> m_blockArray;
  };
  State m_state;
  std::unique_ptr<Snapshot> m_snapshot;
  uint64_t m_countPlay;
  GoBoardConst m_const;
  GoGrid m_size;
  GoRules m_rules;
  GoSetup m_setup;
  GoMoveInfo m_moveInfo;
  GoArrayList<Block, GO_MAX_NUM_MOVES>* m_blockList;
  mutable SgMarker m_marker;
  GoPointList m_capturedStones;
  bool m_allowAnyRepetition;
  bool m_allowKoRepetition;
  bool m_koModifiesHash;
  SgEmptyBlackWhite m_koColor;
  SgEmptyBlackWhite m_koLoser;
  GoArray<bool, GO_MAXPOINT> m_isBorder;
  GoArrayList<StackEntry, GO_MAX_NUM_MOVES>* m_moves;
  static bool IsPass(GoPoint p);
  GoBoard(const GoBoard&);
  GoBoard& operator=(const GoBoard&);
  bool CheckKo(SgBlackWhite player);
  void AddLibToAdjBlocks(GoPoint p);
  void AddLibToAdjBlocks(GoPoint p, SgBlackWhite c);
  void AddStoneToBlock(GoPoint p, SgBlackWhite c, Block* block,
                       StackEntry& entry);
  Block& CreateNewBlock();
  void CreateSingleStoneBlock(GoPoint p, SgBlackWhite c);
  GoArrayList<Block*, 4> GetAdjacentBlocks(GoPoint p) const;
  GoArrayList<Block*, 4> GetAdjacentBlocks(GoPoint p, SgBlackWhite c) const;
  void InitBlock(GoBoard::Block& block, SgBlackWhite c, GoPoint anchor);
  bool IsAdjacentTo(GoPoint p, const Block* block) const;
  void MergeBlocks(GoPoint p, SgBlackWhite c,
                   const GoArrayList<Block*, 4>& adjBlocks);
  void RemoveLibAndKill(GoPoint p, SgBlackWhite opp, StackEntry& entry);
  void RemoveLibFromAdjBlocks(GoPoint p, SgBlackWhite c);
  void RestoreKill(Block* block, SgBlackWhite c);
  void UpdateBlocksAfterAddStone(GoPoint p, SgBlackWhite c,
                                 StackEntry& entry);
  void UpdateBlocksAfterUndo(const StackEntry& entry);
  void CheckConsistencyBlock(GoPoint p) const;
  bool FullBoardRepetition() const;
  bool CheckSuicide(GoPoint p, StackEntry& entry);
  void AddStone(GoPoint p, SgBlackWhite c);
  void RemoveStone(GoPoint p);
  void KillBlock(const Block* block);
  bool HasLiberties(GoPoint p) const;
  void RestoreState(const StackEntry& entry);
  void SaveState(StackEntry& entry);
  friend class LibertyCopyIterator;
  friend class LibertyIterator;
  friend class StoneIterator;

 public:
  class StoneIterator {
   public:
    StoneIterator(const GoBoard& bd, GoPoint p);
    void operator++();
    GoPoint operator*() const;
    explicit operator bool() const;

   private:

    GoBoard::Block::StoneIterator m_it;
    const GoBoard& m_board;

#ifndef NDEBUG
    uint64_t m_countPlay;
#endif

    operator int() const;
    StoneIterator(const StoneIterator&);
    StoneIterator& operator=(const StoneIterator&);
  };
  class Iterator
      : public GoPointRangeIterator {
   public:
    explicit Iterator(const GoBoard& bd);
  };
  class LibertyIterator {
   public:
    LibertyIterator(const GoBoard& bd, GoPoint p);
    void operator++();
    GoPoint operator*() const;
    operator bool() const;

   private:
    Block::LibertyList::Iterator m_it;
    const GoBoard& m_board;

#ifndef NDEBUG
    uint64_t m_countPlay;
#endif

    operator int() const;
    LibertyIterator(const LibertyIterator&);
    LibertyIterator& operator=(const LibertyIterator&);
  };
  class LibertyCopyIterator {
   public:
    LibertyCopyIterator(const GoBoard& bd, GoPoint p);
    void operator++();
    int operator*() const;
    operator bool() const;

   private:

    Block::LibertyList m_liberties;
    Block::LibertyList::Iterator m_it;
    const GoBoard& m_board;

#ifndef NDEBUG
    SgHashCode m_oldHash;
#endif

    operator int() const;
    LibertyCopyIterator(const LibertyCopyIterator&);
    LibertyCopyIterator& operator=(const LibertyCopyIterator&);
  };
};
template<class BOARD>
class GoNb4Iterator
    : public SgNbIterator {
 public:
  GoNb4Iterator(const BOARD& bd, GoPoint p);
};

template<class BOARD>
inline GoNb4Iterator<BOARD>::GoNb4Iterator(const BOARD& bd, GoPoint p)
    : SgNbIterator(bd.BoardConst(), p) {}

typedef GoNb4Iterator<GoBoard> GoNbIterator;

inline GoBoard::StoneIterator::StoneIterator(const GoBoard& bd, GoPoint p)
    : m_it(bd.m_state.m_block[p]->Stones()),
      m_board(bd) {
  DBG_ASSERT(m_board.Occupied(p));
#ifndef NDEBUG
  m_countPlay = m_board.CountPlay();
#endif
}

inline void GoBoard::StoneIterator::operator++() {
  ++m_it;
}

inline GoPoint GoBoard::StoneIterator::operator*() const {
  DBG_ASSERT(m_board.CountPlay() == m_countPlay);
  return *m_it;
}

inline GoBoard::StoneIterator::operator bool() const {
  return m_it;
}

inline GoBoard::Iterator::Iterator(const GoBoard& bd)
    : GoPointRangeIterator(bd.BoardConst().BoardIterAddress(),
                           bd.BoardConst().BoardIterEnd()) {}

inline GoBoard::LibertyIterator::LibertyIterator(const GoBoard& bd, GoPoint p)
    : m_it(bd.m_state.m_block[p]->Liberties()),
      m_board(bd) {
  DBG_ASSERT(m_board.Occupied(p));
#ifndef NDEBUG
  m_countPlay = m_board.CountPlay();
#endif
}

inline void GoBoard::LibertyIterator::operator++() {
  ++m_it;
}

inline GoPoint GoBoard::LibertyIterator::operator*() const {
  DBG_ASSERT(m_board.CountPlay() == m_countPlay);
  return *m_it;
}

inline GoBoard::LibertyIterator::operator bool() const {
  return m_it;
}

inline GoBoard::LibertyCopyIterator::LibertyCopyIterator(const GoBoard& bd,
                                                         GoPoint p)
    : m_liberties(bd.m_state.m_block[p]->Liberties()),
      m_it(m_liberties),
      m_board(bd) {
  DBG_ASSERT(m_board.Occupied(p));
#ifndef NDEBUG
  m_oldHash = m_board.GetHashCode();
#endif
}

inline void GoBoard::LibertyCopyIterator::operator++() {
  ++m_it;
}

inline int GoBoard::LibertyCopyIterator::operator*() const {
  DBG_ASSERT(m_board.GetHashCode() == m_oldHash);
  return *m_it;
}

inline GoBoard::LibertyCopyIterator::operator bool() const {
  return m_it;
}

inline void GoBoard::HashCode::Clear() {
  m_hash.Clear();
}

inline const SgHashCode& GoBoard::HashCode::Get() const {
  return m_hash;
}

inline SgHashCode GoBoard::HashCode::GetInclToPlay(SgBlackWhite toPlay) const {
  SgHashCode hash = m_hash;
  BOOST_STATIC_ASSERT(SG_BLACK == 0);
  BOOST_STATIC_ASSERT(SG_WHITE == 1);
  int index = toPlay + 1;
  DBG_ASSERTRANGE(index, START_INDEX_TOPLAY, END_INDEX_TOPLAY);
  SgHashUtil::XorZobrist(hash, index);
  return hash;
}

inline void GoBoard::HashCode::XorCaptured(int moveNumber,
                                           GoPoint firstCapturedStone) {
  int index = 2 * GO_MAXPOINT + moveNumber % 64 + firstCapturedStone;
  DBG_ASSERTRANGE(index, START_INDEX_CAPTURES, END_INDEX_CAPTURES);
  SgHashUtil::XorZobrist(m_hash, index);
}

inline void GoBoard::HashCode::XorStone(GoPoint p, SgBlackWhite c) {
  DBG_ASSERT_BOARDRANGE(p);
  DBG_ASSERT_BW(c);
  BOOST_STATIC_ASSERT(SG_BLACK == 0);
  BOOST_STATIC_ASSERT(SG_WHITE == 1);
  int index = p + c * GO_MAXPOINT;
  DBG_ASSERTRANGE(index, START_INDEX_STONES, END_INDEX_STONES);
  SgHashUtil::XorZobrist(m_hash, index);
}

inline void GoBoard::HashCode::XorWinKo(int level, SgBlackWhite c) {
  DBG_ASSERT(level > 0 && level <= MAX_KOLEVEL);
  DBG_ASSERT_BW(c);
  BOOST_STATIC_ASSERT(SG_BLACK == 0);
  BOOST_STATIC_ASSERT(SG_WHITE == 1);
  int index = level + MAX_KOLEVEL * c + 2 * GO_MAXPOINT;
  DBG_ASSERTRANGE(index, START_INDEX_WINKO, END_INDEX_WINKO);
  SgHashUtil::XorZobrist(m_hash, index);
}

inline const GoPointSet& GoBoard::All(SgBlackWhite color) const {
  return m_state.m_all[color];
}

inline const GoPointSet& GoBoard::AllEmpty() const {
  return m_state.m_empty;
}

inline void GoBoard::AllowAnyRepetition(bool allowAny) {
  m_allowAnyRepetition = allowAny;
}

inline void GoBoard::AllowKoRepetition(bool allowKo) {
  m_allowKoRepetition = allowKo;
}

inline const GoPointSet& GoBoard::AllPoints() const {
  return GoPointSet::AllPoints(Size());
}

inline GoPoint GoBoard::Anchor(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return m_state.m_block[p]->Anchor();
}

inline bool GoBoard::AnyRepetitionAllowed() const {
  return m_allowAnyRepetition;
}

inline bool GoBoard::AreInSameBlock(GoPoint p1, GoPoint p2) const {
  return Occupied(p1) && Occupied(p2) && Anchor(p1) == Anchor(p2);
}

inline bool GoBoard::AtLeastNumLibs(GoPoint block, int n) const {
  return NumLiberties(block) >= n;
}

inline bool GoBoard::AtMostNumLibs(GoPoint block, int n) const {
  return NumLiberties(block) <= n;
}

inline bool GoBoard::CanUndo() const {
  return (m_moves->Length() > 0);
}

inline const GoPointList& GoBoard::CapturedStones() const {
  return m_capturedStones;
}

inline bool GoBoard::CapturingMove() const {
  return !m_capturedStones.IsEmpty();
}

inline const GoPointSet& GoBoard::Centers() const {
  return m_const.Centers();
}

inline const GoPointSet& GoBoard::Corners() const {
  return m_const.Corners();
}

inline uint64_t GoBoard::CountPlay() const {
  return m_countPlay;
}

inline const GoPointSet& GoBoard::Edges() const {
  return m_const.Edges();
}

inline int GoBoard::FirstBoardPoint() const {
  return m_const.FirstBoardPoint();
}

inline const GoBoardConst& GoBoard::BoardConst() const {
  return m_const;
}

inline GoPoint GoBoard::Get2ndLastMove() const {
  int moveNumber = MoveNumber();
  if (moveNumber < 2)
    return GO_NULLMOVE;
  const StackEntry& entry1 = (*m_moves)[moveNumber - 1];
  const StackEntry& entry2 = (*m_moves)[moveNumber - 2];
  SgBlackWhite toPlay = ToPlay();
  if (entry1.m_color != SgOppBW(toPlay) || entry2.m_color != toPlay)
    return GO_NULLMOVE;
  return entry2.m_point;
}

inline SgBoardColor GoBoard::GetColor(GoPoint p) const {
  return m_state.m_color[p];
}

inline const SgHashCode& GoBoard::GetHashCode() const {
  return m_state.m_hash.Get();
}

inline SgHashCode GoBoard::GetHashCodeInclToPlay() const {
  return m_state.m_hash.GetInclToPlay(ToPlay());
}

inline GoPoint GoBoard::GetLastMove() const {
  int moveNumber = MoveNumber();
  if (moveNumber == 0)
    return GO_NULLMOVE;
  const StackEntry& entry = (*m_moves)[moveNumber - 1];
  if (entry.m_color != SgOppBW(ToPlay()))
    return GO_NULLMOVE;
  return entry.m_point;
}

inline GoMoveInfo GoBoard::GetLastMoveInfo() const {
  return m_moveInfo;
}

inline SgBlackWhite GoBoard::GetStone(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return m_state.m_color[p];
}

inline bool GoBoard::HasDiagonals(GoPoint p, SgBoardColor c) const {
  return (IsColor(p - GO_NORTH_SOUTH - GO_WEST_EAST, c)
      || IsColor(p - GO_NORTH_SOUTH + GO_WEST_EAST, c)
      || IsColor(p + GO_NORTH_SOUTH - GO_WEST_EAST, c)
      || IsColor(p + GO_NORTH_SOUTH + GO_WEST_EAST, c));
}

inline bool GoBoard::HasEmptyNeighbors(GoPoint p) const {
  return m_state.m_nuNeighborsEmpty[p] != 0;
}

inline bool GoBoard::HasLiberties(GoPoint p) const {
  return NumLiberties(p) > 0;
}

inline bool GoBoard::HasNeighbors(GoPoint p, SgBlackWhite c) const {
  return (m_state.m_nuNeighbors[c][p] > 0);
}

inline bool GoBoard::HasNeighborsOrDiags(GoPoint p, SgBlackWhite c) const {
  return HasNeighbors(p, c) || HasDiagonals(p, c);
}

inline bool GoBoard::InAtari(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  return AtMostNumLibs(p, 1);
}

inline bool GoBoard::IsInBlock(GoPoint p, GoPoint anchor) const {
  DBG_ASSERT(Occupied(anchor));
  DBG_ASSERT(Anchor(anchor) == anchor);
  const Block* b = m_state.m_block[p];
  return (b != 0 && b->Anchor() == anchor);
}

inline bool GoBoard::IsLibertyOfBlock(GoPoint p, GoPoint anchor) const {
  DBG_ASSERT(IsEmpty(p));
  DBG_ASSERT(Occupied(anchor));
  DBG_ASSERT(Anchor(anchor) == anchor);
  const Block* b = m_state.m_block[anchor];
  if (m_state.m_nuNeighbors[b->Color()][p] == 0)
    return false;
  return (m_state.m_block[p - GO_NORTH_SOUTH] == b
      || m_state.m_block[p - GO_WEST_EAST] == b
      || m_state.m_block[p + GO_WEST_EAST] == b
      || m_state.m_block[p + GO_NORTH_SOUTH] == b);
}

inline bool GoBoard::CanCapture(GoPoint p, SgBlackWhite c) const {
  SgBlackWhite opp = SgOppBW(c);
  for (GoNbIterator nb(*this, p); nb; ++nb)
    if (IsColor(*nb, opp) && AtMostNumLibs(*nb, 1))
      return true;
  return false;
}

inline bool GoBoard::InCenter(GoPoint p) const {
  return Centers()[p];
}

inline bool GoBoard::InCorner(GoPoint p) const {
  return Corners()[p];
}

inline void GoBoard::Init(int size, const GoSetup& setup) {
  Init(size, m_rules, setup);
}

inline void GoBoard::Reset() {
  Init(m_size, m_rules, m_setup);
}

inline bool GoBoard::IsSuicide(GoPoint p, SgBlackWhite toPlay) const {
  if (HasEmptyNeighbors(p))
    return false;
  SgBlackWhite opp = SgOppBW(toPlay);
  for (GoNbIterator it(*this, p); it; ++it) {
    SgEmptyBlackWhite c = GetColor(*it);
    if (c == toPlay && NumLiberties(*it) > 1)
      return false;
    if (c == opp && NumLiberties(*it) == 1)
      return false;
  }
  return true;
}

inline bool GoBoard::IsBorder(GoPoint p) const {
  DBG_ASSERT(p != GO_PASS);
  return m_isBorder[p];
}

inline bool GoBoard::IsColor(GoPoint p, int c) const {
  DBG_ASSERT(p != GO_PASS);
  DBG_ASSERT_EBW(c);
  return m_state.m_color[p] == c;
}

inline bool GoBoard::IsEmpty(GoPoint p) const {
  DBG_ASSERT(p != GO_PASS);
  return m_state.m_color[p] == SG_EMPTY;
}

inline bool GoBoard::IsFirst(GoPoint p) const {
  DBG_ASSERT(IsEmpty(p));
  return m_state.m_isFirst[p];
}

inline bool GoBoard::IsLegal(int p, SgBlackWhite player) const {
  DBG_ASSERT_BW(player);
  if (IsPass(p))
    return true;
  DBG_ASSERT(GoPointUtil::InBoardRange(p));
  if (!IsEmpty(p))
    return false;
  if (!Rules().AllowSuicide() && IsSuicide(p, player))
    return false;
  if (IsFirst(p))
    return true;
  if (p == m_state.m_koPoint && m_state.m_toPlay == player)
    return (AnyRepetitionAllowed() || KoRepetitionAllowed());
  if (Rules().GetKoRule() == GoRules::SIMPLEKO)
    return true;
  if (IsNewPosition() && !CanCapture(p, player))
    return true;
  auto* bd = const_cast<GoBoard*>(this);
  bd->Play(p, player);
  bool isLegal = !LastMoveInfo(GO_MOVEFLAG_ILLEGAL);
  bd->Undo();
  return isLegal;
}

inline bool GoBoard::IsNewPosition() const {
  return m_state.m_isNewPosition;
}

inline bool GoBoard::IsLegal(int p) const {
  return IsLegal(p, ToPlay());
}

inline bool GoBoard::IsPass(GoPoint p) {
  return (p == GO_PASS || GoMoveUtil::IsCouponMove(p));
}

inline bool GoBoard::IsSingleStone(GoPoint p) const {
  return (Occupied(p) && NumNeighbors(p, GetColor(p)) == 0);
}

inline bool GoBoard::IsSuicide(GoPoint p) const {
  return IsSuicide(p, ToPlay());
}

inline bool GoBoard::IsValidPoint(GoPoint p) const {
  return GoPointUtil::InBoardRange(p) && !IsBorder(p);
}

inline SgEmptyBlackWhite GoBoard::KoColor() const {
  return m_koColor;
}

inline int GoBoard::KoLevel() const {
  return m_state.m_koLevel;
}

inline SgEmptyBlackWhite GoBoard::KoLoser() const {
  return m_koLoser;
}

inline bool GoBoard::KoModifiesHash() const {
  return m_koModifiesHash;
}

inline GoPoint GoBoard::KoPoint() const {
  return m_state.m_koPoint;
}

inline bool GoBoard::KoRepetitionAllowed() const {
  return m_allowKoRepetition;
}

inline int GoBoard::LastBoardPoint() const {
  return m_const.LastBoardPoint();
}

inline bool GoBoard::LastMoveInfo(GoMoveInfoFlag flag) const {
  return m_moveInfo.test(flag);
}

inline int GoBoard::Left(GoPoint p) const {
  return m_const.Left(p);
}

inline GoGrid GoBoard::Line(GoPoint p) const {
  return m_const.Line(p);
}

inline const GoPointSet& GoBoard::LineSet(GoGrid line) const {
  return m_const.LineSet(line);
}

inline int GoBoard::MoveNumber() const {
  return m_moves->Length();
}

inline int GoBoard::Num8Neighbors(GoPoint p, SgBlackWhite c) const {
  return NumNeighbors(p, c) + NumDiagonals(p, c);
}

inline int GoBoard::Num8EmptyNeighbors(GoPoint p) const {
  return NumEmptyNeighbors(p) + NumEmptyDiagonals(p);
}

inline int GoBoard::NuCapturedStones() const {
  return m_capturedStones.Length();
}

inline int GoBoard::NumDiagonals(GoPoint p, SgBoardColor c) const {
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

inline int GoBoard::NumEmptyDiagonals(GoPoint p) const {
  return NumDiagonals(p, SG_EMPTY);
}

inline int GoBoard::NumEmptyNeighbors(GoPoint p) const {
  return m_state.m_nuNeighborsEmpty[p];
}

inline int GoBoard::NumLiberties(GoPoint p) const {
  DBG_ASSERT(IsValidPoint(p));
  DBG_ASSERT(Occupied(p));
  return m_state.m_block[p]->NumLiberties();
}

inline int GoBoard::NumNeighbors(GoPoint p, SgBlackWhite c) const {
  return m_state.m_nuNeighbors[c][p];
}

inline int GoBoard::NumPrisoners(SgBlackWhite color) const {
  return m_state.m_prisoners[color];
}

inline int GoBoard::NumStones(GoPoint block) const {
  DBG_ASSERT(Occupied(block));
  return m_state.m_block[block]->NumStones();
}

inline GoPointSet GoBoard::Occupied() const {
  return m_state.m_all[SG_BLACK] | m_state.m_all[SG_WHITE];
}

inline bool GoBoard::Occupied(GoPoint p) const {
  return (m_state.m_block[p] != 0);
}

inline bool GoBoard::OccupiedInAtari(GoPoint p) const {
  const Block* b = m_state.m_block[p];
  return (b != 0 && b->NumLiberties() <= 1);
}

inline bool GoBoard::OnEdge(GoPoint p) const {
  return Edges()[p];
}

inline SgBlackWhite GoBoard::Opponent() const {
  return SgOppBW(m_state.m_toPlay);
}

inline void GoBoard::Play(GoPlayerMove move) {
  Play(move.Point(), move.Color());
}

inline void GoBoard::Play(GoPoint p) {
  Play(p, ToPlay());
}

inline GoGrid GoBoard::Pos(GoPoint p) const {
  return m_const.Pos(p);
}

inline int GoBoard::Right(GoPoint p) const {
  return m_const.Right(p);
}

inline GoRules& GoBoard::Rules() {
  return m_rules;
}

inline const GoRules& GoBoard::Rules() const {
  return m_rules;
}

inline void GoBoard::SetKoLoser(SgEmptyBlackWhite color) {
  DBG_ASSERT(KoLevel() == 0);
  m_koLoser = color;
}

inline void GoBoard::SetKoModifiesHash(bool modify) {
  m_koModifiesHash = modify;
}

inline void GoBoard::SetToPlay(SgBlackWhite player) {
  DBG_ASSERT_BW(player);
  m_state.m_toPlay = player;
}

inline const GoSetup& GoBoard::Setup() const {
  return m_setup;
}

inline int GoBoard::Side(GoPoint p, int index) const {
  return m_const.Side(p, index);
}

inline const GoPointSet& GoBoard::SideExtensions() const {
  return m_const.SideExtensions();
}

inline GoGrid GoBoard::Size() const {
  return m_size;
}

inline GoPoint GoBoard::TheLiberty(GoPoint p) const {
  DBG_ASSERT(Occupied(p));
  DBG_ASSERT(NumLiberties(p) == 1);
  return m_state.m_block[p]->Liberties()[0];
}

inline SgBlackWhite GoBoard::ToPlay() const {
  return m_state.m_toPlay;
}

inline int GoBoard::TotalNumEmpty() const {
  return (Size() * Size() - m_state.m_numStones[SG_BLACK]
      - m_state.m_numStones[SG_WHITE]);
}

inline const SgBWArray<int>& GoBoard::TotalNumStones() const {
  return m_state.m_numStones;
}

inline int GoBoard::TotalNumStones(SgBlackWhite color) const {
  return m_state.m_numStones[color];
}

inline int GoBoard::Up(GoPoint p) const {
  return m_const.Up(p);
}

#endif
