
#ifndef SG_UCTVALUE_H
#define SG_UCTVALUE_H

#include <cmath>
#include <limits>
#include <boost/static_assert.hpp>
#include "SgStatistics.h"
#include "SgStatisticsVlt.h"

#ifdef UCT_VALUE_TYPE
typedef UCT_VALUE_TYPE UctValueType;
#else
typedef double UctValueType;
#endif

BOOST_STATIC_ASSERT(!std::numeric_limits<UctValueType>::is_integer);

typedef UctStatisticsBase<UctValueType, UctValueType> UctStatistics;
typedef UctStatisticsVltBase<UctValueType, UctValueType> UctStatisticsVolatile;


namespace UctValueUtil {
template<typename T>
inline bool IsPrecise(T val) {
  const int radix = std::numeric_limits<T>::radix;
  const int digits = std::numeric_limits<T>::digits;
  const T max = std::pow(T(radix), digits) - 1;
  return val <= max;
}

inline UctValueType InverseValue(UctValueType v) {
  return 1 - v;
}

inline UctValueType MinusInverseValue(UctValueType v) {
  return -1 * v;
}
}

#endif
