
#ifndef SG_EBWARRAY_H
#define SG_EBWARRAY_H

#include "board/GoBoardColor.h"

template<class T>
class SgEBWArray {
 public:
  SgEBWArray() {}
  SgEBWArray(const T &val) {
    m_array[SG_BLACK] = val;
    m_array[SG_WHITE] = val;
    m_array[SG_EMPTY] = val;
  }

  SgEBWArray(const T &empty, const T &black, const T &white) {
    m_array[SG_BLACK] = black;
    m_array[SG_WHITE] = white;
    m_array[SG_EMPTY] = empty;
  }

  const T &operator[](SgEmptyBlackWhite c) const {
    DBG_ASSERT_EBW(c);
    return m_array[c];
  }

  T &operator[](SgEmptyBlackWhite c) {
    DBG_ASSERT_EBW(c);
    return m_array[c];
  }

 private:
  T m_array[3];
};

#endif
