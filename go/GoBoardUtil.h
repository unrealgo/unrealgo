

#ifndef GO_BOARDUTIL_H
#define GO_BOARDUTIL_H

#include "GoBoard.h"
#include "board/GoBoardColor.h"
#include "platform/SgDebug.h"
#include "board/GoPoint.h"
#include "board/GoPointArray.h"
#include "lib/SgRandom.h"
#include "lib/SgStack.h"
#include "lib/SgVector.h"

namespace GoBoardDebug {

template<class BOARD = GoBoard>
class EventPrinter {
 public:
  EventPrinter(int maxNuInstances);
  void PrintFirstFew(const BOARD& bd, std::string message,
                     float probability,
                     GoPoint move,
                     GoPoint move2 = GO_NULLMOVE);

 private:
  int m_maxNuInstances;
  SgRandom m_random;
};

}

template<class BOARD>
GoBoardDebug::EventPrinter<BOARD>::EventPrinter(int maxNuInstances)
    : m_maxNuInstances(maxNuInstances) {}

template<class BOARD>
void GoBoardDebug::EventPrinter<BOARD>::
PrintFirstFew(const BOARD& bd,
              std::string message,
              float probability,
              GoPoint move,
              GoPoint move2) {
  if (m_random.Float_01() <= probability && --m_maxNuInstances >= 0) {
    SgDebug() << message << ' '
              << SgBW(bd.ToPlay()) << ' '
              << GoWritePoint(move);
    if (move2 != GO_NULLMOVE)
      SgDebug() << ' ' << GoWritePoint(move2);
    SgDebug() << '\n' << bd << std::endl;
  }
}

