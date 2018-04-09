
#ifndef SG_POINTSETUTIL_H
#define SG_POINTSETUTIL_H

#include <iosfwd>
#include <string>
#include "board/GoPointSet.h"


class PointSetWriter {
 public:
  explicit PointSetWriter(const GoPointSet &pointSet, std::string label = "",
                  bool writeSize = true);
  std::ostream &Write(std::ostream &out) const;

 private:
  bool m_writeSize;
  const GoPointSet &m_pointSet;
  std::string m_label;
};

std::ostream &operator<<(std::ostream &out, const PointSetWriter &write);

std::ostream &operator<<(std::ostream &out, const GoPointSet &set);


class PointSetIDWriter {
 public:
  explicit PointSetIDWriter(const GoPointSet &points)
      : m_points(points) {}

  const GoPointSet &Points() const {
    return m_points;
  }

 private:
  const GoPointSet &m_points;
};
std::ostream &operator<<(std::ostream &stream, const PointSetIDWriter &w);


std::istream &operator>>(std::istream &in, GoPointSet &pointSet);

namespace GoPointSetUtil {
  void Rotate(int rotation, GoPointSet &pointSet, int boardSize);
}

#endif
