
#ifndef SG_WRITE_H
#define SG_WRITE_H

#include <iomanip>
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>
#include "lib/SgArrayList.h"
#include "board/GoBoardColor.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class SgWriteLabel {
 public:
  explicit SgWriteLabel(const std::string &label)
      : m_label(label) {}

  friend std::ostream &operator<<(std::ostream &out,
                                  const SgWriteLabel &w);
 private:
  std::string m_label;
};

class SgWritePointList {
 public:
  explicit SgWritePointList(const std::vector<GoPoint> &pointList,
                   std::string label = "",
                   bool writeSize = true);
  explicit SgWritePointList(const SgVector<GoPoint> &pointList,
                   std::string label = "",
                   bool writeSize = true);
  std::ostream &Write(std::ostream &out) const;

 private:
  bool m_writeSize;
  std::vector<GoPoint> m_pointList;
  std::string m_label;
};
std::ostream &operator<<(std::ostream &out, const SgWritePointList &write);

template<int SIZE>
class SgWriteSPointList {
 public:
  explicit SgWriteSPointList(const GoArrayList<GoPoint, SIZE> &list,
                    std::string label = "", bool writeSize = true);
  std::ostream &Write(std::ostream &out) const;

 private:
  bool m_writeSize;
  const GoArrayList<GoPoint, SIZE> &m_list;
  std::string m_label;
};

template<int SIZE>
std::ostream &operator<<(std::ostream &out,
                         const SgWriteSPointList<SIZE> &write);

template<int SIZE>
SgWriteSPointList<SIZE>::SgWriteSPointList(
    const GoArrayList<GoPoint, SIZE> &list,
    std::string label, bool writeSize)
    : m_writeSize(writeSize),
      m_list(list),
      m_label(label) {}

template<int SIZE>
std::ostream &SgWriteSPointList<SIZE>::Write(std::ostream &out) const {
  std::vector<GoPoint> list;
  for (typename GoArrayList<GoPoint, SIZE>::Iterator it(m_list); it; ++it)
    list.push_back(*it);
  return (out << SgWritePointList(list, m_label, m_writeSize));
}

template<int SIZE>
std::ostream &operator<<(std::ostream &out,
                         const SgWriteSPointList<SIZE> &write) {
  return write.Write(out);
}

class SgWriteLine {
 public:
  SgWriteLine() {}

  friend std::ostream &operator<<(std::ostream &out,
                                  const SgWriteLine &w);
};


class SgWriteBoolean {
 public:
  explicit SgWriteBoolean(bool value) : m_value(value) {}

 private:
  friend std::ostream &operator<<(std::ostream &out,
                                  const SgWriteBoolean &w);
  bool m_value;
};


class SgWriteBoolAsInt {
 public:
  explicit SgWriteBoolAsInt(bool value);
  std::ostream &Write(std::ostream &out) const;

 private:
  bool m_value;
};
std::ostream &operator<<(std::ostream &out, const SgWriteBoolAsInt &write);


#endif
