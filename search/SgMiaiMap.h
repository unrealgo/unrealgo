
#ifndef SG_MIAIMAP_H
#define SG_MIAIMAP_H

#include "lib/Array.h"
#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "SgMiaiStrategy.h"
#include "board/GoPoint.h"


class SgMiaiMap {
 public:
  SgMiaiMap();
  void ExecuteMove(GoPoint p, SgBlackWhite player);


  GoPoint ForcedMove() const { return m_forcedMove; }


  SgStrategyStatus Status() const;

  void ConvertFromSgMiaiStrategy(const SgMiaiStrategy &s);

  void ConvertToSgMiaiStrategy(SgMiaiStrategy *s) const;

 private:

  GoPoint m_forcedMove;

  bool m_failed;

  SgBWArray<GoArray<int, GO_MAXPOINT> > m_map;
};
std::ostream &operator<<(std::ostream &stream, const SgStrategy &s);

#endif
