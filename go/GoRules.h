

#ifndef GO_RULES_H
#define GO_RULES_H

#include <iosfwd>
#include <string>
#include "GoKomi.h"
const static float DEFAULT_KOMI = 7.5f;
class GoRules {
 public:

  enum KoRule {

    POS_SUPERKO,

    SIMPLEKO,

    SUPERKO
  };
  explicit GoRules(int handicap = 0, const GoKomi& komi = GoKomi(DEFAULT_KOMI),
                   bool japanese = false, bool twoPassesEndGame = true);
  bool operator==(const GoRules& rules) const;
  bool operator!=(const GoRules& rules) const;
  bool AllowSuicide() const;
  void SetAllowSuicide(bool allowSuicide);
  bool CaptureDead() const;
  void SetCaptureDead(bool captureDead);
  KoRule GetKoRule() const;
  void SetKoRule(KoRule koRule);
  int Handicap() const;
  void SetHandicap(int handicap);
  bool JapaneseHandicap() const;
  void SetJapaneseHandicap(bool japaneseHandicap);
  bool JapaneseScoring() const;
  void SetJapaneseScoring(bool japaneseScoring);
  const GoKomi& Komi() const;
  void SetKomi(const GoKomi& komi);
  bool TwoPassesEndGame() const;
  void SetTwoPassesEndGame(bool twoPassesEndGame);
  bool ExtraHandicapKomi() const;
  void SetExtraHandicapKomi(bool enable);
  void SetNamedRules(const std::string& namedRules);

 private:
  bool m_allowSuicide;
  bool m_captureDead;
  bool m_japaneseScoring;
  bool m_extraHandicapKomi;
  int m_handicap;
  GoKomi m_komi;
  bool m_japaneseHandicap;
  bool m_twoPassesEndGame;
  KoRule m_koRule;
};

inline bool GoRules::operator!=(const GoRules& rules) const {
  return !(*this == rules);
}

inline bool GoRules::AllowSuicide() const {
  return m_allowSuicide;
}

inline bool GoRules::CaptureDead() const {
  return m_captureDead;
}

inline bool GoRules::ExtraHandicapKomi() const {
  return m_extraHandicapKomi;
}

inline GoRules::KoRule GoRules::GetKoRule() const {
  return m_koRule;
}

inline int GoRules::Handicap() const {
  return m_handicap;
}

inline bool GoRules::JapaneseHandicap() const {
  return m_japaneseHandicap;
}

inline bool GoRules::JapaneseScoring() const {
  return m_japaneseScoring;
}

inline const GoKomi& GoRules::Komi() const {
  return m_komi;
}

inline void GoRules::SetAllowSuicide(bool allowSuicide) {
  m_allowSuicide = allowSuicide;
}

inline void GoRules::SetCaptureDead(bool captureDead) {
  m_captureDead = captureDead;
}

inline void GoRules::SetExtraHandicapKomi(bool enable) {
  m_extraHandicapKomi = enable;
}

inline void GoRules::SetHandicap(int handicap) {
  DBG_ASSERT(handicap >= 0);
  m_handicap = handicap;
}

inline void GoRules::SetJapaneseHandicap(bool japaneseHandicap) {
  m_japaneseHandicap = japaneseHandicap;
}

inline void GoRules::SetJapaneseScoring(bool japaneseScoring) {
  m_japaneseScoring = japaneseScoring;
}

inline void GoRules::SetKomi(const GoKomi& komi) {
  m_komi = komi;
}

inline void GoRules::SetKoRule(KoRule koRule) {
  m_koRule = koRule;
}

inline void GoRules::SetTwoPassesEndGame(bool twoPassesEndGame) {
  m_twoPassesEndGame = twoPassesEndGame;
}

inline bool GoRules::TwoPassesEndGame() const {
  return m_twoPassesEndGame;
}

std::ostream& operator<<(std::ostream& out, GoRules::KoRule koRule);

#endif