namespace GoBoardUtil {

void AddNeighborBlocksOfColor(const GoBoard& bd,
                              GoPoint p,
                              SgBlackWhite color,
                              SgVector<GoPoint>& neighbors);
void AddWall(GoBoard& bd,
             SgBlackWhite color,
             GoPoint start,
             int length,
             int direction);
GoPointList AdjacentStones(const GoBoard& bd, GoPoint point);
void AdjacentBlocks(const GoBoard& bd, GoPoint p, int maxLib,
                    SgVector<GoPoint>* blocks);
GoPointList AllLegalMoves(const GoBoard& bd);
int Approx2Libs(const GoBoard& board, GoPoint block, GoPoint p,
                SgBlackWhite color);
template<class BOARD>
bool AtariDefenseMoves(const BOARD& bd, const GoPoint lastMove,
                       GoPointList& moves);
bool AtLeastTwoSharedLibs(const GoBoard& bd, GoPoint block1,
                          GoPoint block2);
bool BlockIsAdjacentTo(const GoBoard& bd, GoPoint block,
                       const GoPointSet& walls);
void BlocksAdjacentToPoints(const GoBoard& bd,
                            const SgVector<GoPoint>& points,
                            SgBlackWhite c,
                            SgVector<GoPoint>* anchors);
void BlocksAdjacentToPoints(const GoBoard& bd, const GoPointSet& points,
                            SgBlackWhite c, SgVector<GoPoint>* anchors);
template<class BOARD>
bool CaptureAdjacentBlocks(const BOARD& bd, GoPoint anchor,
                           GoPointList& moves);
GoPointArray<int> CfgDistance(const GoBoard& bd, GoPoint p,
                              int maxDist = std::numeric_limits<int>::max());
bool ContainsAnchor(const GoPoint anchor[], const GoPoint p);
void DiagonalsOfColor(const GoBoard& bd, GoPoint p, int c,
                      SgVector<GoPoint>* diagonals);
void DumpBoard(const GoBoard& bd, std::ostream& out = SgDebug());
bool EndOfGame(const GoBoard& bd);
void ExpandToBlocks(const GoBoard& board, GoPointSet& pointSet);
template<class BOARD>
GoPoint FindNeighbor(const BOARD& bd, GoPoint p, SgEmptyBlackWhite c);
template<class BOARD>
GoPoint FindDiagNeighbor(const BOARD& bd, GoPoint p, SgEmptyBlackWhite c);
template<class BOARD>
bool GainsLiberties(const BOARD& bd, GoPoint anchor, GoPoint lib);
bool GenerateIfLegal(const GoBoard& bd,
                     GoPoint move,
                     SgVector<GoPoint>* moves);
void GetCoordString(GoMove move, std::string* s, int boardSize);
void GetCoordString(const GoBoard& board, GoMove move, std::string* s);
GoRect GetDirtyRegion(const GoBoard& bd, GoMove move, SgBlackWhite color,
                      bool checklibs = false, bool premove = false);
bool HasAdjacentBlocks(const GoBoard& bd, GoPoint p, int maxLib);
bool HasStonesOfBothColors(const GoBoard& bd,
                           const SgVector<GoPoint>& stones);
template<class BOARD>
bool IsBoardEmpty(const BOARD& bd);
template<class BOARD>
bool IsCompletelySurrounded(const BOARD& bd, GoPoint p);
template<class BOARD>
int NumNeighborBlocks(const BOARD& bd, GoPoint p,
                      SgBlackWhite blockColor);
template<class BOARD>
bool IsCuttingPoint(const BOARD& bd, GoPoint p, SgBlackWhite blockColor);
bool IsHandicapPoint(GoGrid size, GoGrid col, GoGrid row);
template<class BOARD>
bool IsNeighborOfSome(const BOARD& bd, GoPoint p, GoPoint anchors[],
                      SgBlackWhite toPlay);
template<class BOARD>
bool IsSimpleChain(const BOARD& bd, GoPoint block, GoPoint& other);
bool IsSimpleEyeOfBlock(const GoBoard& bd, GoPoint lib,
                        GoPoint blockAnchor,
                        const SgVector<GoPoint>& eyes);
bool IsSnapback(const GoBoard& bd, GoPoint p);
template<class BOARD>
bool KeepsOrGainsLiberties(const BOARD& bd, GoPoint anchor, GoPoint lib);
GoPointSet Lines(const GoBoard& bd, GoGrid from, GoGrid to);
bool ManySecondaryLibs(const GoBoard& bd, GoPoint block);
bool MoveNotLegalOrAtari(const GoBoard& bd, GoPoint move);
bool MoveLegalAndNotAtari(const GoBoard& bd, GoPoint move);
GoArrayList<GoPoint, 4> NeighborsOfColor(const GoBoard& bd, GoPoint p,
                                         int c);
void NeighborsOfColor(const GoBoard& bd, GoPoint p, int c,
                      SgVector<GoPoint>* neighbors);
GoPoint OtherLiberty(const GoBoard& bd, GoPoint anchor, GoPoint lib1);
bool PlayIfLegal(GoBoard& bd, GoPoint p, SgBlackWhite player);
bool PlayIfLegal(GoBoard& bd, GoPoint p);
bool PointHasAdjacentBlock(const GoBoard& bd, GoPoint p,
                           SgBlackWhite blockColor, int maxLib);
void ReduceToAnchors(const GoBoard& bd, SgVector<GoPoint>* stones);
void ReduceToAnchors(const GoBoard& bd, const SgVector<GoPoint>& stones,
                     GoArrayList<GoPoint, GO_MAXPOINT>& anchors);
void RegionCode(const GoBoard& bd, const SgVector<GoPoint>& region,
                SgHashCode* c);
bool RemainingChineseHandicap(const GoBoard& bd);
template<class BOARD>
float Score(const BOARD& bd,
            float komi,
            GoPointArray<SgEmptyBlackWhite>* scoreBoard = 0);
bool ScorePosition(const GoBoard& bd, const GoPointSet& deadStones,
                   float& score);
template<class BOARD>
SgEmptyBlackWhite ScorePoint(const BOARD& bd, GoPoint p, bool noCheck);
template<class BOARD>
float ScoreSimpleEndPosition(const BOARD& bd, float komi,
                             const GoBWSet& safe, bool noCheck,
                             GoPointArray<SgEmptyBlackWhite>* scoreBoard);
float ScoreSimpleEndPosition(const GoBoard& bd, float komi,
                             bool noCheck = false);
template<class BOARD>
bool SelfAtari(const BOARD& bd, GoPoint p);
template<class BOARD>
bool SelfAtari(const BOARD& bd, GoPoint p, int& numStones);
template<class BOARD>
bool SelfAtariForColor(const BOARD& bd, GoPoint p,
                       SgBlackWhite toPlay);
void SharedLiberties(const GoBoard& bd, GoPoint block1, GoPoint block2,
                     SgVector<GoPoint>* sharedLibs);
void SharedLibertyBlocks(const GoBoard& bd, GoPoint anchor, int maxLib,
                         SgVector<GoPoint>* blocks);
int Stones(const GoBoard& bd, GoPoint p, GoPoint stones[]);
void TestForChain(GoBoard& bd, GoPoint block, GoPoint block2, GoPoint lib,
                  SgVector<GoPoint>* extended);
bool TrompTaylorPassWins(const GoBoard& bd, SgBlackWhite toPlay);
template<class BOARD>
float TrompTaylorScore(const BOARD& bd, float komi,
                       GoPointArray<SgEmptyBlackWhite>* scoreBoard = 0);
template<class BOARD>
float JapaneseScore(const BOARD& bd, float komi,
                    GoPointArray<SgEmptyBlackWhite>* scoreBoard = 0);
bool TwoPasses(const GoBoard& bd);
void UndoAll(GoBoard& bd);

}
namespace {

template<class BOARD>
bool AchievesLibertyTarget(const BOARD& bd, GoPoint anchor,
                           GoPoint lib, int nu) {
  DBG_ASSERT(bd.IsEmpty(lib));
  DBG_ASSERT(bd.Anchor(anchor) == anchor);
  DBG_ASSERT(bd.IsLibertyOfBlock(lib, anchor));

  const SgBlackWhite color = bd.GetStone(anchor);
  for (GoNb4Iterator<BOARD> it(bd, lib); it; ++it) {
    const SgEmptyBlackWhite c = bd.GetColor(*it);
    if (c == SG_EMPTY) {
      if (!bd.IsLibertyOfBlock(*it, anchor))
        if (++nu >= 0)
          return true;
    } else if (c == color) {
      const GoPoint anchor2 = bd.Anchor(*it);
      if (anchor != anchor2)
        for (typename BOARD::LibertyIterator lit(bd, anchor2); lit;
             ++lit)
          if (!bd.IsLibertyOfBlock(*lit, anchor))
            if (++nu >= 0)
              return true;
    }
  }
  return false;
}
}

