
#ifndef GOUCT_ROOTFILTER_H
#define GOUCT_ROOTFILTER_H

#include <vector>
#include "board/GoPoint.h"

class GoBoard;
class GoUctMoveFilter {
 public:
  virtual ~GoUctMoveFilter();
  virtual std::vector<GoPoint> Get() = 0;
};

#endif
