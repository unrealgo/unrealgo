
#include "platform/SgSystem.h"
#include "SgEvaluatedMoves.h"

#include <iomanip>
#include "platform/SgDebug.h"
#include "lib/SgRandom.h"
#include "board/SgWrite.h"

using namespace std;


void SgEvaluatedMoves::AddMove(GoPoint move, int value) {
  if (GoPointUtil::InBoardRange(move) && m_relevant[move]) {
    if (value > m_bestValue) {
      m_bestValue = value;
      m_moveList.Clear();
    }
    if (value >= m_bestValue)
      m_moveList.PushBack(move);
  }
}

void SgEvaluatedMoves::AddMoves(const GoPointSet &moves, int value) {
  for (SgSetIterator it(moves); it; ++it)
    AddMove(*it, value);
}

void SgEvaluatedMoves::AddMoves(const SgVector<GoPoint> &moves, int value) {
  for (SgVectorIterator<GoPoint> it(moves); it; ++it)
    AddMove(*it, value);
}

GoPoint SgEvaluatedMoves::BestMove() {
  if (m_moveList.IsEmpty())
    return GO_PASS;
  else
    return m_moveList[0];
}

void SgEvaluatedMoves::BestMoves(SgVector<GoPoint> &best, int nuMoves) const {
  SuppressUnused(nuMoves);
  best = m_moveList;
}


SgEvaluatedMovesArray::SgEvaluatedMovesArray(const GoPointSet &relevant,
                                             int boardSize)
    : SgEvaluatedMoves(relevant),
      m_boardSize(boardSize) {
  Clear();
}

void SgEvaluatedMovesArray::AddMove(GoPoint move, int value) {
  if (GoPointUtil::InBoardRange(move) && m_relevant[move]) {
    m_value[move] += value;
    SgEvaluatedMoves::AddMove(move, m_value[move]);
  }
}

void SgEvaluatedMovesArray::ReduceMove(GoPoint move, int value) {
  if (GoPointUtil::InBoardRange(move) && m_relevant[move]) {
    m_value[move] -= value;
    SgEvaluatedMoves::AddMove(move, m_value[move]);
  }
}

GoPoint SgEvaluatedMovesArray::SelectNextBest(SgVector<GoPoint> &bestSoFar)
const {
  int bestValue = s_minValue;
  GoPoint best = 0;
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    if ((m_value[p] > bestValue) && !bestSoFar.Contains(p)) {
      bestValue = m_value[p];
      best = p;
    }
  }
  return best;
}

void SgEvaluatedMovesArray::BestMoves(SgVector<GoPoint> &best, int nuMoves)
const {
  best.Clear();
  while (--nuMoves >= 0) {
    int nextBest = SelectNextBest(best);
    best.PushBack(nextBest);
  }
}

void SgEvaluatedMovesArray::Write() const {
  int i, j;
  SgDebug() << "      ";
  for (j = 1; j <= m_boardSize; ++j) {
    SgDebug() << GoPointUtil::Letter(j) << "    ";
  }

  for (i = 1; i <= m_boardSize; ++i) {
    SgDebug() << '\n' << setw(2) << i;
    for (j = 1; j <= m_boardSize; ++j)
      SgDebug() << setw(5) << m_value[GoPointUtil::Pt(j, i)];
    SgDebug() << '\n';
  }
}

void SgEvaluatedMovesArray::Clear() {
  memset(m_value, 0, GO_MAXPOINT * sizeof(m_value[0]));
}

