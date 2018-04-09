
#ifndef GOUCT_DEFAULTROOTFILTER_H
#define GOUCT_DEFAULTROOTFILTER_H

#include "GoLadder.h"
#include "GoUctMoveFilter.h"

class GoBoard;
class GoUctDefaultMoveFilterParam {
 public:
  GoUctDefaultMoveFilterParam();
  bool CheckLadders() const;
  void SetCheckLadders(bool enable);
  bool CheckOffensiveLadders() const;
  void SetCheckOffensiveLadders(bool enable);
  int MinLadderLength() const;
  void SetMinLadderLength(int length);
  bool FilterFirstLine() const;
  void SetFilterFirstLine(bool enable);
  bool CheckSafety() const;
  void SetCheckSafety(bool enable);

 public:
  bool m_checkLadders;
  bool m_checkOffensiveLadders;
  int m_minLadderLength;
  bool m_filterFirstLine;
  bool m_checkSafety;
};

inline bool GoUctDefaultMoveFilterParam::CheckLadders() const {
  return m_checkLadders;
}

inline void GoUctDefaultMoveFilterParam::SetCheckLadders(bool enable) {
  m_checkLadders = enable;
}

inline bool GoUctDefaultMoveFilterParam::CheckOffensiveLadders() const {
  return m_checkOffensiveLadders;
}

inline void GoUctDefaultMoveFilterParam::SetCheckOffensiveLadders(bool enable) {
  m_checkOffensiveLadders = enable;
}

inline bool GoUctDefaultMoveFilterParam::FilterFirstLine() const {
  return m_filterFirstLine;
}

inline void GoUctDefaultMoveFilterParam::SetFilterFirstLine(bool flag) {
  m_filterFirstLine = flag;
}

inline bool GoUctDefaultMoveFilterParam::CheckSafety() const {
  return m_checkSafety;
}

inline void GoUctDefaultMoveFilterParam::SetCheckSafety(bool flag) {
  m_checkSafety = flag;
}

inline int GoUctDefaultMoveFilterParam::MinLadderLength() const {
  return m_minLadderLength;
}

inline void GoUctDefaultMoveFilterParam::SetMinLadderLength(int length) {
  m_minLadderLength = length;
}

class GoUctDefaultMoveFilter : public GoUctMoveFilter {
 public:
  GoUctDefaultMoveFilter(const GoBoard &bd, const GoUctDefaultMoveFilterParam &param);
  std::vector<GoPoint> Get();

 private:
  const GoBoard &m_bd;
  const GoUctDefaultMoveFilterParam &m_param;
  GoLadder m_ladder;
  mutable SgVector<GoPoint> m_ladderSequence;
};

#endif
