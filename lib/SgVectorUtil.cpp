//----------------------------------------------------------------------------
/** @file SgVectorUtil.cpp
    See SgVectorUtil.h */
//----------------------------------------------------------------------------

#include "lib/SgVectorUtil.h"

void SgVectorUtil::Difference(SgVector<int>* left,
                              const SgVector<int>& right) {
  for (SgVectorIterator<int> it(right); it; ++it)
    left->Exclude(*it);
}

void SgVectorUtil::Intersection(SgVector<int>* left,
                                const SgVector<int>& right) {
  SgVector<int> newVector;
  for (SgVectorIterator<int> it(*left); it; ++it) {
    if (right.Contains(*it))
      newVector.PushBack(*it);
  }
  newVector.SwapWith(left);
}
