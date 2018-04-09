#include "platform/SgSystem.h"
#include "GoUctDefaultMoveFilter.h"

#include "GoBensonSolver.h"
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoModBoard.h"
#include "GoSafetySolver.h"
#include "board/SgWrite.h"
namespace {
inline bool IsEmpty2x3Box(const GoBoard &bd, GoPoint p) {
  DBG_ASSERT (bd.Line(p) == 1);
  DBG_ASSERT (bd.Pos(p) > 1);
  return bd.IsEmpty(p) && bd.Num8EmptyNeighbors(p) == 5;
}

inline bool IsEmptyOrInCorner(const GoBoard &bd, GoPoint p, int direction) {
  return bd.Pos(p + direction) == 1
      || IsEmpty2x3Box(bd, p + direction);
}

bool IsEmptyEdge(const GoBoard &bd, GoPoint p) {
  DBG_ASSERT (bd.IsEmpty(p));
  DBG_ASSERT (bd.Line(p) == 1);
  if (bd.Num8EmptyNeighbors(p) < 5 && bd.Pos(p) > 1)
    return false;
  const GoPoint pUp = p + bd.Up(p);
  DBG_ASSERT(bd.Line(pUp) == 2);
  DBG_ASSERT(bd.Pos(pUp) >= 2);
  if (bd.Num8EmptyNeighbors(pUp) < 8)
    return false;

  switch (bd.Pos(p)) {
    case 1:
    case 2:
    case 3:
      return IsEmptyOrInCorner(bd, p, bd.Left(p))
          && IsEmptyOrInCorner(bd, p, bd.Right(p));
    default:
      return IsEmpty2x3Box(bd, p + 2 * bd.Left(p))
          && IsEmpty2x3Box(bd, p + 2 * bd.Right(p));
  }
}

bool LibertiesAreDiagonal(const GoBoard &bd, GoPoint anchor) {
  DBG_ASSERT(bd.NumLiberties(anchor) == 2);
  GoBoard::LibertyIterator it(bd, anchor);
  GoPoint lib1 = *it;
  ++it;
  DBG_ASSERT(it);
  return GoPointUtil::AreDiagonal(lib1, *it);
}

}

GoUctDefaultMoveFilterParam::GoUctDefaultMoveFilterParam()
    : m_checkLadders(true),
      m_checkOffensiveLadders(true),
      m_minLadderLength(6),
      m_filterFirstLine(true),
      m_checkSafety(true) {}

GoUctDefaultMoveFilter::GoUctDefaultMoveFilter(const GoBoard &bd, const GoUctDefaultMoveFilterParam &param)
    : m_bd(bd),
      m_param(param) {}

std::vector<GoPoint> GoUctDefaultMoveFilter::Get() {
  std::vector<GoPoint> rootFilter;
  const SgBlackWhite toPlay = m_bd.ToPlay();
  const SgBlackWhite opp = SgOppBW(toPlay);
  if (m_param.m_checkSafety) {
    GoBWSet alternateSafe;
    GoSafetySolver safetySolver(m_bd);
    safetySolver.FindSafePoints(&alternateSafe);
    GoBensonSolver bensonSolver(m_bd);
    GoBWSet unconditionalSafe;
    bensonSolver.FindSafePoints(&unconditionalSafe);

    for (GoBoard::Iterator it(m_bd); it; ++it) {
      const GoPoint p = *it;
      if (m_bd.IsLegal(p)) {
        bool isUnconditionalSafe = unconditionalSafe[toPlay].Contains(p);
        bool isUnconditionalSafeOpp = unconditionalSafe[opp].Contains(p);
        bool isAlternateSafeOpp = alternateSafe[opp].Contains(p);
        bool hasOppNeighbors = m_bd.HasNeighbors(p, opp);
        if (isAlternateSafeOpp
            || isUnconditionalSafeOpp
            || (isUnconditionalSafe && !hasOppNeighbors)
            || (alternateSafe[toPlay].Contains(p)
                && !safetySolver.PotentialCaptureMove(p, toPlay)
            )
            )
          rootFilter.push_back(p);
      }
    }
  }
  if (m_param.m_checkLadders) {
    for (GoBlockIterator it(m_bd); it; ++it) {
      const GoPoint p = *it;
      if (m_bd.GetStone(p) == toPlay && m_bd.InAtari(p)) {
        if (m_ladder.Ladder(m_bd, p, toPlay, &m_ladderSequence,
                            false) < 0) {
          if (m_ladderSequence.Length() >= m_param.m_minLadderLength)
            rootFilter.push_back(m_bd.TheLiberty(p));
        }
      }

    }
  }

  if (m_param.m_checkOffensiveLadders) {
    for (GoBlockIterator it(m_bd); it; ++it) {
      const GoPoint p = *it;
      if (m_bd.GetStone(p) == opp
          && m_bd.NumStones(p) >= 5
          && m_bd.NumLiberties(p) == 2
          && LibertiesAreDiagonal(m_bd, p)
          && m_ladder.Ladder(m_bd, p, toPlay, &m_ladderSequence,
                             false) > 0
          && m_ladderSequence.Length() >= m_param.m_minLadderLength
          )
        rootFilter.push_back(m_ladderSequence[0]);
    }
  }

  if (m_param.m_filterFirstLine) {
    const GoBoardConst &bc = m_bd.BoardConst();
    for (SgLineIterator it(bc, 1); it; ++it) {
      const GoPoint p = *it;
      if (m_bd.IsEmpty(p) && IsEmptyEdge(m_bd, p))
        rootFilter.push_back(p);
    }
  }

  return rootFilter;
}
