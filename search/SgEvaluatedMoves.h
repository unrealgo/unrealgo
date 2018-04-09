
#ifndef SG_EVALUATEDMOVES_H
#define SG_EVALUATEDMOVES_H

#include "board/GoPointSet.h"
#include "lib/SgVector.h"


class SgEvaluatedMoves {
 public:
  explicit SgEvaluatedMoves(const GoPointSet &relevant)
      : m_bestValue(s_minValue),
        m_relevant(relevant) {}

  explicit SgEvaluatedMoves(const SgEvaluatedMoves &original)
      : m_bestValue(original.m_bestValue),
        m_moveList(original.m_moveList),
        m_relevant(original.m_relevant) {}

  virtual ~SgEvaluatedMoves() {}

  virtual void AddMove(GoPoint move, int value);
  virtual void AddMoves(const GoPointSet &moves, int value);
  virtual void AddMoves(const SgVector<GoPoint> &moves, int value);

  virtual void Clear() {
    m_bestValue = s_minValue;
    m_moveList.Clear();
  }

  GoPoint BestMove();

  int BestValue() {
    return m_bestValue;
  }

  const GoPointSet &Relevant() const {
    return m_relevant;
  }

  bool IsRelevant(GoPoint p) const {
    return m_relevant[p];
  }

  void Disable(GoPoint p) {
    m_relevant.Exclude(p);
  }

  void Enable(GoPoint p) {
    m_relevant.Include(p);
  }

  virtual SgEvaluatedMoves *Duplicate() const {
    return new SgEvaluatedMoves(*this);
  }

  virtual int GetEvaluation(GoPoint p) const {
    if (m_moveList.Contains(p))
      return m_bestValue;
    else
      return 0;
  }


  virtual void BestMoves(SgVector<GoPoint> &best, int nuMoves) const;

 protected:
  int m_bestValue;
  SgVector<GoPoint> m_moveList;
  GoPointSet m_relevant;
  // DS: INT_MIN is sometimes used to mark illegal moves
  static const int s_minValue = INT_MIN + 1;
};



class SgEvaluatedMovesArray
    : public SgEvaluatedMoves {
 public:
  explicit SgEvaluatedMovesArray(const GoPointSet &relevant,
                                 int boardSize = GO_MAX_SIZE);

  virtual ~SgEvaluatedMovesArray() {}

  SgEvaluatedMovesArray(const SgEvaluatedMovesArray &original)
      : SgEvaluatedMoves(original),
        m_boardSize(original.m_boardSize) {
    for (int i = 0; i < GO_MAXPOINT; ++i)
      m_value[i] = original.m_value[i];
  }

  virtual void AddMove(GoPoint move, int value);
  virtual void ReduceMove(GoPoint move, int value);
  virtual void Clear();
  void Write() const;

  virtual SgEvaluatedMoves *Duplicate() const {
    return new SgEvaluatedMovesArray(*this);
  }

  virtual int GetEvaluation(GoPoint p) const {
    return m_value[p];
  }

  virtual void BestMoves(SgVector<GoPoint> &best, int nuMoves) const;

 private:
  int m_value[GO_MAXPOINT];
  int m_boardSize;
  GoPoint SelectNextBest(SgVector<GoPoint> &bestSoFar) const;
};

#endif // SG_EVALUATEDMOVES_H
