
#ifndef SG_SEARCHVALUE_H
#define SG_SEARCHVALUE_H

#include <cstdlib>
#include <string>
#include "board/GoBlackWhite.h"
#include "SgABSearch.h"

class SgSearchValue {
 public:
  enum {
    MAX_LEVEL = 125,
    MAX_VALUE = MAX_LEVEL * SgABSearch::MAX_DEPTH,
    MIN_VALUE = -MAX_VALUE,
    MAX_KO_LEVEL = 3,
    KO_VALUE = MAX_VALUE - SgABSearch::MAX_DEPTH,
    MIN_PROVEN_VALUE = MAX_VALUE - (MAX_KO_LEVEL + 1) * SgABSearch::MAX_DEPTH
  };
  SgSearchValue();
  explicit SgSearchValue(int v);
  SgSearchValue(SgBlackWhite goodForPlayer, int depth);
  SgSearchValue(SgBlackWhite goodForPlayer, int depth, int koLevel);
  explicit operator int() const;
  int Depth() const;

  bool FromString(const std::string &s);
  bool IsEstimate() const;
  bool IsKoValue() const;
  bool IsPositive() const;
  static inline bool IsSolved(int value);
  bool IsSureValue() const;
  int KoLevel() const;
  void SetValueForPlayer(SgBlackWhite player);
  int ValueForBlack() const;
  int ValueForPlayer(SgBlackWhite player) const;
  int ValueForWhite() const;
  std::string ToString(int unitPerPoint = 1) const;

 private:
  int m_value;
};

inline SgSearchValue::SgSearchValue()
    : m_value(0) {}

inline SgSearchValue::SgSearchValue(int v)
    : m_value(v) {
  DBG_ASSERT(-MAX_VALUE <= v && v <= MAX_VALUE);
}

inline SgSearchValue::SgSearchValue(SgBlackWhite goodForPlayer, int depth)
    : m_value(MAX_VALUE - depth) {
  DBG_ASSERT_BW(goodForPlayer);
  DBG_ASSERT(0 <= depth && depth < SgABSearch::MAX_DEPTH);
  SetValueForPlayer(goodForPlayer);
  // Make sure value gets encoded/decoded consistently.
  DBG_ASSERT(KoLevel() == 0);
  DBG_ASSERT(Depth() == depth);
}

inline SgSearchValue::SgSearchValue(SgBlackWhite goodForPlayer, int depth, int koLevel)
    : m_value(MAX_VALUE - depth - koLevel * SgABSearch::MAX_DEPTH) {
  DBG_ASSERT_BW(goodForPlayer);
  DBG_ASSERT(0 <= depth && depth < SgABSearch::MAX_DEPTH);
  DBG_ASSERT(0 <= koLevel && koLevel <= MAX_KO_LEVEL);
  SetValueForPlayer(goodForPlayer);
  DBG_ASSERT(KoLevel() == koLevel);
  DBG_ASSERT(Depth() == depth);
}

inline SgSearchValue::operator int() const {
  return m_value;
}

inline int SgSearchValue::Depth() const {
  if (IsEstimate())
    return 0;
  else
    return (SgABSearch::MAX_DEPTH - 1)
        - (std::abs(m_value) - 1) % SgABSearch::MAX_DEPTH;
}

inline bool SgSearchValue::IsEstimate() const {
  return -MIN_PROVEN_VALUE < m_value && m_value < MIN_PROVEN_VALUE;
}

inline bool SgSearchValue::IsKoValue() const {
  return IsSureValue() && -KO_VALUE < m_value && m_value < KO_VALUE;
}

inline bool SgSearchValue::IsPositive() const {
  return 0 <= m_value;
}

inline bool SgSearchValue::IsSureValue() const {
  return m_value <= -MIN_PROVEN_VALUE || MIN_PROVEN_VALUE <= m_value;
}

inline bool SgSearchValue::IsSolved(int value) {
  return abs(value) == MAX_VALUE;
}

inline void SgSearchValue::SetValueForPlayer(SgBlackWhite player) {
  if (player == SG_WHITE)
    m_value = -m_value;
}

inline int SgSearchValue::ValueForBlack() const {
  return +m_value;
}

inline int SgSearchValue::ValueForPlayer(SgBlackWhite player) const {
  DBG_ASSERT_BW(player);
  return player == SG_WHITE ? -m_value : +m_value;
}

inline int SgSearchValue::ValueForWhite() const {
  return -m_value;
}

#endif // SG_SEARCHVALUE_H
