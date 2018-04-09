#ifndef SG_TIMECONTROL_H
#define SG_TIMECONTROL_H

#include "board/GoBlackWhite.h"

class SgTimeRecord;
class SgTimeControl {
 public:
  virtual ~SgTimeControl();
  virtual double TimeForCurrentMove(const SgTimeRecord &timeRecord,
                                    bool quiet = false) = 0;

};
class SgDefaultTimeControl : public SgTimeControl {
 public:
  SgDefaultTimeControl();
  double FastOpenFactor() const;
  void SetFastOpenFactor(double factor);
  int FastOpenMoves() const;
  void SetFastOpenMoves(int nummoves);
  double RemainingConstant() const;
  void SetRemainingConstant(double value);
  void SetMinTime(double mintime);
  double TimeForCurrentMove(const SgTimeRecord &timeRecord,
                            bool quiet = false);
  virtual void GetPositionInfo(SgBlackWhite &toPlay,
                               int &movesPlayed,
                               int &estimatedRemainingMoves) = 0;

 private:
  double m_fastOpenFactor;
  int m_fastOpenMoves;
  double m_minTime;
  double m_remainingConstant;
};

class SgObjectWithDefaultTimeControl {
 public:
  virtual ~SgObjectWithDefaultTimeControl();
  virtual SgDefaultTimeControl &TimeControl() = 0;
  virtual const SgDefaultTimeControl &TimeControl() const = 0;
};

#endif
