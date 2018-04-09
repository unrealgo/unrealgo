
#include "platform/SgSystem.h"
#include "SgProbCut.h"

#include "SgMath.h"
#include "SgABSearch.h"


bool SgProbCut::ProbCut(SgABSearch &search, int depth, int alpha, int beta,
                        SgSearchStack &moveStack, bool *isExactValue,
                        int *value) {
  DBG_ASSERT(IsEnabled());
  SetEnabled(false);

  Cutoff c;
  int index = 0;
  while (GetCutoff(depth / SgABSearch::DEPTH_UNIT, index++, c)) {
    SgSearchStack newStack;
    bool isExact;
    float threshold = GetThreshold();

    if (beta < SgABSearch::SG_INFINITY - 1) {
      float b = (+threshold * c.sigma + float(beta) - c.b) / c.a;
      int bound = SgMath::RoundToInt(b);
      int res = search.SearchEngine(c.shallow * SgABSearch::DEPTH_UNIT,
                                    bound - 1, bound, newStack, &isExact);
      if (res >= bound) {
        SetEnabled(true);
        newStack.PushAll(moveStack);
        newStack.SwapWith(moveStack);
        *isExactValue = isExact;
        *value = beta;
        return true;
      }
    }

    if (alpha > -SgABSearch::SG_INFINITY + 1) {
      float b = (-threshold * c.sigma + float(alpha) - c.b) / c.a;
      int bound = SgMath::RoundToInt(b);
      int res = search.SearchEngine(c.shallow * SgABSearch::DEPTH_UNIT,
                                    bound, bound + 1, newStack, &isExact);

      if (res <= bound) {
        SetEnabled(true);
        newStack.PushAll(moveStack);
        newStack.SwapWith(moveStack);
        *isExactValue = isExact;
        *value = alpha;
        return true;
      }
    }
  }
  SetEnabled(true);
  return false;
}

//----------------------------------------------------------------------------
