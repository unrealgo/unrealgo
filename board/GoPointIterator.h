
#ifndef SG_POINTITERATOR_H
#define SG_POINTITERATOR_H

#include "board/GoPoint.h"

class GoPointIterator {
 public:
  GoPointIterator(const GoPoint *first);
  virtual ~GoPointIterator();
  void operator++();
  GoPoint operator*() const;
  operator bool() const;

 private:
  const GoPoint *m_point;
  operator int() const;
  GoPointIterator(const GoPointIterator &);
  GoPointIterator &operator=(const GoPointIterator &);
};

inline GoPointIterator::GoPointIterator(const GoPoint *first)
    : m_point(first) {}

inline GoPointIterator::~GoPointIterator() {}

inline void GoPointIterator::operator++() {
  ++m_point;
}

inline GoPoint GoPointIterator::operator*() const {
  return *m_point;
}

inline GoPointIterator::operator bool() const {
  return *m_point != GO_ENDPOINT;
}


class GoPointRangeIterator {
 public:
  GoPointRangeIterator(const GoPoint *first, const GoPoint *end);
  virtual ~GoPointRangeIterator();
  void operator++();
  GoPoint operator*() const;
  operator bool() const;

 private:
  const GoPoint *m_point;
  const GoPoint *m_end;
  
  operator int() const;
  GoPointRangeIterator(const GoPointRangeIterator &);
  GoPointRangeIterator &operator=(const GoPointRangeIterator &);
};

inline GoPointRangeIterator::GoPointRangeIterator(const GoPoint *first,
                                                  const GoPoint *end)
    : m_point(first),
      m_end(end) {}

inline GoPointRangeIterator::~GoPointRangeIterator() {}

inline void GoPointRangeIterator::operator++() {
  ++m_point;
}

inline GoPoint GoPointRangeIterator::operator*() const {
  return *m_point;
}

inline GoPointRangeIterator::operator bool() const {
  return m_point != m_end;
}

#endif
