
#include "platform/SgSystem.h"
#include "board/GoPointSetUtil.h"

#include <iostream>
#include "board/GoPointSet.h"
#include "board/SgWrite.h"

using namespace std;

PointSetWriter::PointSetWriter(const GoPointSet &pointSet, string label,
                                 bool writeSize)
    : m_writeSize(writeSize),
      m_pointSet(pointSet),
      m_label(label) {}

ostream &PointSetWriter::Write(ostream &out) const {
  const size_t charPerLine = 60;
  int size = m_pointSet.Size();
  if (m_label != "")
    out << SgWriteLabel(m_label);
  ostringstream buffer;
  if (m_writeSize)
    buffer << size;
  if (size > 0) {
    if (m_writeSize)
      buffer << "  ";
    for (SgSetIterator point(m_pointSet); point; ++point) {
      if (buffer.str().size() > charPerLine) {
        out << buffer.str() << '\n';
        buffer.str("");
        if (m_label != "")
          out << SgWriteLabel("");
      }
      buffer << GoWritePoint(*point) << ' ';
    }
  }
  out << buffer.str() << '\n';
  return out;
}

ostream &operator<<(ostream &out, const PointSetWriter &write) {
  return write.Write(out);
}

ostream &operator<<(ostream &out, const GoPointSet &pointSet) {
  return out << PointSetWriter(pointSet, "");
}

ostream &operator<<(ostream &stream, const PointSetIDWriter &w) {
  const GoPointSet &points = w.Points();
  if (points.IsEmpty())
    stream << "(none)";
  else {
    stream << GoWritePoint(points.Center())
           << ", Size " << points.Size();
  }
  return stream;
}


istream &operator>>(istream &in, GoPointSet &pointSet) {
  string pointstr;
  int size;
  in >> size;
  for (int i = 0; i < size; ++i) {
    in >> pointstr;
    if (pointstr.size() < 4)
    {
      int col = toupper(pointstr[0]) - 'A' + 1;
      int row = toupper(pointstr[1]) - '0';
      if (pointstr.size() == 3)
        row = row * 10 + pointstr[2] - '0';
      pointSet.Include(GoPointUtil::Pt(col, row));
    }
  }
  return in;
}


void GoPointSetUtil::Rotate(int rotation, GoPointSet &pointSet, int boardSize) {
  GoPointSet newSet;
  for (SgSetIterator it(pointSet); it; ++it)
    newSet.Include(GoPointUtil::Rotate(rotation, *it, boardSize));
  pointSet = newSet;
}