inline GoPointList GoBoardUtil::AllLegalMoves(const GoBoard& bd) {
  GoPointList legalBoardMoves;
  for (GoBoard::Iterator it(bd); it; ++it)
    if (bd.IsLegal(*it))
      legalBoardMoves.PushBack(*it);
  return legalBoardMoves;
}

inline bool GoBoardUtil::ContainsAnchor(const GoPoint anchor[],
                                        const GoPoint p) {
  for (int i = 0; anchor[i] != GO_ENDPOINT; ++i)
    if (p == anchor[i])
      return true;
  return false;
}

template<class BOARD>
inline GoPoint GoBoardUtil::FindNeighbor(const BOARD& bd, GoPoint p,
                                         SgEmptyBlackWhite c) {
  if (bd.IsColor(p + GO_NORTH_SOUTH, c))
    return p + GO_NORTH_SOUTH;
  if (bd.IsColor(p - GO_NORTH_SOUTH, c))
    return p - GO_NORTH_SOUTH;
  if (bd.IsColor(p + GO_WEST_EAST, c))
    return p + GO_WEST_EAST;
  DBG_ASSERT(bd.IsColor(p - GO_WEST_EAST, c));
  return p - GO_WEST_EAST;
}

template<class BOARD>
inline GoPoint GoBoardUtil::FindDiagNeighbor(const BOARD& bd, GoPoint p,
                                             SgEmptyBlackWhite c) {
  if (bd.IsColor(p + GO_NORTH_SOUTH + GO_WEST_EAST, c))
    return p + GO_NORTH_SOUTH + GO_WEST_EAST;
  if (bd.IsColor(p + GO_NORTH_SOUTH - GO_WEST_EAST, c))
    return p + GO_NORTH_SOUTH - GO_WEST_EAST;
  if (bd.IsColor(p - GO_NORTH_SOUTH + GO_WEST_EAST, c))
    return p - GO_NORTH_SOUTH + GO_WEST_EAST;
  DBG_ASSERT(bd.IsColor(p - GO_NORTH_SOUTH - GO_WEST_EAST, c));
  return p - GO_NORTH_SOUTH - GO_WEST_EAST;
}

inline void GoBoardUtil::GetCoordString(const GoBoard& board, GoMove move,
                                        std::string* s) {
  GetCoordString(move, s, board.Size());
}

template<class BOARD>
bool GoBoardUtil::IsBoardEmpty(const BOARD& bd) {
  return bd.TotalNumStones(SG_BLACK) + bd.TotalNumStones(SG_WHITE) == 0;
}

template<class BOARD>
inline bool GoBoardUtil::IsCompletelySurrounded(const BOARD& bd, GoPoint p) {
  DBG_ASSERT(bd.IsEmpty(p));
  if (bd.HasEmptyNeighbors(p))
    return false;
  if (bd.HasNeighbors(p, SG_BLACK) && bd.HasNeighbors(p, SG_WHITE))
    return false;
  if (!bd.IsBorder(p - GO_NORTH_SOUTH) && bd.NumLiberties(p - GO_NORTH_SOUTH) == 1)
    return false;
  if (!bd.IsBorder(p - GO_WEST_EAST) && bd.NumLiberties(p - GO_WEST_EAST) == 1)
    return false;
  if (!bd.IsBorder(p + GO_WEST_EAST) && bd.NumLiberties(p + GO_WEST_EAST) == 1)
    return false;
  if (!bd.IsBorder(p + GO_NORTH_SOUTH) && bd.NumLiberties(p + GO_NORTH_SOUTH) == 1)
    return false;
  return true;
}

