

#ifndef GO_BENSONSOLVER_H
#define GO_BENSONSOLVER_H

#include "GoBoard.h"
#include "GoRegion.h"
#include "GoStaticSafetySolver.h"

class GoBensonSolver : public GoStaticSafetySolver {
 public:
  explicit GoBensonSolver(const GoBoard& board, GoRegionBoard* regions = 0)
      : GoStaticSafetySolver(board, regions) {}

  void FindSafePoints(GoBWSet* safe);
};

#endif
