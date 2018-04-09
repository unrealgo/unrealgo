
#ifndef SG_TIMER_H
#define SG_TIMER_H

#include "platform/SgTime.h"


class SgTimer {
 public:
  SgTimer();
  double GetTime() const;
  bool IsStopped() const;
  bool IsTimeOut(double maxTime, std::size_t checkFreq = 16);
  void Start();
  void Stop();

 private:
  bool m_isStopped;
  bool m_isTimeOut;
  std::size_t m_counter;
  double m_timeStart;
  double m_timeStop;
  SgTimer(const SgTimer &timer);
};

inline SgTimer::SgTimer()
    : m_isStopped(false),
      m_isTimeOut(false),
      m_counter(0),
      m_timeStart(0),
      m_timeStop(0) {
  Start();
}

inline double SgTimer::GetTime() const {
  if (m_isStopped)
    return m_timeStop;
  return (SgTime::Get() - m_timeStart);
}

inline bool SgTimer::IsStopped() const {
  return m_isStopped;
}

inline bool SgTimer::IsTimeOut(double maxTime, std::size_t checkFreq) {
  if (m_isTimeOut)
    return true;
  if (m_counter == 0) {
    double timeNow = SgTime::Get();
    if (timeNow - m_timeStart > maxTime) {
      m_isTimeOut = true;
      return true;
    } else
      m_counter = checkFreq;
  } else
    --m_counter;
  return false;
}

inline void SgTimer::Start() {
  m_timeStart = SgTime::Get();
  m_isStopped = false;
}

inline void SgTimer::Stop() {
  DBG_ASSERT(!IsStopped());
  m_timeStop = (SgTime::Get() - m_timeStart);
  m_isStopped = true;
}

#endif // SG_TIMER_H