template<class BOARD>
inline int GoBoardUtil::NumNeighborBlocks(const BOARD& bd, GoPoint p,
                                          SgBlackWhite blockColor) {
  DBG_ASSERT(bd.GetColor(p) != blockColor);
  GoPoint anchors[4 + 1];
  bd.NeighborBlocks(p, blockColor, anchors);
  for (int i = 0; i < 5; ++i)
    if (anchors[i] == GO_ENDPOINT)
      return i;
  DBG_ASSERT(false);
  return 4;
}

template<class BOARD>
inline bool GoBoardUtil::IsCuttingPoint(const BOARD& bd, GoPoint p,
                                        SgBlackWhite blockColor) {
  return NumNeighborBlocks(bd, p, blockColor) >= 2;
}

template<class BOARD>
inline bool GoBoardUtil::IsNeighborOfSome(const BOARD& bd, GoPoint p,
                                          GoPoint anchors[],
                                          SgBlackWhite toPlay) {
  for (GoNb4Iterator<BOARD> it(bd, p); it; ++it) {
    const GoPoint nb = *it;
    if (bd.IsColor(nb, toPlay)) {
      GoPoint anchor = bd.Anchor(nb);
      for (int i = 0; anchors[i] != GO_ENDPOINT; ++i)
        if (anchor == anchors[i])
          return true;
    }
  }
  return false;
}

template<class BOARD>
bool GoBoardUtil::IsSimpleChain(const BOARD& bd,
                                GoPoint block,
                                GoPoint& other) {
  if (bd.NumLiberties(block) < 2)
    return false;
  block = bd.Anchor(block);
  const SgBlackWhite color = bd.GetStone(block);
  typename BOARD::LibertyIterator it(bd, block);
  const GoPoint lib1 = *it;
  ++it;
  const GoPoint lib2 = *it;
  GoPoint anchors1[4 + 1];
  GoPoint anchors2[4 + 1];
  bd.NeighborBlocks(lib1, color, anchors1);
  bd.NeighborBlocks(lib2, color, anchors2);
  for (int i = 0; anchors1[i] != GO_ENDPOINT; ++i) {
    const GoPoint anchor = anchors1[i];
    if (anchor != block
        && GoBoardUtil::ContainsAnchor(anchors2, anchor)
        ) {
      other = anchor;
      return true;
    }
  }
  return false;
}

template<class BOARD>
inline bool GoBoardUtil::GainsLiberties(const BOARD& bd,
                                        GoPoint anchor,
                                        GoPoint lib) {
  int nu = -2;
  return AchievesLibertyTarget(bd, anchor, lib, nu);
}

template<class BOARD>
inline bool GoBoardUtil::KeepsOrGainsLiberties(const BOARD& bd,
                                               GoPoint anchor,
                                               GoPoint lib) {
  int nu = -1;
  return AchievesLibertyTarget(bd, anchor, lib, nu);
}

inline GoPoint GoBoardUtil::OtherLiberty(const GoBoard& bd,
                                         GoPoint anchor,
                                         GoPoint lib1) {
  DBG_ASSERT(bd.NumLiberties(anchor) == 2);
  DBG_ASSERT(bd.IsLibertyOfBlock(lib1, anchor));
  GoBoard::LibertyIterator it(bd, anchor);
  DBG_ASSERT(it);
  if (*it != lib1)
    return *it;
  ++it;
  DBG_ASSERT(it);
  DBG_ASSERT(*it != lib1);
  return *it;
}

inline bool GoBoardUtil::PlayIfLegal(GoBoard& bd, GoPoint p) {
  return PlayIfLegal(bd, p, bd.ToPlay());
}

