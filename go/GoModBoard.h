

#ifndef GO_MODBOARD_H
#define GO_MODBOARD_H

#include "GoAssertBoardRestored.h"

class GoModBoard {
 public:

  GoModBoard(const GoBoard& bd, bool locked = false);
  ~GoModBoard();
  GoBoard& Board() const;
  operator GoBoard&() const;
  void Unlock();
  void Lock();

 private:
  bool m_locked;
  GoBoard& m_bd;
  GoAssertBoardRestored m_assertRestored;
};

inline GoModBoard::GoModBoard(const GoBoard& bd, bool locked)
    : m_locked(locked),
      m_bd(const_cast<GoBoard&>(bd)),
      m_assertRestored(bd) {}

inline GoModBoard::~GoModBoard() {
}

inline GoModBoard::operator GoBoard&() const {
  return Board();
}

inline GoBoard& GoModBoard::Board() const {
  DBG_ASSERT(!m_locked);
  return m_bd;
}

inline void GoModBoard::Unlock() {
  m_assertRestored.Init(m_bd);
  m_locked = false;
}

inline void GoModBoard::Lock() {
  m_assertRestored.AssertRestored();
  m_assertRestored.Clear();
  m_locked = true;
}

#endif
