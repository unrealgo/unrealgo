
#ifndef SG_BOARDCOLOR_H
#define SG_BOARDCOLOR_H

#include <climits>
#include "board/GoBlackWhite.h"
#include <boost/static_assert.hpp>

const int SG_EMPTY = 2;
const int SG_BORDER = 3;
const static int sg_st[] = {'B', 'W', 'E', '#'};

BOOST_STATIC_ASSERT(SG_BLACK == 0);
BOOST_STATIC_ASSERT(SG_WHITE == 1);
BOOST_STATIC_ASSERT(SG_EMPTY == 2);

typedef int SgEmptyBlackWhite;
typedef int SgBoardColor;

#define DBG_ASSERT_EBW(c) \
    DBG_ASSERT(c == SG_BLACK || c == SG_WHITE || c == SG_EMPTY)

#define DBG_ASSERT_COLOR(c) \
DBG_ASSERT(c == SG_BLACK || c == SG_WHITE || c == SG_EMPTY || c == SG_BORDER)

inline bool SgIsEmptyBlackWhite(SgBoardColor c) {
  return c == SG_BLACK || c == SG_WHITE || c == SG_EMPTY;
}

inline SgBoardColor SgOpp(SgBoardColor c) {
  DBG_ASSERT_COLOR(c);
  return c <= SG_WHITE ? SgOppBW(c) : c;
}

inline char SgEBW(SgEmptyBlackWhite color) {
  DBG_ASSERT_EBW(color);
  return sg_st[color];
}

inline char SgBoardColorChar(SgBoardColor color) {
  DBG_ASSERT_COLOR(color);
  return sg_st[color];
}


class SgEBWIterator {
 public:
  SgEBWIterator()
      : m_color(SG_BLACK) {}

  void operator++() {
    ++m_color;
  }

  SgEmptyBlackWhite operator*() const {
    return m_color;
  }

  operator bool() const {
    return m_color <= SG_EMPTY;
  }

 private:
  SgEmptyBlackWhite m_color;

  operator int() const = delete;
  SgEBWIterator(const SgEBWIterator &) = delete;
  SgEBWIterator &operator=(const SgEBWIterator &) = delete;
};

#endif
