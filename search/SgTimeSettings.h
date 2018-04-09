
#ifndef GO_TIMESETTINGS_H
#define GO_TIMESETTINGS_H


class SgTimeSettings {
 public:
  SgTimeSettings();
  explicit SgTimeSettings(double mainTime);

  SgTimeSettings(double mainTime, double overtime, int overtimeMoves);
  bool operator==(const SgTimeSettings &timeSettings) const;
  double MainTime() const;
  double Overtime() const;
  int OvertimeMoves() const;
  bool IsUnknown() const;

 private:

  double m_mainTime;

  double m_overtime;

  int m_overtimeMoves;
};

inline double SgTimeSettings::MainTime() const {
  return m_mainTime;
}

inline double SgTimeSettings::Overtime() const {
  return m_overtime;
}

inline int SgTimeSettings::OvertimeMoves() const {
  return m_overtimeMoves;
}

#endif