template<class BOARD>
SgEmptyBlackWhite GoBoardUtil::ScorePoint(const BOARD& bd, GoPoint p,
                                          bool noCheck) {
  SG_DEBUG_ONLY(noCheck);
  SgEmptyBlackWhite c = bd.GetColor(p);
  if (c != SG_EMPTY)
    return c;
  DBG_ASSERT(noCheck || bd.NumEmptyNeighbors(p) == 0
                 || GoBoardUtil::SelfAtari(bd, p));
  if (bd.NumNeighbors(p, SG_BLACK) > 0
      && bd.NumNeighbors(p, SG_WHITE) == 0)
    return SG_BLACK;
  else if (bd.NumNeighbors(p, SG_WHITE) > 0
      && bd.NumNeighbors(p, SG_BLACK) == 0) {
    DBG_ASSERT(bd.NumNeighbors(p, SG_WHITE) > 0);
    return SG_WHITE;
  } else {
    DBG_ASSERT(noCheck || GoBoardUtil::SelfAtari(bd, p));
    return SG_EMPTY;
  }
}

template<class BOARD>
float GoBoardUtil::ScoreSimpleEndPosition(const BOARD& bd, float komi,
                                          const GoBWSet& safe, bool noCheck,
                                          GoPointArray<SgEmptyBlackWhite>* scoreBoard) {
  float score = -komi;
  for (typename BOARD::Iterator it(bd); it; ++it) {
    GoPoint p = *it;
    SgEmptyBlackWhite c;
    if (safe[SG_BLACK].Contains(p))
      c = SG_BLACK;
    else if (safe[SG_WHITE].Contains(p))
      c = SG_WHITE;
    else
      c = ScorePoint(bd, p, noCheck);
    switch (c) {
      case SG_BLACK:++score;
        break;
      case SG_WHITE:--score;
        break;
      default:break;
    }
    if (scoreBoard != 0)
      (*scoreBoard)[p] = c;
  }
  return score;
}

template<class BOARD>
inline bool GoBoardUtil::SelfAtari(const BOARD& bd, GoPoint p) {
  return SelfAtariForColor(bd, p, bd.ToPlay());
}

template<class BOARD>
inline bool GoBoardUtil::SelfAtariForColor(const BOARD& bd, GoPoint p,
                                           SgBlackWhite toPlay) {

  DBG_ASSERT(bd.IsEmpty(p));
  if (bd.NumEmptyNeighbors(p) >= 2)
    return false;
  GoPoint lib = GO_NULLPOINT;
  bool hasOwnNb = false;
  bool hasCapture = false;
  for (GoNb4Iterator<BOARD> nbit(bd, p); nbit; ++nbit) {
    const GoPoint nb = *nbit;
    const SgBlackWhite nbColor = bd.GetColor(nb);
    if (nbColor == SG_EMPTY) {
      if (lib == GO_NULLPOINT)
        lib = nb;
      else if (lib != nb)
        return false;
    } else if (nbColor == toPlay) {
      if (bd.NumLiberties(nb) > 2)
        return false;
      else
        for (typename BOARD::LibertyIterator it(bd, nb); it; ++it) {
          if (*it != p) {
            if (lib == GO_NULLPOINT)
              lib = *it;
            else if (lib != *it)
              return false;
          }
        }
      hasOwnNb = true;
    } else {
      DBG_ASSERT(nbColor == SgOppBW(toPlay));
      if (bd.InAtari(nb)) {
        if (lib == GO_NULLPOINT) {
          lib = nb;
          hasCapture = true;
        } else if (lib != nb)
          return false;
      }
    }
  }

  if (lib == GO_NULLPOINT)
    return false;
  if (!hasOwnNb && hasCapture)
    return false;
  if (hasOwnNb && hasCapture) {
    GoPoint anchors[4 + 1];
    bd.NeighborBlocks(p, toPlay, 1, anchors);
    DBG_ASSERT(bd.IsColor(lib, SgOppBW(toPlay)));
    for (typename BOARD::StoneIterator it(bd, lib); it; ++it) {
      if (*it != lib && IsNeighborOfSome(bd, *it, anchors, toPlay))
        return false;
    }
  }
  return true;
}

