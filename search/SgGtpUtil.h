
#ifndef SG_GTPUTIL_H
#define SG_GTPUTIL_H

#include <boost/io/ios_state.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include "UctSearch.h"

class GtpCommand;
class GoPointSet;
struct SgRGB {
 public:
  SgRGB(unsigned char r, unsigned char g, unsigned char b)
      : m_r(r), m_g(g), m_b(b) {}

  std::string ToString() const;
  SgRGB operator+(const SgRGB &color) const;
  bool operator==(const SgRGB &color) const;
  unsigned char m_r;
  unsigned char m_g;
  unsigned char m_b;
};

inline SgRGB SgRGB::operator+(const SgRGB &color) const {
  return SgRGB(m_r + color.m_r,
               m_g + color.m_g,
               m_b + color.m_b);
}

inline bool SgRGB::operator==(const SgRGB &color) const {
  return m_r == color.m_r
      && m_g == color.m_g
      && m_b == color.m_b;
}

inline SgRGB operator*(float f, const SgRGB &color) {
  return SgRGB(static_cast<unsigned char>(f * color.m_r),
               static_cast<unsigned char>(f * color.m_g),
               static_cast<unsigned char>(f * color.m_b));
}

inline std::ostream &operator<<(std::ostream &stream, const SgRGB &color) {
  boost::io::ios_flags_saver saver(stream);
  stream << '#' << std::hex << std::setfill('0')
         << std::setw(2) << int(color.m_r)
         << std::setw(2) << int(color.m_g)
         << std::setw(2) << int(color.m_b);
  return stream;
}

struct SgColorGradient {
 public:
  SgColorGradient(SgRGB start, float startVal, SgRGB end, float endVal);
  SgRGB ColorOf(float value);

 private:
  SgRGB m_start;
  float m_startVal;
  SgRGB m_end;
  float m_endVal;
};

namespace SgGtpUtil {
  void RespondPointSet(GtpCommand &cmd, const GoPointSet &pointSet);
  UctMoveSelect MoveSelectArg(const GtpCommand &cmd, size_t number);
  std::string MoveSelectToString(UctMoveSelect moveSelect);
} // namespace SgGtpUtil

#endif // SG_GTPUTIL_H
