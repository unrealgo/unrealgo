
#ifndef GOUCTGAMMAMOVEGENERATOR_H
#define GOUCTGAMMAMOVEGENERATOR_H

#include <iostream>
#include "GoBoardUtil.h"
#include "GoUctUtil.h"
#include "board/SgWrite.h"

template<class BOARD>
class GoUctGammaMoveGenerator {
 public:
  GoUctGammaMoveGenerator(const BOARD &bd,
                          float patternGammaThreshold,
                          SgRandom &randomGenerator);
  /** Generate move with probability according to gamma values */
  GoPoint GenerateBiasedPatternMove();
  float GetGammaValueForMoveAt(const int index) const;
  GoPoint GetGammaMoveAt(const int index) const;
  int NuMoves() const;
  void WriteMovesAndGammas(std::ostream &stream, bool writeGammas) const;

 private:
  /** Generate pattern moves with gamma values */
  bool GenerateAllPatternMoves();
  /** Generate pattern move on point p (near last move) */
  void GeneratePatternMove(GoPoint p);
  /** Generate pattern move on point p (near 2nd last move) */
  void GeneratePatternMove2(GoPoint p, GoPoint lastMove);
  GoPoint SelectAccordingToGammas();
  const BOARD &m_bd;
  float m_patternGammaThreshold;
  /** Copy of GoUctPlayoutPolicy::m_moves */
  GoPointList m_moves;
  SgRandom &m_random;
  /** gamma values for each pattern move */
  GoArrayList<float, GO_MAX_ONBOARD + 1> m_gammas;
  /** partial sums of gamma values.
      m_gammasSums[i] = sum(m_gammas[0....i]) */
  GoArrayList<float, GO_MAX_ONBOARD + 1> m_gammaSums;
  /** subset of m_moves, filtered by GoUctUtil::GeneratePoint */
  GoPointList m_movesGammas;
};

template<class BOARD>
inline GoPoint GoUctGammaMoveGenerator<BOARD>
::GetGammaMoveAt(const int index) const {
  DBG_ASSERT(index < m_movesGammas.Length());
  return m_movesGammas[index];
}

template<class BOARD>
inline float GoUctGammaMoveGenerator<BOARD>
::GetGammaValueForMoveAt(const int index) const {
  if (m_gammaSums.Length() == 0)
    return 0;
  DBG_ASSERT(index < m_gammaSums.Length());
  return index == 0 ?
         m_gammaSums[0] :
         m_gammaSums[index] - m_gammaSums[index - 1];
}

template<class BOARD>
inline int GoUctGammaMoveGenerator<BOARD>::NuMoves() const {
  return m_movesGammas.Length();
}

template<class BOARD>
GoUctGammaMoveGenerator<BOARD>::GoUctGammaMoveGenerator(
    const BOARD &bd,
    float patternGammaThreshold,
    SgRandom &random)
    : m_bd(bd),
      m_patternGammaThreshold(patternGammaThreshold),
      m_random(random) {}