template<class BOARD>
bool GoBoardUtil::SelfAtari(const BOARD& bd, GoPoint p, int& numStones) {
  DBG_ASSERT(bd.IsEmpty(p));
  if (bd.NumEmptyNeighbors(p) >= 2)
    return false;
  const SgBlackWhite toPlay = bd.ToPlay();
  GoPoint lib = GO_NULLPOINT;
  bool hasOwnNb = false;
  bool hasCapture = false;
  for (GoNb4Iterator<BOARD> nbit(bd, p); nbit; ++nbit) {
    const GoPoint nb = *nbit;
    const SgBlackWhite nbColor = bd.GetColor(nb);
    if (nbColor == SG_EMPTY) {
      if (lib == GO_NULLPOINT)
        lib = nb;
      else if (lib != nb)
        return false;
    } else if (nbColor == toPlay) {
      if (bd.NumLiberties(nb) > 2)
        return false;
      else
        for (typename BOARD::LibertyIterator it(bd, nb); it; ++it) {
          if (*it != p) {
            if (lib == GO_NULLPOINT)
              lib = *it;
            else if (lib != *it)
              return false;
          }
        }
      hasOwnNb = true;
    } else {
      DBG_ASSERT(nbColor == SgOppBW(toPlay));
      if (bd.InAtari(nb)) {
        if (lib == GO_NULLPOINT) {
          lib = nb;
          hasCapture = true;
        } else if (lib != nb)
          return false;
      }
    }
  }

  if (lib == GO_NULLPOINT)
    return false;
  if (!hasOwnNb && hasCapture)
    return false;
  if (hasOwnNb && hasCapture) {
    GoPoint anchors[4 + 1];
    bd.NeighborBlocks(p, toPlay, 1, anchors);
    DBG_ASSERT(bd.IsColor(lib, SgOppBW(toPlay)));
    for (typename BOARD::StoneIterator it(bd, lib); it; ++it) {
      if (*it != lib && IsNeighborOfSome(bd, *it, anchors, toPlay))
        return false;
    }
  }
  numStones = 1;
  if (hasOwnNb) {
    GoPoint anchors[4 + 1];
    bd.NeighborBlocks(p, toPlay, 2, anchors);
    for (int i = 0; anchors[i] != GO_ENDPOINT; ++i)
      numStones += bd.NumStones(anchors[i]);
  }
  return true;
}

template<class BOARD>
class GoAdjBlockIterator;

template<class BOARD>
inline bool GoBoardUtil::AtariDefenseMoves(const BOARD& bd,
                                           const GoPoint lastMove,
                                           GoPointList& moves) {
  DBG_ASSERT(moves.IsEmpty());
  DBG_ASSERT(!SgIsSpecialMove(lastMove));
  SgBlackWhite toPlay = bd.ToPlay();
  if (bd.NumNeighbors(lastMove, toPlay) == 0)
    return false;
  GoArrayList<GoPoint, 4> anchorList;
  for (GoNb4Iterator<BOARD> it(bd, lastMove); it; ++it) {
    if (bd.GetColor(*it) != toPlay || !bd.InAtari(*it))
      continue;
    GoPoint anchor = bd.Anchor(*it);
    if (anchorList.Contains(anchor))
      continue;
    anchorList.PushBack(anchor);
    GoPoint theLiberty = bd.TheLiberty(anchor);
    if (!GoBoardUtil::SelfAtari(bd, theLiberty))
      moves.PushBack(theLiberty);
    for (GoAdjBlockIterator<BOARD> it2(bd, anchor, 1); it2; ++it2) {
      GoPoint oppLiberty = bd.TheLiberty(*it2);
      if (oppLiberty != theLiberty)
        moves.PushBack(oppLiberty);
    }
  }
  return !moves.IsEmpty();
}

template<class BOARD>
bool GoBoardUtil::CaptureAdjacentBlocks(const BOARD& bd, GoPoint anchor,
                                        GoPointList& moves) {
  DBG_ASSERT(bd.Anchor(anchor) == anchor);
  bool found = false;
  for (GoAdjBlockIterator<BOARD> it(bd, anchor, 1); it; ++it) {
    GoPoint oppLiberty = bd.TheLiberty(*it);
    moves.PushBack(oppLiberty);
    found = true;
  }
  return found;
}

template<class BOARD>
float GoBoardUtil::Score(const BOARD& bd, float komi,
                         GoPointArray<SgEmptyBlackWhite>* scoreBoard) {
  return bd.Rules().JapaneseScoring() ?
         GoBoardUtil::JapaneseScore(bd, komi, scoreBoard)
                                      : GoBoardUtil::TrompTaylorScore(bd, komi, scoreBoard);
}

template<class BOARD>
float GoBoardUtil::JapaneseScore(const BOARD& bd, float komi,
                                 GoPointArray<SgEmptyBlackWhite>* scoreBoard) {
  return GoBoardUtil::TrompTaylorScore(bd, komi, scoreBoard);
}

