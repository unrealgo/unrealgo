

#ifndef GO_TIMECONTROL_H
#define GO_TIMECONTROL_H

#include "SgTimeControl.h"

class GoBoard;
class GoTimeControl : public SgDefaultTimeControl {
 public:
  explicit GoTimeControl(const GoBoard& bd);
  float FinalSpace() const;
  void SetFinalSpace(float finalspace);
  void GetPositionInfo(SgBlackWhite& toPlay, int& movesPlayed,
                       int& estimatedRemainingMoves);

 private:
  const GoBoard& m_bd;
  float m_finalSpace;
};

#endif
