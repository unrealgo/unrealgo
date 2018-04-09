
#ifndef GOUCT_UTIL_H
#define GOUCT_UTIL_H

#include <iosfwd>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoEyeUtil.h"
#include "GoModBoard.h"
#include "GoUctBoard.h"
#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "lib/SgRandom.h"
#include "UctSearch.h"
#include "board/SgUtil.h"

class GoBWSet;

template<typename T, int N>
class GoArrayList;

namespace GoUctUtil {
const bool REMOVE_SELF_ATARI = false;
const bool REMOVE_MUTUAL_ATARI = true;
const int SELF_ATARI_LIMIT = 8;
const int MUTUAL_ATARI_LIMIT = 2;
const bool CONSERVATIVE_CLUMP = true;
const int LINE_1_LIMIT = CONSERVATIVE_CLUMP ? 4 : 3;
const int LINE_2_OR_MORE_LIMIT = CONSERVATIVE_CLUMP ? 6 : 5;
void ClearStatistics(GoPointArray<UctStatistics> &stats);
template<class BOARD>
bool DoClumpCorrection(const BOARD &bd, GoPoint &move);
template<class BOARD>
bool DoFalseEyeToCaptureCorrection(const BOARD &bd, GoPoint &move);
template<class BOARD>
bool DoSelfAtariCorrection(const BOARD &bd, GoPoint &p);
GoPoint GenForcedOpeningMove(const GoBoard &bd);
template<class BOARD>
bool GeneratePoint(const BOARD &bd, GoPoint p,
                   SgBlackWhite toPlay);
void GfxBestMove(const UctSearch &search, SgBlackWhite toPlay,
                 std::ostream &out);
void GfxCounts(const UctSearchTree &tree, std::ostream &out);
void GfxMoveValues(const UctSearch &search, SgBlackWhite toPlay,
                   std::ostream &out);
void GfxSequence(const UctSearch &search, SgBlackWhite toPlay,
                 std::ostream &out);
void GfxStatus(const UctSearch &search, std::ostream &out);
void GfxTerritoryStatistics(
    const GoPointArray<UctStatistics> &territoryStatistics,
    const GoBoard &bd, std::ostream &out);
template<class BOARD>
bool IsMutualAtari(const BOARD &bd, GoPoint p, SgBlackWhite toPlay);
void SaveTree(const UctSearchTree &tree, int boardSize, const GoBWSet &stones,
              SgBlackWhite toPlay, std::ostream &out, int maxDepth = -1);
template<class BOARD>
GoPoint SelectRandom(const BOARD &bd, SgBlackWhite toPlay,
                     GoPointList &emptyPts,
                     SgRandom &random);
template<class BOARD>
void SetEdgeCorrection(const BOARD &bd, GoPoint p, int &edgeCorrection);
std::string ChildrenStatistics(const UctSearch &search,
                               bool bSort, const UctNode &node);
template<class BOARD>
bool SubsetOfBlocks(const BOARD &bd, const GoPoint anchor[], GoPoint nb);
}

template<class BOARD>
bool GoUctUtil::DoClumpCorrection(const BOARD &bd, GoPoint &move) {
  if (bd.NumEmptyNeighbors(move) != 1)
    return false;
  const SgBlackWhite toPlay = bd.ToPlay();
  if (bd.Line(move) == 1) {
    if (bd.Num8Neighbors(move, toPlay) < LINE_1_LIMIT
        || bd.NumNeighbors(move, toPlay) != 2
        )
      return false;
  } else if (bd.Num8Neighbors(move, toPlay) < LINE_2_OR_MORE_LIMIT
      || bd.NumNeighbors(move, toPlay) != 3
      )
    return false;

  const GoPoint nb = GoEyeUtil::EmptyNeighbor(bd, move);
  int edgeCorrection_move = 0;
  int edgeCorrection_nb = 0;
  SetEdgeCorrection(bd, move, edgeCorrection_move);
  SetEdgeCorrection(bd, nb, edgeCorrection_nb);
  if (bd.Num8Neighbors(nb, toPlay) + edgeCorrection_nb <
      bd.Num8Neighbors(move, toPlay) + edgeCorrection_move
      && bd.NumNeighbors(nb, toPlay) <= bd.NumNeighbors(move, toPlay)
      &&
          (bd.NumEmptyNeighbors(nb) >= 2
              || !GoBoardUtil::SelfAtari(bd, nb)
          )
      ) {
    if (CONSERVATIVE_CLUMP) // no further tests, nb is assumed to be good
    {
      move = nb;
      return true;
    } else {
      GoPoint anchor[4 + 1];
      bd.NeighborBlocks(move, toPlay, anchor);
      DBG_ASSERT(anchor[0] != GO_ENDPOINT); // at least 1 block
      if (anchor[1] == GO_ENDPOINT // no connection, only 1 block
          || SubsetOfBlocks<BOARD>(bd, anchor, nb)
          ) {
        move = nb;
        return true;
      }
    }
  }
  return false;
}

template<class BOARD>
inline bool GoUctUtil::DoFalseEyeToCaptureCorrection(const BOARD &bd,
                                                     GoPoint &move) {
  DBG_ASSERT(bd.IsEmpty(move));
  const SgBlackWhite opp = bd.Opponent();
  if (bd.HasEmptyNeighbors(move) || bd.HasNeighbors(move, opp))
    return false;
  if (bd.Line(move) == 1) {
    if (bd.NumDiagonals(move, SG_EMPTY) > 0
        || bd.NumDiagonals(move, opp) > 1
        )
      return false;
  } else if (bd.NumDiagonals(move, SG_EMPTY) > 1
      || bd.NumDiagonals(move, opp) > 2
      )
    return false;

  for (SgNb4DiagIterator it(move); it; ++it) {
    const GoPoint p = *it;
    if (bd.GetColor(p) == opp && bd.InAtari(p)) // try to capture p
    {
      const GoPoint lib = bd.TheLiberty(p);
      if (bd.IsLegal(lib)) {
        move = lib;
        return true;
      }
    }
  }

  return false;
}

