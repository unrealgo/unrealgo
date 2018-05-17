
#ifndef SG_POINT_H
#define SG_POINT_H

#include <climits>
#include <cstdlib>
#include <iosfwd>
#include <string>
#include <boost/static_assert.hpp>
#include "lib/Array.h"
#include "board/SgUtil.h"
#include "config/BoardStaticConfig.h"

namespace GoMoveUtil {
bool IsCouponMove(GoMove move);
}

inline bool GoMoveUtil::IsCouponMove(GoMove move) {
  return (move == GO_COUPONMOVE || move == GO_COUPONMOVE_VIRTUAL);
}

inline bool SgIsSpecialMove(GoMove m) {
  return m < 0 || m == GO_PASS;
}

typedef int GoGrid;

#define DBG_ASSERT_GRIDRANGE(c) DBG_ASSERTRANGE(c, 1, GO_MAX_SIZE)

#define DBG_ASSERT_BOARDRANGE(p) \
    DBG_ASSERTRANGE(p, GoPointUtil::Pt(1, 1), \
                   GoPointUtil::Pt(GO_MAX_SIZE, GO_MAX_SIZE))

struct GoWritePoint {
  const GoPoint m_p;
  explicit GoWritePoint(GoPoint p);
};

std::ostream &operator<<(std::ostream &out, const GoWritePoint &writePoint);

inline GoWritePoint::GoWritePoint(GoPoint p)
    : m_p(p) {}


class GoWriteMove {
 public:
  GoWriteMove(GoPoint point, SgBlackWhite color)
      : m_point(point),
        m_color(color) {}

 private:
  friend std::ostream &operator<<(std::ostream &out, const GoWriteMove &w);
  const GoPoint m_point;
  const SgBlackWhite m_color;
};


class GoReadPoint {
 public:
  explicit GoReadPoint(GoPoint &point);
  void Read(std::istream &in) const;

 private:
  GoPoint *m_point;
};

inline GoReadPoint::GoReadPoint(GoPoint &point)
    : m_point(&point) {}


inline std::istream &operator>>(std::istream &in,
                                const GoReadPoint &readPoint) {
  readPoint.Read(in);
  return in;
}

namespace GoPointUtil {

inline char Letter(int coord) {
  DBG_ASSERT_GRIDRANGE(coord);
  return char('A' + (coord - (coord >= 9 ? 0 : 1)));
}

std::string ToString(GoPoint p);
std::string ToString2(GoPoint p);
std::string ToStringFull(GoPoint p);
GoPoint Pt(int col, int row);

class PointToRow {
 public:
  PointToRow() {
    m_row.Fill(-1);
    for (GoGrid row = 1; row <= GO_MAX_SIZE; ++row)
      for (GoGrid col = 1; col <= GO_MAX_SIZE; ++col)
        m_row[Pt(col, row)] = row;
  }

  GoGrid Row(GoPoint p) const {
    DBG_ASSERT_BOARDRANGE(p);
    DBG_ASSERT(m_row[p] >= 0);
    return m_row[p];
  }

 private:
  GoArray<GoGrid, GO_MAXPOINT> m_row;
};

class PointToCol {
 public:
  PointToCol() {
    m_col.Fill(-1);
    for (GoGrid row = 1; row <= GO_MAX_SIZE; ++row)
      for (GoGrid col = 1; col <= GO_MAX_SIZE; ++col)
        m_col[Pt(col, row)] = col;
  }

  GoGrid Col(GoPoint p) const {
    DBG_ASSERT_BOARDRANGE(p);
    DBG_ASSERT(m_col[p] >= 0);
    return m_col[p];
  }

 private:
  GoArray<GoGrid, GO_MAXPOINT> m_col;
};


inline GoGrid Col(GoPoint p) {
  static PointToCol pointToCol;
  return pointToCol.Col(p);
}


inline GoGrid Row(GoPoint p) {
  static PointToRow pointToRow;
  return pointToRow.Row(p);
}

inline int Point2Index(GoPoint p) {
  if (p == GO_PASS)
    return GO_MAX_ONBOARD;

  GoGrid row = Row(p) - 1;
  GoGrid col = Col(p) - 1;
  return row * GO_MAX_SIZE + col;
}


inline GoPoint Pt(int col, int row) {
  DBG_ASSERT_GRIDRANGE(col);
  DBG_ASSERT_GRIDRANGE(row);
  return GO_NORTH_SOUTH * row + col;
}

inline bool InBoardRange(GoPoint p) {
  return SgUtil::InRange(p, Pt(1, 1), Pt(GO_MAX_SIZE, GO_MAX_SIZE));
}


inline bool AreAdjacent(GoPoint p1, GoPoint p2) {
  int d = p2 - p1;
  return (d == GO_NORTH_SOUTH || d == GO_WEST_EAST || d == -GO_NORTH_SOUTH || d == -GO_WEST_EAST);
}


inline bool AreDiagonal(GoPoint p1, GoPoint p2) {
  return (p2 == p1 - GO_NORTH_SOUTH - GO_WEST_EAST || p2 == p1 - GO_NORTH_SOUTH + GO_WEST_EAST
      || p2 == p1 + GO_NORTH_SOUTH - GO_WEST_EAST || p2 == p1 + GO_NORTH_SOUTH + GO_WEST_EAST);
}


inline int Distance(GoPoint p1, GoPoint p2) {
  return (std::abs(GoPointUtil::Row(p1) - GoPointUtil::Row(p2))
      + std::abs(GoPointUtil::Col(p1) - GoPointUtil::Col(p2)));
}


inline bool In8Neighborhood(GoPoint p1, GoPoint p2) {
  int d = p2 - p1;
  return (d == 0 || d == GO_NORTH_SOUTH || d == GO_WEST_EAST || d == -GO_NORTH_SOUTH || d == -GO_WEST_EAST
      || d == GO_NORTH_SOUTH - GO_WEST_EAST || d == GO_NORTH_SOUTH + GO_WEST_EAST
      || d == -GO_NORTH_SOUTH - GO_WEST_EAST || d == -GO_NORTH_SOUTH + GO_WEST_EAST);
}


GoPoint Rotate(int rotation, GoPoint p, int size);

int InvRotation(int rotation);
}

#endif
