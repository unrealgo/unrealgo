
#include "platform/SgSystem.h"
#include "board/GoPoint.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include "board/SgUtil.h"

using namespace std;

std::string GoPointUtil::ToString(GoPoint p) {
  ostringstream buffer;
  if (p == GO_NULLMOVE)
    buffer << "NULL";
  else if (p == GO_PASS)
    buffer << "PASS";
  else if (p == GO_COUPONMOVE)
    buffer << "COUPON";
  else if (p == GO_COUPONMOVE_VIRTUAL)
    buffer << "COUPON_VIRTUAL";
  else if (p == UCT_RESIGN)
    buffer << "RESIGN";
  else
    buffer << Letter(Col(p)) << Row(p);
  return buffer.str();
}

std::string GoPointUtil::ToString2(GoPoint p) {
  ostringstream buffer;
  if (p == GO_NULLMOVE)
    buffer << "NULL";
  else if (p == GO_PASS)
    buffer << "PASS";
  else if (p == GO_COUPONMOVE)
    buffer << "COUPON";
  else if (p == GO_COUPONMOVE_VIRTUAL)
    buffer << "COUPON_VIRTUAL";
  else if (p == UCT_RESIGN)
    buffer << "RESIGN";
  else
    buffer << Letter(Col(p)) << std::setfill('0') << std::setw(2) << Row(p);
  return buffer.str();
}

std::string GoPointUtil::ToStringFull(GoPoint move) {
  ostringstream os;
  os << "[" << GoPointUtil::ToString2(move) << "," << move << "]";
  return os.str();
}

GoPoint GoPointUtil::Rotate(int rotation, GoPoint p, int size) {
  DBG_ASSERT(rotation < 8);
  if (p == GO_PASS)
    return GO_PASS;
  int col = Col(p);
  int row = Row(p);
  switch (rotation) {
    case 0:return Pt(col, row);
    case 1:return Pt(size - col + 1, row);
    case 2:return Pt(col, size - row + 1);
    case 3:return Pt(row, col);
    case 4:return Pt(size - row + 1, col);
    case 5:return Pt(row, size - col + 1);
    case 6:return Pt(size - col + 1, size - row + 1);
    case 7:return Pt(size - row + 1, size - col + 1);
    default:DBG_ASSERT(false);
      return p;
  }
}

int GoPointUtil::InvRotation(int rotation) {
  switch (rotation) {
    case 0:return 0;
    case 1:return 1;
    case 2:return 2;
    case 3:return 3;
    case 4:return 5;
    case 5:return 4;
    case 6:return 6;
    case 7:return 7;
    default:DBG_ASSERT(false);
      return 0;
  }
}


void GoReadPoint::Read(std::istream &in) const {
  string s;
  in >> s;
  if (!in)
    return;
  DBG_ASSERT(s.length() > 0);
  if (s == "PASS" || s == "pass") {
    *m_point = GO_PASS;
    return;
  }
  if (s == "COUPON" || s == "coupon") {
    *m_point = GO_COUPONMOVE;
    return;
  }
  if (s == "COUPON_VIRTUAL" || s == "coupon_virtual") {
    *m_point = GO_COUPONMOVE_VIRTUAL;
    return;
  }
  if (s == "RESIGN" || s == "resign") {
    *m_point = UCT_RESIGN;
    return;
  }
  char c = s[0];
  if (c >= 'A' && c <= 'Z')
    c = char(c - 'A' + 'a');
  else if (c < 'a' || c > 'z') {
    in.setstate(ios::failbit);
    return;
  }
  int col = c - 'a' + 1;
  if (c >= 'j')
    --col;
  istringstream sin(s.substr(1));
  int row;
  sin >> row;
  if (!sin || !SgUtil::InRange(col, 1, GO_MAX_SIZE)
      || !SgUtil::InRange(row, 1, GO_MAX_SIZE)) {
    in.setstate(ios::failbit);
    return;
  }
  *m_point = GoPointUtil::Pt(col, row);
}


ostream &operator<<(ostream &out, const GoWritePoint &writePoint) {
  out << GoPointUtil::ToString(writePoint.m_p);
  return out;
}

ostream &operator<<(ostream &out, const GoWriteMove &w) {
  out << SgBW(w.m_color) << ' ' << GoWritePoint(w.m_point) << ' ';
  return out;
}

