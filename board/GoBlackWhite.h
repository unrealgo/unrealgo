
#ifndef SG_BLACKWHITE_H
#define SG_BLACKWHITE_H

#include "platform/SgSystem.h"
#include <boost/static_assert.hpp>

const int SG_BLACK = 0;
const int SG_WHITE = 1;
const int SG_BW_SUM = SG_BLACK + SG_WHITE;
BOOST_STATIC_ASSERT(SG_BLACK + 1 == SG_WHITE);

typedef int SgBlackWhite;

inline bool SgIsBlackWhite(int c) {
  return c == SG_BLACK || c == SG_WHITE;
}

#define DBG_ASSERT_BW(c) DBG_ASSERT(SgIsBlackWhite(c))

inline SgBlackWhite SgOppBW(SgBlackWhite c) {
  DBG_ASSERT_BW(c);
  return SG_BW_SUM - c;
}

inline char SgBW(SgBlackWhite color) {
  DBG_ASSERT_BW(color);
  return color == SG_BLACK ? 'B' : 'W';
}

inline const char *SgBWToString(SgBlackWhite color) {
  DBG_ASSERT_BW(color);
  return color == SG_BLACK ? "Black" : "White";
}



class SgBWIterator {
 public:
  SgBWIterator()
      : m_color(SG_BLACK) {}

  void operator++() {
    DBG_ASSERT_BW(m_color);
    ++m_color;
  }

  SgBlackWhite operator*() const {
    return m_color;
  }

  SgBlackWhite Opp() const {
    return SgOppBW(m_color);
  }

  operator bool() const {
    return m_color <= SG_WHITE;
  }

 private:
  int m_color;

  operator int() const = delete;
  SgBWIterator(const SgBWIterator &) = delete;
  SgBWIterator &operator=(const SgBWIterator &) = delete;
};

#endif
