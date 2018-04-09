//----------------------------------------------------------------------------


#ifndef SG_RECT_H
#define SG_RECT_H

#include "board/GoPoint.h"


class GoRect {
 public:
  GoRect();

  GoRect(int left, int right, int top, int bottom)
      : m_left(left),
        m_right(right),
        m_top(top),
        m_bottom(bottom) {}

  GoRect(const GoPoint &topleft, const GoPoint &bottomright)
      : m_left(GoPointUtil::Col(topleft)),
        m_right(GoPointUtil::Col(bottomright)),
        m_top(GoPointUtil::Row(topleft)),
        m_bottom(GoPointUtil::Row(bottomright)) {}

  bool operator==(const GoRect &rhs) const {
    return m_left == rhs.m_left
        && m_right == rhs.m_right
        && m_top == rhs.m_top
        && m_bottom == rhs.m_bottom;
  }

  void Set(int left, int right, int top, int bottom) {
    m_left = left;
    m_right = right;
    m_top = top;
    m_bottom = bottom;
  }

  void MirrorX(int boardSize) {
    int temp = m_left;
    m_left = boardSize + 1 - m_right;
    m_right = boardSize + 1 - temp;
  }

  void MirrorY(int boardSize) {
    int temp = m_top;
    m_top = boardSize + 1 - m_bottom;
    m_bottom = boardSize + 1 - temp;
  }

  void SwapXY() {
    std::swap(m_top, m_left);
    std::swap(m_bottom, m_right);
  }


  void Include(GoPoint p);

  void Include(const GoRect &rect);

  void IncludeXY(int x, int y);
  void Intersect(const GoRect &rect);

  bool IsEmpty() const {
    return m_left > m_right;
  }

  bool InRect(GoPoint p) const;
  GoPoint Center() const;

  bool Contains(GoPoint p) const {
    return InRect(p);
  }

  bool Contains(const GoRect &rect) const;
  bool Overlaps(const GoRect &rect) const;
  void Expand(int margin);

  int Left() const {
    return m_left;
  }

  int Right() const {
    return m_right;
  }

  int Top() const {
    return m_top;
  }

  int Bottom() const {
    return m_bottom;
  }

  int Width() const {
    DBG_ASSERT(!IsEmpty());
    return m_right - m_left + 1;
  }

  int Height() const {
    DBG_ASSERT(!IsEmpty());
    return m_bottom - m_top + 1;
  }

  int Area() const {
    return Width() * Height();
  }

  void IncLeft() {
    ++m_left;
  }

  void DecRight() {
    --m_right;
  }

  void IncTop() {
    ++m_top;
  }

  void DecBottom() {
    --m_bottom;
  }

  void SetLeft(int value) {
    m_left = value;
  }

  void SetRight(int value) {
    m_right = value;
  }

  void SetTop(int value) {
    m_top = value;
  }

  void SetBottom(int value) {
    m_bottom = value;
  }

 private:
  int m_left;
  int m_right;
  int m_top;
  int m_bottom;
};


class SgRectIterator {
 public:

  explicit SgRectIterator(const GoRect &rect)
      : m_rect(rect),
        m_cursor(GoPointUtil::Pt(rect.Left(), rect.Top())),
        m_end(GoPointUtil::Pt(rect.Right(), rect.Bottom())) {}


  void operator++();

  GoPoint operator*() const;

  operator bool() const;

  bool AtEndOfLine() const;

 private:
  const GoRect &m_rect;
  GoPoint m_cursor;
  GoPoint m_end;

  operator int() const;
};
std::ostream &operator<<(std::ostream &stream, const GoRect &rect);
inline void SgRectIterator::operator++() {
  DBG_ASSERT(m_rect.Contains(m_cursor));
  if (AtEndOfLine())
    m_cursor += GO_NORTH_SOUTH + m_rect.Left() - m_rect.Right();
  else
    m_cursor += GO_WEST_EAST;
}

inline GoPoint SgRectIterator::operator*() const {
  return m_cursor;
}

inline SgRectIterator::operator bool() const {
  return m_cursor <= m_end;
}

inline bool SgRectIterator::AtEndOfLine() const {
  return GoPointUtil::Col(m_cursor) == m_rect.Right();
}

#endif
