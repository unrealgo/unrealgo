

#ifndef GO_SETUP_H
#define GO_SETUP_H

#include "board/GoBlackWhite.h"
#include "board/GoBWSet.h"
#include "board/GoPoint.h"

class GoSetup {
 public:
  GoBWSet m_stones;
  SgBlackWhite m_player;
  GoSetup();
  bool operator==(const GoSetup& setup) const;
  bool operator!=(const GoSetup& setup) const;
  void AddBlack(GoPoint p);
  void AddWhite(GoPoint p);
  bool IsEmpty() const;
};

inline GoSetup::GoSetup()
    : m_player(SG_BLACK) {}

inline bool GoSetup::operator==(const GoSetup& setup) const {
  return (m_stones == setup.m_stones && m_player == setup.m_player);
}

inline bool GoSetup::operator!=(const GoSetup& setup) const {
  return !operator==(setup);
}

inline void GoSetup::AddBlack(GoPoint p) {
  m_stones[SG_BLACK].Include(p);
}

inline void GoSetup::AddWhite(GoPoint p) {
  m_stones[SG_WHITE].Include(p);
}

inline bool GoSetup::IsEmpty() const {
  return (m_stones.BothEmpty() && m_player == SG_BLACK);
}

#endif

