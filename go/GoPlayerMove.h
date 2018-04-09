

#ifndef GO_PLAYERMOVE_H
#define GO_PLAYERMOVE_H

#include <iosfwd>
#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"

class GoPlayerMove {
 public:
  GoPlayerMove(SgBlackWhite color, GoPoint point = GO_NULLMOVE);
  GoPlayerMove();
  ~GoPlayerMove();
  bool operator==(const GoPlayerMove& move) const;
  bool operator!=(const GoPlayerMove& move) const;
  GoPoint Point() const;
  SgBlackWhite Color() const;
  void SetPoint(GoPoint move);

 private:
  SgBlackWhite m_color;
  GoPoint m_point;
};

inline GoPlayerMove::GoPlayerMove()
    : m_color(SG_BLACK),
      m_point(GO_NULLMOVE) {}

inline GoPlayerMove::GoPlayerMove(SgBlackWhite color, GoPoint point)
    : m_color(color),
      m_point(point) {
  DBG_ASSERT_BW(color);
}

inline GoPlayerMove::~GoPlayerMove() {}

inline bool GoPlayerMove::operator==(const GoPlayerMove& move) const {
  return m_color == move.m_color
      && m_point == move.m_point;
}

inline bool GoPlayerMove::operator!=(const GoPlayerMove& move) const {
  return !operator==(move);
}

inline GoPoint GoPlayerMove::Point() const {
  return m_point;
}

inline void GoPlayerMove::SetPoint(GoPoint point) {
  m_point = point;
}

inline SgBlackWhite GoPlayerMove::Color() const {
  return m_color;
}

std::ostream& operator<<(std::ostream& out, GoPlayerMove mv);

#endif
