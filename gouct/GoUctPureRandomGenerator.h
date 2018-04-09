
#ifndef GOUCT_PURERANDOMGENERATOR_H
#define GOUCT_PURERANDOMGENERATOR_H

#include <vector>
#include "GoBoard.h"
#include "GoUctUtil.h"
#include "board/SgWrite.h"

template<class BOARD>
class GoUctPureRandomGenerator {
 public:
  GoUctPureRandomGenerator(const BOARD &bd, SgRandom &random);
  void Start();
  void OnPlay();
  const std::vector<GoPoint> &Candidates() const;
  GoPoint Generate();
  GoPoint GenerateRawPoint() const;
  GoPoint GenerateFillboardMove(int numberTries);

 private:
  const BOARD &m_bd;
  float m_invNuPoints;
  float m_nuEmptyFloat;
  SgRandom &m_random;
  std::vector<GoPoint> m_candidates;
  bool Empty3x3(GoPoint p) const;
  void CheckConsistency() const;
  void Insert(GoPoint p);
};

template<class BOARD>
GoUctPureRandomGenerator<BOARD>::GoUctPureRandomGenerator(const BOARD &bd,
                                                          SgRandom &random)
    : m_bd(bd),
      m_random(random) {
  m_candidates.reserve(GO_MAX_NUM_MOVES);
}

template<class BOARD>
inline const std::vector<GoPoint> &
GoUctPureRandomGenerator<BOARD>::Candidates()
const {
  return m_candidates;
}

template<class BOARD>
inline void GoUctPureRandomGenerator<BOARD>::CheckConsistency() const {
#if 0 // Expensive check, enable only for debugging
  for (GoBoard::Iterator it(m_bd); it; ++it)
  {
      GoPoint p = *it;
      if (m_bd.IsEmpty(p))
          if (find(m_candidates.begin(), m_candidates.end(), p)
              == m_candidates.end())
          {
              SgDebug() << m_bd
                        << "Candidates: " << SgWritePointList(m_candidates)
                        << "does not contain: " << GoWritePoint(p)
                        << "\nm_bd.CapturedStones(): "
                        << SgWriteSPointList<GO_MAX_ONBOARD + 1>
                                                 (m_bd.CapturedStones())
                        << "Last move: "
                        << GoWritePoint(m_bd.GetLastMove()) << '\n';
              DBG_ASSERT(false);
          }
  }
#endif
}

template<class BOARD>
inline bool GoUctPureRandomGenerator<BOARD>::Empty3x3(GoPoint p)
const {
  return (m_bd.NumEmptyNeighbors(p) == 4
      && m_bd.NumEmptyDiagonals(p) == 4);
}

template<class BOARD>
inline GoPoint GoUctPureRandomGenerator<BOARD>::GenerateRawPoint() const {
  if (m_candidates.empty())
    return GO_NULLMOVE;
  return m_candidates.back();
}

template<class BOARD>
inline GoPoint GoUctPureRandomGenerator<BOARD>::Generate() {
  CheckConsistency();
  SgBlackWhite toPlay = m_bd.ToPlay();
  size_t i = m_candidates.size();
  while (true) {
    if (i == 0)
      break;
    --i;
    GoPoint p = m_candidates[i];
    if (!m_bd.IsEmpty(p)) {
      m_candidates[i] = m_candidates[m_candidates.size() - 1];
      m_candidates.pop_back();
      continue;
    }
    if (GoUctUtil::GeneratePoint(m_bd, p, toPlay)) {
      CheckConsistency();
      return p;
    }
  }
  CheckConsistency();
  return GO_NULLMOVE;
}

template<class BOARD>
inline GoPoint GoUctPureRandomGenerator<BOARD>::GenerateFillboardMove(
    int numberTries) {
  float effectiveTries
      = float(numberTries) * m_nuEmptyFloat * m_invNuPoints;
  size_t i = m_candidates.size();
  while (effectiveTries > 1.f) {
    if (i == 0)
      return GO_NULLMOVE;
    --i;
    GoPoint p = m_candidates[i];
    if (!m_bd.IsEmpty(p)) {
      m_candidates[i] = m_candidates[m_candidates.size() - 1];
      m_candidates.pop_back();
      continue;
    }
    if (Empty3x3(p))
      return p;
    effectiveTries -= 1.f;
  }
  if (m_random.SmallInt(100) > 100 * effectiveTries)
    return GO_NULLMOVE;
  while (true) {
    if (i == 0)
      break;
    --i;
    GoPoint p = m_candidates[i];
    if (!m_bd.IsEmpty(p)) {
      m_candidates[i] = m_candidates[m_candidates.size() - 1];
      m_candidates.pop_back();
      continue;
    }
    if (Empty3x3(p))
      return p;
    break;
  }
  return GO_NULLMOVE;
}

template<class BOARD>
inline void GoUctPureRandomGenerator<BOARD>::OnPlay() {
  GoPoint lastMove = m_bd.GetLastMove();
  if (lastMove != GO_NULLMOVE && lastMove != GO_PASS
      && !m_bd.IsEmpty(lastMove))
    m_nuEmptyFloat -= 1.f;
  const GoPointList &capturedStones = m_bd.CapturedStones();
  if (!capturedStones.IsEmpty()) {
    for (GoPointList::Iterator it(capturedStones); it; ++it)
      Insert(*it);
    m_nuEmptyFloat += float(capturedStones.Length());
  }
  CheckConsistency();
}

template<class BOARD>
inline void GoUctPureRandomGenerator<BOARD>::Insert(GoPoint p) {
  size_t size = m_candidates.size();
  if (size == 0)
    m_candidates.push_back(p);
  else {
    GoPoint &swapPoint = m_candidates[m_random.SmallInt(size)];
    m_candidates.push_back(swapPoint);
    swapPoint = p;
  }
}

template<class BOARD>
inline void GoUctPureRandomGenerator<BOARD>::Start() {
  m_nuEmptyFloat = 0;
  m_candidates.clear();
  for (typename BOARD::Iterator it(m_bd); it; ++it)
    if (m_bd.IsEmpty(*it)) {
      Insert(*it);
      m_nuEmptyFloat += 1.f;
    }
  m_invNuPoints = 1.f / float(m_bd.Size() * m_bd.Size());
  CheckConsistency();
}

#endif // GOUCT_PURERANDOMGENERATOR_H
