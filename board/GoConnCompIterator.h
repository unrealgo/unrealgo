#ifndef SG_CONNCOMPITERATOR_H
#define SG_CONNCOMPITERATOR_H

#include "board/GoPointSet.h"

class GoConnCompIterator {
 public:

  GoConnCompIterator(const GoPointSet &set, int boardSize);
  void operator++();
  const GoPointSet &operator*() const;

  operator bool() const {
    return m_nextPoint <= m_lastBoardPoint;
  }

 private:
  GoPointSet m_set;
  GoPointSet m_nextSet;
  int m_nextPoint;
  int m_lastBoardPoint;

  operator int() const;

  GoConnCompIterator(const GoConnCompIterator &);

  GoConnCompIterator &operator=(const GoConnCompIterator &);
};



class GoConnComp8Iterator {
 public:

  GoConnComp8Iterator(const GoPointSet &set, int boardSize);
  void operator++();
  const GoPointSet &operator*() const;

  operator bool() const {
    return m_nextPoint <= m_lastBoardPoint;
  }

 private:
  GoPointSet m_set;
  GoPointSet m_nextSet;
  int m_nextPoint;
  int m_lastBoardPoint;

  operator int() const;

  GoConnComp8Iterator(const GoConnComp8Iterator &);

  GoConnComp8Iterator &operator=(const GoConnComp8Iterator &);
};

#endif