template<class BOARD>
float GoBoardUtil::TrompTaylorScore(const BOARD& bd, float komi,
                                    GoPointArray<SgEmptyBlackWhite>* scoreBoard) {
  float score = -komi;
  SgMarker mark;
  for (typename BOARD::Iterator it(bd); it; ++it) {
    if (mark.Contains(*it))
      continue;
    SgEmptyBlackWhite c = bd.GetColor(*it);
    if (c == SG_BLACK) {
      ++score;
      if (scoreBoard != 0)
        (*scoreBoard)[*it] = SG_BLACK;
      continue;
    }
    if (c == SG_WHITE) {
      --score;
      if (scoreBoard != 0)
        (*scoreBoard)[*it] = SG_WHITE;
      continue;
    }
    SgStack<GoPoint, GO_MAXPOINT> stack;
    GoPointList list;
    DBG_ASSERT(c == SG_EMPTY);
    stack.Push(*it);
    mark.Include(*it);
    list.PushBack(*it);
    SgBWArray<bool> adjacent(false);
    int size = 0;
    while (!stack.IsEmpty()) {
      GoPoint p = stack.Pop();
      DBG_ASSERT(bd.GetColor(p) == SG_EMPTY);
      ++size;
      if (bd.HasNeighbors(p, SG_BLACK))
        adjacent[SG_BLACK] = true;
      if (bd.HasNeighbors(p, SG_WHITE))
        adjacent[SG_WHITE] = true;
      for (GoNb4Iterator<BOARD> it2(bd, p); it2; ++it2)
        if (bd.GetColor(*it2) == SG_EMPTY
            && !mark.Contains(*it2)
            ) {
          stack.Push(*it2);
          mark.Include(*it2);
          list.PushBack(*it2);
        }
    }
    if (adjacent[SG_BLACK] && !adjacent[SG_WHITE]) {
      score += float(size);
      c = SG_BLACK;
    } else if (!adjacent[SG_BLACK] && adjacent[SG_WHITE]) {
      score -= float(size);
      c = SG_WHITE;
    } else
      c = SG_EMPTY;
    if (scoreBoard != 0)
      for (GoPointList::Iterator it2(list); it2; ++it2)
        (*scoreBoard)[*it2] = c;
  }
  return score;
}

template<class BOARD>
std::ostream& GoWriteBoard(std::ostream& out, const BOARD& bd) {
  std::ostringstream buffer;
  GoGrid size = bd.Size();
  if (size > 9)
    buffer << "   ";
  else
    buffer << "  ";
  {
    GoGrid col = 1;
    for (char c = 'A'; col <= size; ++col, ++c) {
      if (c == 'I')
        ++c;
      buffer << c << ' ';
    }
  }
  buffer << '\n';
  for (GoGrid row = size; row >= 1; --row) {
    if (size > 9 && row < 10)
      buffer << ' ';
    buffer << row << ' ';
    for (GoGrid col = 1; col <= size; ++col) {
      GoPoint p = GoPointUtil::Pt(col, row);
      switch (bd.GetColor(p)) {
        case SG_BLACK:buffer << 'X';
          break;
        case SG_WHITE:buffer << 'O';
          break;
        case SG_EMPTY:
          if (GoBoardUtil::IsHandicapPoint(size, col, row))
            buffer << '+';
          else
            buffer << '.';
          break;
        default:DBG_ASSERT(false);
      }
      buffer << ' ';
    }
    buffer << row;
    if (row <= 2) {
      if (size < 10)
        buffer << "  ";
      else
        buffer << "   ";
      if (row == 1)
        buffer << SgBWToString(bd.ToPlay()) << " to play";
    }
    buffer << '\n';
  }
  if (size > 9)
    buffer << "   ";
  else
    buffer << "  ";
  {
    GoGrid col = 1;
    for (char c = 'A'; col <= size; ++col, ++c) {
      if (c == 'I')
        ++c;
      buffer << c << ' ';
    }
  }
  buffer << '\n';
  out << buffer.str();
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const GoBoard& bd) {
  return GoWriteBoard(out, bd);
}

class GoRestoreKoRule {
 public:
  GoRestoreKoRule(GoBoard& board);
  ~GoRestoreKoRule();

 private:
  GoBoard& m_board;
  GoRules::KoRule m_koRule;
  GoRestoreKoRule(const GoRestoreKoRule&);
  GoRestoreKoRule& operator=(const GoRestoreKoRule&);
};

inline GoRestoreKoRule::GoRestoreKoRule(GoBoard& board)
    : m_board(board),
      m_koRule(board.Rules().GetKoRule()) {}

inline GoRestoreKoRule::~GoRestoreKoRule() {
  m_board.Rules().SetKoRule(m_koRule);
}

class GoRestoreToPlay {
 public:
  GoRestoreToPlay(GoBoard& board)
      : m_board(board),
        m_oldToPlay(board.ToPlay()) {}

  ~GoRestoreToPlay() {
    m_board.SetToPlay(m_oldToPlay);
  }

