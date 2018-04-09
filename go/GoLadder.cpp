

#include "platform/SgSystem.h"
#include "GoLadder.h"

#include <algorithm>
#include <memory>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoModBoard.h"
#include "lib/SgVector.h"
#include "lib/SgStack.h"

using GoBoardUtil::NeighborsOfColor;
using GoBoardUtil::PlayIfLegal;

namespace {

const int GOOD_FOR_PREY = 1000;
const int GOOD_FOR_HUNTER = -1000;

}

GoLadder::GoLadder() {}

inline bool GoLadder::CheckMoveOverflow() const {
  return m_bd->MoveNumber() >= m_maxMoveNumber;
}

void GoLadder::InitMaxMoveNumber() {
  const int RESERVE = 5;
  m_maxMoveNumber = std::min(m_bd->MoveNumber() + MAX_LADDER_MOVES,
                             GO_MAX_NUM_MOVES - RESERVE);
}

void GoLadder::MarkStonesAsPrey(GoPoint p, SgVector<GoPoint>* stones) {
  DBG_ASSERT(m_bd->IsValidPoint(p));
  if (m_bd->Occupied(p)) {
    for (GoBoard::StoneIterator it(*m_bd, p); it; ++it) {
      GoPoint s = *it;
      m_partOfPrey.Include(s);
      if (stones)
        stones->PushBack(s);
    }
  }
}

void GoLadder::FilterAdjacent(GoPointList& adjBlocks) {
  GoPointList temp;
  for (GoPointList::Iterator it(adjBlocks); it; ++it) {
    GoPoint block = *it;
    if (m_bd->IsColor(block, m_hunterColor)
        && m_bd->InAtari(block)
        && BlockIsAdjToPrey(block, 1))
      temp.PushBack(block);
  }
  ReduceToBlocks(temp);
  adjBlocks = temp;
}

bool GoLadder::PointIsAdjToPrey(GoPoint p) {
  return m_partOfPrey[p - GO_NORTH_SOUTH]
      || m_partOfPrey[p - GO_WEST_EAST]
      || m_partOfPrey[p + GO_WEST_EAST]
      || m_partOfPrey[p + GO_NORTH_SOUTH];
}

bool GoLadder::BlockIsAdjToPrey(GoPoint p, int numAdj) {
  DBG_ASSERT(m_bd->IsColor(p, m_hunterColor));
  for (GoBoard::StoneIterator it(*m_bd, p); it; ++it)
    if (PointIsAdjToPrey(*it) && --numAdj == 0)
      return true;
  return false;
}

int GoLadder::PlayHunterMove(int depth, GoPoint move, GoPoint lib1,
                             GoPoint lib2, const GoPointList& adjBlk,
                             SgVector<GoPoint>* sequence) {
  DBG_ASSERT(move == lib1 || move == lib2);
  int result = 0;
  if (PlayIfLegal(*m_bd, move, m_hunterColor)) {
    GoPointList newAdj;
    if (m_bd->InAtari(move))
      newAdj.PushBack(move);
    for (GoPointList::Iterator it(adjBlk); it; ++it) {
      GoPoint block = *it;
      if (!m_bd->AreInSameBlock(block, move)) {
        if (!m_bd->CapturingMove() || m_bd->InAtari(block))
          newAdj.PushBack(block);
      }
    }
    if (move == lib1)
      lib1 = lib2;
    result = PreyLadder(depth + 1, lib1, newAdj, sequence);
    if (sequence)
      sequence->PushBack(move);
    m_bd->Undo();
  } else {
    if (sequence)
      sequence->Clear();
    result = GOOD_FOR_PREY - depth;
  }
  return result;
}

