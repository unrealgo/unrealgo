//----------------------------------------------------------------------------
/** @file GoUctPlayoutUtil.h */
//----------------------------------------------------------------------------

#ifndef GOUCT_PLAYOUT_UTIL_H
#define GOUCT_PLAYOUT_UTIL_H

#include "GoBoard.h"
#include "GoUctPlayoutPolicy.h"

namespace GoUctPlayoutUtil {

template<class BOARD>
std::pair<GoPointList, GoPointList> FindCorrections(
    typename GoUctPlayoutPolicy<BOARD>::Corrector corrFunction,
    const BOARD &bd) {
  GoPointList from;
  GoPointList to;

  for (typename BOARD::Iterator it(bd); it; ++it) {
    if (bd.IsLegal(*it)) {
      GoPoint move = *it;
      if (corrFunction(bd, move)) {
        DBG_ASSERT(move != *it);
        from.PushBack(*it);
        to.PushBack(move);
      }
    }
  }

  return std::make_pair(from, to);
}

} // namespace GoUctPlayoutUtil

#endif // GOUCT_PLAYOUT_UTIL_H
