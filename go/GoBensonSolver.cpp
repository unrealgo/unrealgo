
#include "platform/SgSystem.h"
#include "GoBensonSolver.h"

#include "GoBlock.h"
#include "GoSafetyUtil.h"
#include "platform/SgDebug.h"

namespace {
const bool DEBUG_BENSON = false;
}

void GoBensonSolver::FindSafePoints(GoBWSet* safe) {
  safe->Clear();
  GoStaticSafetySolver::FindSafePoints(safe);

  if (DEBUG_BENSON)
    GoSafetyUtil::WriteStatistics("GoBensonSolver", Regions(), safe);
}