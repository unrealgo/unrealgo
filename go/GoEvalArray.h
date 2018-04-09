

#ifndef GO_EVAL_ARRAY_H
#define GO_EVAL_ARRAY_H

#include <boost/static_assert.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "lib/Array.h"
#include "board/GoPoint.h"
const int GO_MAX_MOVE = GO_PASS + 1;

BOOST_STATIC_ASSERT(GO_PASS > GO_MAXPOINT);

template<class T>
class GoEvalArray
    : public GoArray<T, GO_MAX_MOVE> {
 public:

  GoEvalArray();
  GoEvalArray(const T& value);
  GoEvalArray(const GoEvalArray& array);
  void Write(std::ostream& out, GoGrid boardSize) const;
};

template<class T>
inline GoEvalArray<T>::GoEvalArray() {}

template<class T>
inline GoEvalArray<T>::GoEvalArray(const T& value)
    : GoArray<T, GO_MAX_MOVE>(value) {}

template<class T>
inline GoEvalArray<T>::GoEvalArray(const GoEvalArray& array)
    : GoArray<T, GO_MAX_MOVE>(array) {}

template<class T>
inline void GoEvalArray<T>::Write(std::ostream& out,
                                  GoGrid boardSize) const {
  SgWriteBoardFromArray(out, *this, boardSize);
  out << "Pass: " << (*this)[GO_PASS] << '\n';
}

#endif
