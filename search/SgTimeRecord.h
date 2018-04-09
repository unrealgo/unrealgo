
#ifndef SG_TIMERECORD_H
#define SG_TIMERECORD_H

#include <iosfwd>
#include "board/GoBWArray.h"

class SgNode;

enum SgClockState {
      SG_CLOCK_OFF,
      SG_CLOCK_RUNNING,
      SG_CLOCK_SUSPENDED
};

class SgTimeRecord {
 public:
  explicit SgTimeRecord(int numMoves = 0, double period = 0,
                        double overhead = 0, bool loseOnTime = false);
  SgTimeRecord(bool oneMoveOnly, double timeForMove);
  ~SgTimeRecord();

  bool UseOvertime() const;
  int OTNumMoves() const;
  double OTPeriod() const;
  double Overhead() const;
  bool LoseOnTime() const;
  void SetOTNumMoves(int numMoves);
  void SetOTPeriod(double period);
  void SetOverhead(double overhead);
  void SetLoseOnTime(bool lose);

  SgClockState GetClockState() const;
  bool ClockIsRunning() const;
  double TimeLeft(SgBlackWhite player) const;
  int MovesLeft(SgBlackWhite color) const;
  void SetTimeLeft(SgBlackWhite color, double timeLeft);
  void TurnClockOn(bool turnOn);
  void SetMovesLeft(SgBlackWhite color, int moves);
  void SuspendClock();
  void UpdateTimeLeft();

  void EnterNode(SgNode &node, SgBlackWhite player);
  void PlayedMove(SgNode &node, SgBlackWhite player);
  void SetClock(SgNode &node, SgBlackWhite player, double time);

  static SgBWArray<double> GetTimeFromTree(SgNode &node);
  static SgBWArray<int> GetOTMovesFromTree(SgNode &node);
  static void SetTimeInTree(SgNode &node, SgBWArray<double> time);

 private:
  int m_overtimeNumMoves;
  double m_overtimePeriod;
  double m_overhead;
  bool m_loseOnTime;
  SgBlackWhite m_player;
  bool m_clockIsOn;
  bool m_suspended;
  SgNode *m_atNode;
  SgBWArray<double> m_timeLeft;
  SgBWArray<int> m_movesLeft;
  double m_timeOfLastUpdate;
};

inline SgTimeRecord::~SgTimeRecord() {}

inline bool SgTimeRecord::ClockIsRunning() const {
  return m_clockIsOn;
}

inline bool SgTimeRecord::LoseOnTime() const {
  return m_loseOnTime;
}

inline int SgTimeRecord::MovesLeft(SgBlackWhite color) const {
  return m_movesLeft[color];
}

inline int SgTimeRecord::OTNumMoves() const {
  return m_overtimeNumMoves;
}

inline double SgTimeRecord::OTPeriod() const {
  return m_overtimePeriod;
}

inline double SgTimeRecord::Overhead() const {
  return m_overhead;
}

inline void SgTimeRecord::SetLoseOnTime(bool lose) {
  m_loseOnTime = lose;
}

inline void SgTimeRecord::SetMovesLeft(SgBlackWhite color, int moves) {
  DBG_ASSERT(moves >= 0);
  m_movesLeft[color] = moves;
}

inline void SgTimeRecord::SetOTNumMoves(int numMoves) {
  m_overtimeNumMoves = numMoves;
}

inline void SgTimeRecord::SetOTPeriod(double period) {
  m_overtimePeriod = period;
}

inline void SgTimeRecord::SetOverhead(double overhead) {
  m_overhead = overhead;
}

inline void SgTimeRecord::SetTimeLeft(SgBlackWhite color, double timeLeft) {
  m_timeLeft[color] = timeLeft;
}

inline bool SgTimeRecord::UseOvertime() const {
  return m_overtimeNumMoves > 0;
}

std::ostream &operator<<(std::ostream &out, const SgTimeRecord &time);
std::ostream &operator<<(std::ostream &out, SgClockState clockState);

#endif
