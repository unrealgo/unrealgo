
#include "platform/SgSystem.h"
#include "SgTimeSettings.h"

SgTimeSettings::SgTimeSettings()
    : m_mainTime(0),
      m_overtime(1),
      m_overtimeMoves(0) {
  DBG_ASSERT(IsUnknown());
}

SgTimeSettings::SgTimeSettings(double mainTime)
    : m_mainTime(mainTime),
      m_overtime(0),
      m_overtimeMoves(0) {}

SgTimeSettings::SgTimeSettings(double mainTime, double overtime,
                               int overtimeMoves)
    : m_mainTime(mainTime),
      m_overtime(overtime),
      m_overtimeMoves(overtimeMoves) {
  DBG_ASSERT(mainTime >= 0);
  DBG_ASSERT(overtime >= 0);
  DBG_ASSERT(overtimeMoves >= 0);
}

bool SgTimeSettings::operator==(const SgTimeSettings &timeSettings)
const {
  return (timeSettings.m_mainTime == m_mainTime
      && timeSettings.m_overtime == m_overtime
      && timeSettings.m_overtimeMoves == m_overtimeMoves);
}

bool SgTimeSettings::IsUnknown() const {
  return (m_overtime > 0 && m_overtimeMoves == 0);
}