 private:
  GoBoard& m_board;
  SgBlackWhite m_oldToPlay;
  GoRestoreToPlay(const GoRestoreToPlay&);
  GoRestoreToPlay& operator=(const GoRestoreToPlay&);
};
class GoBlockIterator {
 public:
  GoBlockIterator(const GoBoard& board)
      : m_board(board),
        m_p(board) {
    if (!*this)
      ++(*this);
  }

  void operator++() {
    do {
      ++m_p;
    } while (m_p && !*this);
  }

  GoPoint operator*() const {
    DBG_ASSERT(*this);
    return *m_p;
  }

  operator bool() const {
    GoPoint p = *m_p;
    return m_board.Occupied(p)
        && p == m_board.Anchor(p);
  }

 private:
  const GoBoard& m_board;
  GoBoard::Iterator m_p;
  operator int() const;
  GoBlockIterator(const GoBlockIterator&);
  GoBlockIterator& operator=(const GoBlockIterator&);
};
class GoRestoreSuicide {
 public:
  GoRestoreSuicide(GoBoard& board, bool allow)
      : m_board(board),
        m_oldState(board.Rules().AllowSuicide()) {
    m_board.Rules().SetAllowSuicide(allow);
  }

  ~GoRestoreSuicide() {
    m_board.Rules().SetAllowSuicide(m_oldState);
  }

 private:
  GoBoard& m_board;
  bool m_oldState;
  GoRestoreSuicide(const GoRestoreSuicide&);
  GoRestoreSuicide& operator=(const GoRestoreSuicide&);
};
class GoRestoreRepetitionAndSuicide {
 public:
  GoRestoreRepetitionAndSuicide(GoBoard& board, bool allowAnyRepetition,
                                bool allowKoRepetition, bool allowSuicide)
      : m_board(board),
        m_oldAnyRepetition(board.AnyRepetitionAllowed()),
        m_oldKoRepetition(board.KoRepetitionAllowed()),
        m_oldSuicide(board.Rules().AllowSuicide()) {
    m_board.AllowAnyRepetition(allowAnyRepetition);
    m_board.AllowKoRepetition(allowKoRepetition);
    m_board.Rules().SetAllowSuicide(allowSuicide);
  }

  ~GoRestoreRepetitionAndSuicide() {
    m_board.AllowAnyRepetition(m_oldAnyRepetition);
    m_board.AllowKoRepetition(m_oldKoRepetition);
    m_board.Rules().SetAllowSuicide(m_oldSuicide);
  }

 private:
  GoBoard& m_board;
  bool m_oldAnyRepetition;
  bool m_oldKoRepetition;
  bool m_oldSuicide;
  GoRestoreRepetitionAndSuicide(const GoRestoreRepetitionAndSuicide&);
  GoRestoreRepetitionAndSuicide&
  operator=(const GoRestoreRepetitionAndSuicide&);
};
class GoNeighborBlockIterator
    : public GoPointIterator {
 public:
  GoNeighborBlockIterator(const GoBoard& board, GoPoint p, SgBlackWhite c)
      : GoPointIterator(m_points) {
    board.NeighborBlocks(p, c, m_points);
  }

  GoNeighborBlockIterator(const GoBoard& board, GoPoint p, SgBlackWhite c,
                          int maxLib)
      : GoPointIterator(m_points) {
    board.NeighborBlocks(p, c, maxLib, m_points);
  }

 private:

  GoPoint m_points[5];
};
static const int MAX_ADJACENT = (GO_MAX_SIZE + 1) * (GO_MAX_SIZE + 1) / 4;
template<class BOARD>
class GoAdjBlockIterator
    : public GoPointIterator {
 public:
  GoAdjBlockIterator(const BOARD& board, GoPoint p, int maxLib);

 private:

  GoPoint m_points[MAX_ADJACENT];
};

template<class BOARD>
GoAdjBlockIterator<BOARD>::GoAdjBlockIterator(const BOARD& board,
                                              GoPoint p, int maxLib)
    : GoPointIterator(m_points) {
  board.AdjacentBlocks(p, maxLib, m_points, MAX_ADJACENT);
}

namespace GoBoardWrite {

class WriteMap {
 public:
  WriteMap(const GoBoard& bd, const GoPointSet& points)
      : m_bd(bd),
        m_points(points) {}

  const GoBoard& Board() const {
    return m_bd;
  }

  const GoPointSet& Points() const {
    return m_points;
  }

 private:
  const GoBoard& m_bd;
  const GoPointSet& m_points;
};

}
std::ostream& operator<<(std::ostream& out, const GoBoardWrite::WriteMap& w);

#endif