int GoLadder::PlayPreyMove(int depth, GoPoint move, GoPoint lib1,
                           const GoPointList& adjBlk,
                           SgVector<GoPoint>* sequence) {
  int result = 0;
  GoPointList newAdj(adjBlk);
  SgVector<GoPoint> newLib;
  SgVector<GoPoint> newStones;
  SgVector<GoPoint> neighbors;
  if (move == lib1) {
    NeighborsOfColor(*m_bd, move, m_preyColor, &neighbors);
    for (SgVectorIterator<GoPoint> iter(neighbors); iter; ++iter) {
      GoPoint block = *iter;
      if (!m_partOfPrey[block]) {
        MarkStonesAsPrey(block, &newStones);
        GoPointList temp =
            GoBoardUtil::AdjacentStones(*m_bd, block);
        newAdj.PushBackList(temp);
        for (GoBoard::LibertyIterator it(*m_bd, block); it; ++it)
          newLib.Include(*it);
      }
    }
    m_partOfPrey.Include(move);
  }
  if (PlayIfLegal(*m_bd, move, m_preyColor)) {
    if (move == lib1) {
      NeighborsOfColor(*m_bd, move, SG_EMPTY, &neighbors);
      for (SgVectorIterator<GoPoint> iter(newLib); iter; ++iter) {
        GoPoint point = *iter;
        if (m_bd->IsEmpty(point))
          neighbors.Include(point);
      }
    } else {
      neighbors.PushBack(lib1);
    }
    if (m_bd->CapturingMove()) {
      for (GoPointList::Iterator it(m_bd->CapturedStones()); it;
           ++it) {
        GoPoint stone = *it;
        if (PointIsAdjToPrey(stone))
          neighbors.Include(stone);
      }
    }
    DBG_ASSERT(!neighbors.IsEmpty());
    lib1 = neighbors[0];
    DBG_ASSERT(m_bd->IsEmpty(lib1));
    GoArrayList<GoPoint, 4> temp =
        NeighborsOfColor(*m_bd, move, m_hunterColor);
    newAdj.PushBackList(temp);
    FilterAdjacent(newAdj);

    if (neighbors.Length() == 1)
      result = HunterLadder(depth + 1, lib1, newAdj, sequence);
    else if (neighbors.Length() == 2) {
      GoPoint lib2 = neighbors[1];
      DBG_ASSERT(m_bd->IsEmpty(lib2));
      result = HunterLadder(depth + 1, lib1, lib2, newAdj, sequence);
    } else {
      if (sequence)
        sequence->Clear();
      result = GOOD_FOR_PREY - (depth + 1);
    }
    if (sequence)
      sequence->PushBack(move);
    m_bd->Undo();
  } else {
    if (sequence)
      sequence->Clear();
    result = GOOD_FOR_HUNTER + depth;
  }
  m_partOfPrey.Exclude(move);
  m_partOfPrey.Exclude(newStones);

  return result;
}

int GoLadder::PreyLadder(int depth, GoPoint lib1,
                         const GoPointList& adjBlk,
                         SgVector<GoPoint>* sequence) {
  if (CheckMoveOverflow())
    return GOOD_FOR_PREY;
  int result = 0;
  for (GoPointList::Iterator iter(adjBlk); iter; ++iter) {
    GoPoint block = *iter;
    GoPoint move = *GoBoard::LibertyIterator(*m_bd, block);
    if (BlockIsAdjToPrey(block, 3)) {
      if (sequence)
        sequence->SetTo(move);
      result = GOOD_FOR_PREY - depth;
    } else if (move != lib1) {
      result = PlayPreyMove(depth, move, lib1, adjBlk, sequence);
    }
    if (0 < result)
      break;
  }
  if (result <= 0) {
    if (sequence) {
      SgVector<GoPoint> seq2;
      int result2 = PlayPreyMove(depth, lib1, lib1, adjBlk, &seq2);
      if (result < result2 || result == 0) {
        result = result2;
        sequence->SwapWith(&seq2);
      }
    } else {
      int result2 = PlayPreyMove(depth, lib1, lib1, adjBlk, 0);
      if (result < result2 || result == 0)
        result = result2;
    }
  }
  return result;
}

int GoLadder::HunterLadder(int depth, GoPoint lib1, const GoPointList& adjBlk,
                           SgVector<GoPoint>* sequence) {
  SuppressUnused(adjBlk);
  if (CheckMoveOverflow())
    return GOOD_FOR_PREY;
  if (sequence)
    sequence->SetTo(lib1);
  return GOOD_FOR_HUNTER + depth;
}