template<class BOARD>
inline bool GoUctUtil::DoSelfAtariCorrection(const BOARD &bd, GoPoint &move) {
  const SgBlackWhite toPlay = bd.ToPlay();
  if (bd.NumEmptyNeighbors(move) >= 2)
    return false;
  if (bd.NumNeighbors(move, toPlay) > 0) // move part of existing block(s)
  {
    if (!GoBoardUtil::SelfAtari(bd, move))
      return false;
    SgBlackWhite opp = SgOppBW(toPlay);
    GoPoint replaceMove = GO_NULLMOVE;
    for (GoNb4Iterator<BOARD> it(bd, move); it; ++it) {
      SgBoardColor c = bd.GetColor(*it);
      if (c == SG_EMPTY)
        replaceMove = *it;
      else if (c == toPlay) {
        for (typename BOARD::LibertyIterator it2(bd, *it); it2; ++it2)
          if (*it2 != move) {
            replaceMove = *it2;
            break;
          }
      } else if (c == opp && bd.InAtari(*it))
        replaceMove = *it;
      if (replaceMove != GO_NULLMOVE)
        break;
    }
    DBG_ASSERT(replaceMove != GO_NULLMOVE);
    if (bd.IsLegal(replaceMove)
        && !GoBoardUtil::SelfAtari(bd, replaceMove)
        ) {
      move = replaceMove;
      return true;
    }
  } else if (bd.NumEmptyNeighbors(move) > 0 && !bd.CanCapture(move, toPlay)) {
    const GoPoint nb = GoEyeUtil::EmptyNeighbor(bd, move);
    if (bd.IsLegal(nb)
        && (bd.NumEmptyNeighbors(nb) >= 2
            || bd.CanCapture(nb, toPlay)
        )
        ) {
      move = nb;
      return true;
    }
  }
  return false;
}

template<class BOARD>
inline bool GoUctUtil::IsMutualAtari(const BOARD &bd,
                                     GoPoint p, SgBlackWhite toPlay) {
  int nuStones = 0;
  if (GoBoardUtil::SelfAtari(bd, p, nuStones)
      && nuStones > MUTUAL_ATARI_LIMIT
      && (nuStones > GoEyeUtil::NAKADE_LIMIT
          || !GoEyeUtil::MakesNakadeShape(bd, p, toPlay)
      )
      ) {
    DBG_ASSERT(bd.ToPlay() == toPlay);
    SgBlackWhite opp = SgOppBW(toPlay);
    bool selfatari =
        bd.HasNeighbors(p, opp) &&
            GoBoardUtil::SelfAtariForColor(bd, p, opp);
    if (selfatari)
      return true;
  }
  return false;
}

template<class BOARD>
inline bool GoUctUtil::GeneratePoint(const BOARD &bd,
                                     GoPoint p, SgBlackWhite toPlay) {
  DBG_ASSERT(bd.IsEmpty(p));
  DBG_ASSERT(bd.ToPlay() == toPlay);
  if (GoBoardUtil::IsCompletelySurrounded(bd, p)
      //|| GoEyeUtil::IsTwoPointEye(bd, p, to_play)
      || !bd.IsLegal(p, toPlay)
      )
    return false;
  if (REMOVE_SELF_ATARI) {
    int nuStones = 0;
    if (GoBoardUtil::SelfAtari(bd, p, nuStones)
        && nuStones > SELF_ATARI_LIMIT
      // todo: check for nakade shapes here.
        ) {
      return false;
    }
  }

  if (REMOVE_MUTUAL_ATARI && IsMutualAtari(bd, p, toPlay))
    return false;
  return true;
}

template<class BOARD>
inline GoPoint GoUctUtil::SelectRandom(const BOARD &bd,
                                       SgBlackWhite toPlay,
                                       GoPointList &emptyPts,
                                       SgRandom &random) {
  for (;;) {
    int length = emptyPts.Length();
    if (length == 0)
      break;
    int index = random.SmallInt(length);
    GoPoint p = emptyPts[index];
    DBG_ASSERT(bd.IsEmpty(p));
    if (GeneratePoint(bd, p, toPlay))
      return p;
    emptyPts[index] = emptyPts[length - 1];
    emptyPts.PopBack();
  }
  return GO_NULLMOVE;
}

template<class BOARD>
void GoUctUtil::SetEdgeCorrection(const BOARD &bd, GoPoint p,
                                  int &edgeCorrection) {
  if (bd.Line(p) == 1) {
    edgeCorrection += 3;
    if (bd.Pos(p) == 1)
      edgeCorrection += 2;
  }
}

template<class BOARD>
bool GoUctUtil::SubsetOfBlocks(const BOARD &bd, const GoPoint anchor[],
                               GoPoint nb) {
  GoPoint nbanchor[4 + 1];
  bd.NeighborBlocks(nb, bd.ToPlay(), nbanchor);
  if (nbanchor[0] == GO_ENDPOINT)
    return false;
  for (int i = 0; anchor[i] != GO_ENDPOINT; ++i)
    if (!GoBoardUtil::ContainsAnchor(nbanchor, anchor[i]))
      return false;
  return true;
}

#endif // GOUCT_UTIL_H
