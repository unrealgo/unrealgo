
#ifndef SG_POINTARRAY_H
#define SG_POINTARRAY_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include "lib/Array.h"
#include "board/GoPoint.h"

template<class T>
class GoPointArray
    : public GoArray<T, GO_MAXPOINT> {
 public:
  
  GoPointArray();
  
  GoPointArray(const T &value);
  
  GoPointArray(const GoPointArray &pointArray);
};

template<class T>
inline GoPointArray<T>::GoPointArray() {}

template<class T>
inline GoPointArray<T>::GoPointArray(const T &value)
    : GoArray<T, GO_MAXPOINT>(value) {}

template<class T>
inline GoPointArray<T>::GoPointArray(const GoPointArray &pointArray)
    : GoArray<T, GO_MAXPOINT>(pointArray) {}


template<typename T, int SIZE>
std::ostream &SgWriteBoardFromArray(std::ostream &out,
                                    const GoArray<T, SIZE> &array,
                                    GoGrid boardSize) {
  std::ostringstream buffer;
  int maxLength = 0;
  for (GoGrid row = 1; row <= boardSize; ++row) {
    for (GoGrid col = 1; col <= boardSize; ++col) {
      buffer.str("");
      buffer << array[GoPointUtil::Pt(col, row)];
      int length = static_cast<int>(buffer.str().length());
      maxLength = std::max(maxLength, length);
    }
  }

  for (GoGrid row = boardSize; row >= 1; --row) {
    for (GoGrid col = 1; col <= boardSize; ++col) {
      GoPoint point = GoPointUtil::Pt(col, row);
      out << std::setw(maxLength) << array[point];
      if (col < boardSize)
        out << ' ';
    }
    out << '\n';
  }
  return out;
}


template<typename T>
class SgWritePointArray {
 public:
  SgWritePointArray(const GoPointArray<T> &array, GoGrid boardSize)
      : m_boardSize(boardSize),
        m_array(array) {}

  std::ostream &Write(std::ostream &out) const;

 private:
  GoGrid m_boardSize;
  const GoPointArray<T> &m_array;
};

template<typename T>
std::ostream &SgWritePointArray<T>::Write(std::ostream &out) const {
  SgWriteBoardFromArray(out, m_array, m_boardSize);
  return out;
}


template<typename T>
std::ostream &operator<<(std::ostream &out,
                         const SgWritePointArray<T> &write) {
  return write.Write(out);
}

template<typename FLOAT, int SIZE>
std::ostream &SgWriteBoardFromArrayFloat(std::ostream &out,
                                         const GoArray<FLOAT, SIZE> &array,
                                         GoGrid boardSize,
                                         bool fixed, int precision) {
  GoArray<std::string, SIZE> stringArray;
  std::ostringstream buffer;
  if (fixed)
    buffer << std::fixed;
  buffer << std::setprecision(precision);
  for (GoGrid row = 1; row <= boardSize; ++row)
    for (GoGrid col = 1; col <= boardSize; ++col) {
      buffer.str("");
      GoPoint p = GoPointUtil::Pt(col, row);
      buffer << array[p];
      stringArray[p] = buffer.str();
    }
  SgWriteBoardFromArray(out, stringArray, boardSize);
  return out;
}



template<typename FLOAT>
class SgWritePointArrayFloat {
 public:
  SgWritePointArrayFloat(const GoPointArray<FLOAT> &array, GoGrid boardSize,
                         bool fixed, int precision)
      : m_fixed(fixed),
        m_precision(precision),
        m_boardSize(boardSize),
        m_array(array) {}

  std::ostream &Write(std::ostream &out) const;

 private:
  bool m_fixed;
  int m_precision;
  GoGrid m_boardSize;
  const GoPointArray<FLOAT> &m_array;
};

template<typename FLOAT>
std::ostream &SgWritePointArrayFloat<FLOAT>::Write(std::ostream &out) const {
  SgWriteBoardFromArrayFloat(out, m_array, m_boardSize,
                             m_fixed, m_precision);
  return out;
}


template<typename FLOAT>
std::ostream &operator<<(std::ostream &out,
                         const SgWritePointArrayFloat<FLOAT> &write) {
  return write.Write(out);
}


#endif
