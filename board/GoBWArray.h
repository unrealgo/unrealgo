//----------------------------------------------------------------------------


#ifndef SG_BWARRAY_H
#define SG_BWARRAY_H

#include <boost/static_assert.hpp>
#include "board/GoBlackWhite.h"



template<class T>
class SgBWArray {
 public:

  SgBWArray();
  SgBWArray(const T &val);
  SgBWArray(const T &black, const T &white);
  bool operator==(const SgBWArray &bwArray) const;
  bool operator!=(const SgBWArray &bwArray) const;
  T &operator[](SgBlackWhite color);
  const T &operator[](SgBlackWhite color) const;

 private:
  BOOST_STATIC_ASSERT(SG_BLACK == 0);
  BOOST_STATIC_ASSERT(SG_WHITE == 1);

  T m_array[2];
};

template<class T>
inline SgBWArray<T>::SgBWArray() {}

template<class T>
inline SgBWArray<T>::SgBWArray(const T &val) {
  m_array[SG_BLACK] = val;
  m_array[SG_WHITE] = val;
}

template<class T>
inline SgBWArray<T>::SgBWArray(const T &black, const T &white) {
  m_array[SG_BLACK] = black;
  m_array[SG_WHITE] = white;
}

template<class T>
inline bool SgBWArray<T>::operator==(const SgBWArray &bwArray) const {
  return (m_array[SG_BLACK] == bwArray.m_array[SG_BLACK]
      && m_array[SG_WHITE] == bwArray.m_array[SG_WHITE]);
}

template<class T>
inline bool SgBWArray<T>::operator!=(const SgBWArray &bwArray) const {
  return !operator==(bwArray);
}

template<class T>
inline T &SgBWArray<T>::operator[](SgBlackWhite color) {
  DBG_ASSERT_BW(color);
  return m_array[color];
}

template<class T>
inline const T &SgBWArray<T>::operator[](SgBlackWhite color) const {
  DBG_ASSERT_BW(color);
  return m_array[color];
}

#endif
