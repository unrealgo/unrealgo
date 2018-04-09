
#include "platform/SgSystem.h"
#include "board/GoRect.h"

#include <iostream>
#include <limits>

using std::numeric_limits;


GoRect::GoRect()
    : m_left(numeric_limits<int>::max()),
      m_right(numeric_limits<int>::min()),
      m_top(numeric_limits<int>::max()),
      m_bottom(numeric_limits<int>::min()) {}

std::ostream &operator<<(std::ostream &stream, const GoRect &rect) {
  stream << "((" << rect.Left() << ',' << rect.Top() << "),"
         << '(' << rect.Right() << ',' << rect.Bottom() << "))";
  return stream;
}

void GoRect::IncludeXY(int x, int y) {
  if (x < m_left)
    m_left = x;
  if (x > m_right)
    m_right = x;
  if (y < m_top)
    m_top = y;
  if (y > m_bottom)
    m_bottom = y;
}

void GoRect::Include(GoPoint p) {
  int x = GoPointUtil::Col(p);
  int y = GoPointUtil::Row(p);
  IncludeXY(x, y);
}

void GoRect::Include(const GoRect &rect) {
  if (rect.m_left < m_left)
    m_left = rect.m_left;
  if (rect.m_right > m_right)
    m_right = rect.m_right;
  if (rect.m_top < m_top)
    m_top = rect.m_top;
  if (rect.m_bottom > m_bottom)
    m_bottom = rect.m_bottom;
}

void GoRect::Intersect(const GoRect &rect) {
  m_left = std::max(m_left, rect.m_left);
  m_right = std::min(m_right, rect.m_right);
  m_top = std::max(m_top, rect.m_top);
  m_bottom = std::min(m_bottom, rect.m_bottom);
}

GoPoint GoRect::Center() const {
  DBG_ASSERT(!IsEmpty());
  return GoPointUtil::Pt((m_left + m_right) / 2, (m_top + m_bottom) / 2);
}

bool GoRect::InRect(GoPoint p) const {
  int x = GoPointUtil::Col(p);
  int y = GoPointUtil::Row(p);
  return (x >= m_left) && (x <= m_right) && (y >= m_top) && (y <= m_bottom);
}

bool GoRect::Contains(const GoRect &rect) const {
  return (m_left <= rect.m_left) && (m_right >= rect.m_right)
      && (m_top <= rect.m_top) && (m_bottom >= rect.m_bottom);
}

bool GoRect::Overlaps(const GoRect &rect2) const {
  return (((m_left >= rect2.m_left) && (m_left <= rect2.m_right))
      || ((rect2.m_left >= m_left) && (rect2.m_left <= m_right)))
      && (((m_top >= rect2.m_top) && (m_top <= rect2.m_bottom))
          || ((rect2.m_top >= m_top) && (rect2.m_top <= m_bottom)));
}

void GoRect::Expand(int margin) {
  m_left -= margin;
  m_right += margin;
  m_top -= margin;
  m_bottom += margin;
}