int GoLadder::HunterLadder(int depth, GoPoint lib1, GoPoint lib2,
                           const GoPointList& adjBlk,
                           SgVector<GoPoint>* sequence) {
  if (CheckMoveOverflow())
    return GOOD_FOR_PREY;
  int result = 0;
  if (m_bd->NumEmptyNeighbors(lib1) < m_bd->NumEmptyNeighbors(lib2)) {
    std::swap(lib1, lib2);
  }
  if (m_bd->NumEmptyNeighbors(lib1) == 3
      && !GoPointUtil::AreAdjacent(lib1, lib2)) {
    m_bd->Play(lib1, m_hunterColor);
    result = PreyLadder(depth + 1, lib2, adjBlk, sequence);
    if (sequence)
      sequence->PushBack(lib1);
    m_bd->Undo();
  } else {
    if (!adjBlk.IsEmpty()
        && *GoBoard::LibertyIterator(*m_bd, adjBlk[0]) == lib2) {
      std::swap(lib1, lib2);
    }
    result = PlayHunterMove(depth, lib1, lib1, lib2,
                            adjBlk, sequence);
    if (0 <= result) {
      if (sequence) {
        SgVector<GoPoint> seq2;
        int result2 = PlayHunterMove(depth, lib2, lib1, lib2,
                                     adjBlk, &seq2);
        if (result2 < result) {
          result = result2;
          sequence->SwapWith(&seq2);
        }
      } else {
        int result2 = PlayHunterMove(depth, lib2, lib1, lib2,
                                     adjBlk, 0);
        if (result2 < result)
          result = result2;
      }
    }
  }
  return result;
}

void GoLadder::ReduceToBlocks(GoPointList& stones) {
  if (stones.IsEmpty());
  else if (stones.Length() <= 1) {
    if (m_bd->IsEmpty(stones[0]))
      stones.Clear();
  } else {
    GoPointList visited;
    GoPointList result;
    for (GoPointList::Iterator it(stones); it; ++it) {
      GoPoint stone = *it;
      if (m_bd->Occupied(stone) && !visited.Contains(stone)) {
        result.PushBack(stone);
        for (GoBoard::StoneIterator sit(*m_bd, stone); sit; ++sit)
          visited.PushBack(*sit);
      }
    }
    stones = result;
  }
}

int GoLadder::Ladder(const GoBoard& bd, GoPoint prey, SgBlackWhite toPlay,
                     SgVector<GoPoint>* sequence, bool twoLibIsEscape) {
  GoModBoard modBoard(bd);
  m_bd = &modBoard.Board();
  InitMaxMoveNumber();
  if (sequence)
    sequence->Clear();
  if (!m_bd->Occupied(prey))
    return 0;
  if (CheckMoveOverflow())
    return GOOD_FOR_PREY;
  int result = 0;
  m_preyColor = m_bd->GetStone(prey);
  m_hunterColor = SgOppBW(m_preyColor);
  int numLib = m_bd->NumLiberties(prey);
  if (2 < numLib)
    result = GOOD_FOR_PREY;
  else {
    GoBoard::LibertyIterator libit(*m_bd, prey);
    GoPoint lib1 = *libit;
    m_partOfPrey.Clear();
    MarkStonesAsPrey(prey);
    GoPointList adjBlk = GoBoardUtil::AdjacentStones(*m_bd, prey);
    FilterAdjacent(adjBlk);
    if (toPlay == m_preyColor) {
      if (numLib == 1)
        result = PreyLadder(0, lib1, adjBlk, sequence);
      else if (twoLibIsEscape)
        result = GOOD_FOR_PREY;
      else {
        SgVector<GoPoint> movesToTry;
        adjBlk = GoBoardUtil::AdjacentStones(*m_bd, prey);
        ReduceToBlocks(adjBlk);
        for (GoPointList::Iterator iterAdj(adjBlk); iterAdj;
             ++iterAdj) {
          GoPoint block = *iterAdj;
          DBG_ASSERT(m_bd->IsColor(block, m_hunterColor));
          DBG_ASSERT(BlockIsAdjToPrey(block, 1));
          if (m_bd->NumLiberties(block) <= 2)
            for (GoBoard::LibertyIterator it(*m_bd, block); it;
                 ++it)
              movesToTry.PushBack(*it);
        }
        ++libit;
        GoPoint lib2 = *libit;
        movesToTry.PushBack(lib1);
        movesToTry.PushBack(lib2);
        SgVector<GoPoint> neighbors;
        NeighborsOfColor(*m_bd, lib1, SG_EMPTY, &neighbors);
        movesToTry.Concat(&neighbors);
        NeighborsOfColor(*m_bd, lib2, SG_EMPTY, &neighbors);
        movesToTry.Concat(&neighbors);
        for (SgVectorIterator<GoPoint> it(movesToTry); it; ++it) {
          if (PlayIfLegal(*m_bd, *it, m_preyColor)) {
            if (Ladder(bd, prey, m_hunterColor, 0, twoLibIsEscape)
                > 0) {
              if (sequence)
                sequence->PushBack(*it);
              result = GOOD_FOR_PREY;
            }
            m_bd->Undo();
          }
          if (result != 0)
            break;
        }
        if (result == 0)
          result = GOOD_FOR_HUNTER;
      }
    } else {
      if (IsSnapback(prey))
        result = GOOD_FOR_PREY;
      else {
        ++libit;
        if (libit)
          result = HunterLadder(0, lib1, *libit, adjBlk, sequence);
        else
          result = HunterLadder(0, lib1, adjBlk, sequence);
      }
    }
  }
  if (sequence)
    sequence->Reverse();
  return result;
}

