//----------------------------------------------------------------------------
/** @file SgVectorUtil.h
    %SgVector utility classes.
    
    SgVector operations that are useful but not basic enough to be included 
    in SgVector itself. */
//----------------------------------------------------------------------------

#ifndef SG_VECTORUTIL_H
#define SG_VECTORUTIL_H

#include "lib/SgVector.h"

//----------------------------------------------------------------------------

namespace SgVectorUtil {
// left = left - right
void Difference(SgVector<int>* left, const SgVector<int>& right);
// left = {x | x in left and x in right}
void Intersection(SgVector<int> *left, const SgVector<int> &right);

#if UNUSED
void Reverse(SgVector<int>* vector);
#endif
}

//----------------------------------------------------------------------------

#endif // SG_VECTORUTIL_H
