
#ifndef SG_MATH_H
#define SG_MATH_H

#include <cmath>

#define SG_SQRT2 1.41421356237309504880

namespace SgMath {
int RoundToInt(double v);
}

inline int SgMath::RoundToInt(double v) {
  return static_cast<int>(std::floor(v + 0.5));
}

#endif // SG_MATH_H