bool GoLadder::IsSnapback(GoPoint prey) {
  bool isSnapback = false;
  if (m_bd->IsSingleStone(prey) && m_bd->InAtari(prey)) {
    GoPoint liberty = *GoBoard::LibertyIterator(*m_bd, prey);
    if (PlayIfLegal(*m_bd, liberty, SgOppBW(m_bd->GetStone(prey)))) {
      isSnapback = (m_bd->InAtari(liberty)
          && !m_bd->IsSingleStone(liberty));
      m_bd->Undo();
    }
  }
  return isSnapback;
}

bool GoLadderUtil::Ladder(const GoBoard& bd, GoPoint prey,
                          SgBlackWhite toPlay, bool twoLibIsEscape,
                          SgVector<GoPoint>* sequence) {
  DBG_ASSERT(bd.IsValidPoint(prey));
  DBG_ASSERT(bd.Occupied(prey));
#ifndef NDEBUG
  SgHashCode oldHash = bd.GetHashCode();
#endif
  GoLadder ladder;
  int result = ladder.Ladder(bd, prey, toPlay, sequence, twoLibIsEscape);
#ifndef NDEBUG
  DBG_ASSERT(oldHash == bd.GetHashCode());
#endif
  DBG_ASSERT(result != 0);
  return (result < 0);
}

GoLadderStatus GoLadderUtil::LadderStatus(const GoBoard& bd, GoPoint prey,
                                          bool twoLibIsEscape,
                                          GoPoint* toCapture,
                                          GoPoint* toEscape) {
  DBG_ASSERT(bd.IsValidPoint(prey));
  DBG_ASSERT(bd.Occupied(prey));
#ifndef NDEBUG
  SgHashCode oldHash = bd.GetHashCode();
#endif
  GoLadder ladder;
  SgBlackWhite preyColor = bd.GetStone(prey);
  SgVector<GoPoint> captureSequence;
  GoLadderStatus status = GO_LADDER_ESCAPED;
  if (ladder.Ladder(bd, prey, SgOppBW(preyColor), &captureSequence,
                    twoLibIsEscape) < 0) {
    SgVector<GoPoint> escapeSequence;
    if (ladder.Ladder(bd, prey, preyColor, &escapeSequence,
                      twoLibIsEscape) < 0)
      status = GO_LADDER_CAPTURED;
    else {
      status = GO_LADDER_UNSETTLED;
      DBG_ASSERT(captureSequence.NonEmpty());
      DBG_ASSERT(twoLibIsEscape || escapeSequence.NonEmpty());
      if (toCapture)
        *toCapture = captureSequence.Front();
      if (toEscape)
        *toEscape = escapeSequence.IsEmpty() ? GO_PASS :
                    escapeSequence.Front();
    }
  }
#ifndef NDEBUG
  DBG_ASSERT(oldHash == bd.GetHashCode());
#endif
  return status;
}

bool GoLadderUtil::IsLadderCaptureMove(const GoBoard& constBd,
                                       GoPoint prey, GoPoint firstMove) {
  DBG_ASSERT(constBd.NumLiberties(prey) == 2);
  DBG_ASSERT(constBd.IsLibertyOfBlock(firstMove, constBd.Anchor(prey)));

  GoModBoard mbd(constBd);
  GoBoard& bd = mbd.Board();
  const SgBlackWhite defender = bd.GetStone(prey);
  const SgBlackWhite attacker = SgOppBW(defender);
  GoRestoreToPlay r(bd);
  bd.SetToPlay(attacker);
  if (PlayIfLegal(bd, firstMove, attacker)) {
    GoLadder ladder;
    bool isCapture = ladder.Ladder(bd, prey, defender,
                                   0, false
    ) < 0;
    bd.Undo();
    return isCapture;
  } else
    return false;
}

