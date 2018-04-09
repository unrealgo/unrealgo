
#ifndef SG_BWSET_H
#define SG_BWSET_H

#include "board/GoPointSet.h"
#include "board/GoBWArray.h"
#include "board/GoPointSetUtil.h"

class GoBWSet {
 public:
  GoBWSet() {}

  GoBWSet(const GoPointSet &black, const GoPointSet &white)
      : m_set(black, white) {}

  const GoPointSet &operator[](SgBlackWhite c) const {
    return m_set[c];
  }

  GoPointSet &operator[](SgBlackWhite c) {
    return m_set[c];
  }

  bool operator==(const GoBWSet &other) const;
  bool operator!=(const GoBWSet &other) const;
  GoBWSet &operator|=(const GoBWSet &other);

  void Clear() {
    m_set[SG_BLACK].Clear();
    m_set[SG_WHITE].Clear();
  }

  bool BothEmpty() const {
    return m_set[SG_BLACK].IsEmpty() && m_set[SG_WHITE].IsEmpty();
  }

  bool Disjoint() const {
    return m_set[SG_BLACK].Disjoint(m_set[SG_WHITE]);
  }

  void AssertDisjoint() const {
    DBG_ASSERT(Disjoint());
  }

  GoPointSet Both() const {
    return m_set[SG_BLACK] | m_set[SG_WHITE];
  }

  bool OneContains(GoPoint p) const {
    return m_set[SG_BLACK].Contains(p)
        || m_set[SG_WHITE].Contains(p);
  }

  bool BothContain(GoPoint p) const {
    return m_set[SG_BLACK].Contains(p)
        && m_set[SG_WHITE].Contains(p);
  }

 private:
  SgBWArray<GoPointSet> m_set;
};

inline bool GoBWSet::operator==(const GoBWSet &other) const {
  return (m_set[SG_BLACK] == other.m_set[SG_BLACK]
      && m_set[SG_WHITE] == other.m_set[SG_WHITE]);
}

inline bool GoBWSet::operator!=(const GoBWSet &other) const {
  return !operator==(other);
}

inline GoBWSet &GoBWSet::operator|=(const GoBWSet &other) {
  m_set[SG_BLACK] |= other.m_set[SG_BLACK];
  m_set[SG_WHITE] |= other.m_set[SG_WHITE];
  return (*this);
}

inline std::ostream &operator<<(std::ostream &out, const GoBWSet &set) {
  out << set[SG_BLACK] << set[SG_WHITE];
  return out;
}

#endif