template<class BOARD>
bool GoUctGammaMoveGenerator<BOARD>::GenerateAllPatternMoves() {
  DBG_ASSERT(m_moves.IsEmpty());
  GoPoint lastMove = m_bd.GetLastMove();
  DBG_ASSERT(!SgIsSpecialMove(lastMove));
  GeneratePatternMove(lastMove + GO_NORTH_SOUTH - GO_WEST_EAST);
  GeneratePatternMove(lastMove + GO_NORTH_SOUTH);
  GeneratePatternMove(lastMove + GO_NORTH_SOUTH + GO_WEST_EAST);
  GeneratePatternMove(lastMove - GO_WEST_EAST);
  GeneratePatternMove(lastMove + GO_WEST_EAST);
  GeneratePatternMove(lastMove - GO_NORTH_SOUTH - GO_WEST_EAST);
  GeneratePatternMove(lastMove - GO_NORTH_SOUTH);
  GeneratePatternMove(lastMove - GO_NORTH_SOUTH + GO_WEST_EAST);

  const GoPoint lastMove2 = m_bd.Get2ndLastMove();
  if (!SgIsSpecialMove(lastMove2)) {
    GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH - GO_WEST_EAST, lastMove);
    GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH, lastMove);
    GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH + GO_WEST_EAST, lastMove);
    GeneratePatternMove2(lastMove2 - GO_WEST_EAST, lastMove);
    GeneratePatternMove2(lastMove2 + GO_WEST_EAST, lastMove);
    GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH - GO_WEST_EAST, lastMove);
    GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH, lastMove);
    GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH + GO_WEST_EAST, lastMove);
  }

  m_movesGammas.Clear();
  m_gammaSums.Clear();
  float sum = 0;

  for (int i = 0; i < m_moves.Length(); ++i) {
    const GoPoint p = m_moves[i];
    if (!GoUctUtil::GeneratePoint(m_bd, p, m_bd.ToPlay()))
      continue;
    m_movesGammas.PushBack(p);
    if (m_gammas[i] > m_patternGammaThreshold)
      sum += m_gammas[i];
    else
      sum += 1.f;
    m_gammaSums.PushBack(sum);
  }

  if (m_movesGammas.Length() == 0)
    m_moves.Clear();
  return !m_moves.IsEmpty();
}

template<class BOARD>
inline void GoUctGammaMoveGenerator<BOARD>::GeneratePatternMove(GoPoint p) {
  float gamma = 0;
  if (m_bd.IsEmpty(p)
      && !GoBoardUtil::SelfAtari(m_bd, p)
      ) {
    m_moves.PushBack(p);
    m_gammas.PushBack(gamma);
  }
}

template<class BOARD>
inline void GoUctGammaMoveGenerator<BOARD>::GeneratePatternMove2(GoPoint p,
                                                                 GoPoint lastMove) {
  float gamma = 0;
  if (m_bd.IsEmpty(p)
      && !GoPointUtil::In8Neighborhood(lastMove, p)
      && !GoBoardUtil::SelfAtari(m_bd, p)) {
    m_moves.PushBack(p);
    m_gammas.PushBack(gamma);
  }
}

template<class BOARD>
inline GoPoint GoUctGammaMoveGenerator<BOARD>::GenerateBiasedPatternMove() {
  m_moves.Clear();
  m_gammas.Clear();
  m_gammaSums.Clear();
  m_movesGammas.Clear();
  if (GenerateAllPatternMoves())
    return SelectAccordingToGammas();
  return GO_NULLPOINT;
}

template<class BOARD>
inline GoPoint GoUctGammaMoveGenerator<BOARD>::SelectAccordingToGammas() {
  GoArrayList<float, GO_MAX_ONBOARD + 1> &gammas = m_gammaSums;
  DBG_ASSERT(gammas.Length() == m_movesGammas.Length());
  DBG_ASSERT(gammas.Length() > 0);
  float randNum = m_random.Float(gammas.Last());
  for (int i = 0; i < gammas.Length(); ++i)
    if (randNum <= gammas[i])
      return m_movesGammas[i];

  DBG_ASSERT(false);
  return GO_NULLPOINT;
}

template<class BOARD>
std::ostream &operator<<(std::ostream &stream,
                         const GoUctGammaMoveGenerator<BOARD> &gen) {
  for (int i = 0; i < gen.NuMoves(); ++i)
    stream << ' ' << GoWritePoint(gen.GetGammaMoveAt(i));
  return stream;
}

template<class BOARD>
void GoUctGammaMoveGenerator<BOARD>::
WriteMovesAndGammas(std::ostream &stream, bool writeGammas) const {
  for (int i = 0; i < NuMoves(); ++i) {
    stream << GoWritePoint(GetGammaMoveAt(i));
    if (writeGammas)
      stream << ":"
             << GetGammaValueForMoveAt(i);
    stream << ' ';
  }
  stream << std::endl;
}

#endif