bool GoLadderUtil::IsLadderEscapeMove(const GoBoard& constBd,
                                      GoPoint prey, GoPoint firstMove) {
  GoModBoard mbd(constBd);
  GoBoard& bd = mbd.Board();
  const SgBlackWhite defender = bd.GetStone(prey);
  const SgBlackWhite attacker = SgOppBW(defender);
  GoRestoreToPlay r(bd);
  bd.SetToPlay(defender);
  if (PlayIfLegal(bd, firstMove, defender)) {
    GoLadder ladder;
    bool isCapture = ladder.Ladder(bd, prey, attacker,
                                   0, false
    ) < 0;
    bd.Undo();
    return !isCapture;
  } else
    return false;
}

void GoLadderUtil::FindLadderEscapeMoves(const GoBoard& bd, GoPoint prey,
                                         SgVector<GoPoint>& escapeMoves) {
  DBG_ASSERT(bd.NumLiberties(prey) == 1);
  DBG_ASSERT(escapeMoves.IsEmpty());

  const GoPoint lib = bd.TheLiberty(prey);
  SgVector<GoPoint> candidates;
  candidates.PushBack(lib);
  if (IsLadderEscapeMove(bd, prey, lib))
    escapeMoves.PushBack(lib);
  for (GoAdjBlockIterator<GoBoard> it(bd, prey, 1); it; ++it) {
    GoPoint p = bd.TheLiberty(*it);
    if (!candidates.Contains(p)) {
      candidates.PushBack(p);
      if (IsLadderEscapeMove(bd, prey, p))
        escapeMoves.PushBack(p);
    }
  }
}

bool GoLadderUtil::IsProtectedLiberty(const GoBoard& bd, GoPoint liberty,
                                      SgBlackWhite color) {
  bool ignoreLadder;
  bool ignoreKo;
  return IsProtectedLiberty(bd, liberty, color, ignoreLadder, ignoreKo,
                            true);
}

bool GoLadderUtil::IsProtectedLiberty(const GoBoard& bd1, GoPoint liberty,
                                      SgBlackWhite col, bool& byLadder,
                                      bool& isKoCut, bool tryLadder) {
  byLadder = false;
  isKoCut = false;
  GoModBoard mbd(bd1);
  GoBoard& bd = mbd.Board();
  const SgBlackWhite toPlay = bd1.ToPlay();
  bd.SetToPlay(SgOppBW(col));
  bool isProtected;
  if (!PlayIfLegal(bd, liberty))
    isProtected = bd.LastMoveInfo(GO_MOVEFLAG_SUICIDE);
  else {
    if (bd.LastMoveInfo(GO_MOVEFLAG_SUICIDE))
      isProtected = true;
    else {
      if (bd.InAtari(liberty)) {
        if (bd.NumStones(liberty) > 1)
          isProtected = true;
        else {
          GoPoint p = bd.TheLiberty(liberty);
          if (PlayIfLegal(bd, p)) {
            isProtected = (bd.NumStones(p) != 1)
                || (bd.NumLiberties(p) != 1);
            bd.Undo();
          } else
            isProtected = false;

          if (!isProtected)
            isKoCut = true;
        }
      } else if (tryLadder) {
        isProtected = Ladder(bd, liberty, bd.ToPlay(), true);
        if (isProtected)
          byLadder = true;
      } else
        isProtected = false;
    }
    bd.Undo();
  }
  bd.SetToPlay(toPlay);
  return isProtected;
}

GoPoint GoLadderUtil::TryLadder(const GoBoard& bd, GoPoint prey,
                                SgBlackWhite firstPlayer) {
  SgVector<GoPoint> sequence;
  bool isCaptured = Ladder(bd, prey, firstPlayer, true, &sequence);
  GoPoint p;
  if (isCaptured != (firstPlayer == bd.GetStone(prey)))
    p = sequence.IsEmpty() ? GO_PASS : sequence.Front();
  else
    p = GO_NULLMOVE;
  return p;
}

