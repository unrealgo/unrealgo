
#ifndef SG_NBITERATOR_H
#define SG_NBITERATOR_H

#include <algorithm>
#include "board/GoPoint.h"


class SgNb4Iterator
    : public GoArray<GoPoint, 4>::Iterator {
 public:
  SgNb4Iterator(GoPoint p)
      : GoArray<GoPoint, 4>::Iterator(s_precomp.m_nb[p]) {
    DBG_ASSERT(GoPointUtil::InBoardRange(p));
  }

 private:

  struct Precomp {
    Precomp();
    GoArray<GoArray<GoPoint, 4>, GO_MAXPOINT> m_nb;
  };
  static const Precomp s_precomp;

  SgNb4Iterator(const SgNb4Iterator &);

  SgNb4Iterator &operator=(const SgNb4Iterator &);
};



class SgNb4DiagIterator {
 public:
  SgNb4DiagIterator(GoPoint p)
      : m_next(0),
        m_p(p) {
    DBG_ASSERT(GoPointUtil::InBoardRange(p));
  }


  void operator++() {
    DBG_ASSERT(m_next < 4);
    ++m_next;
  }


  GoPoint operator*() const {
    return m_p + s_diag_offset[m_next];
  }


  operator bool() const {
    return m_next < 4;
  }

 private:
  int m_next;
  GoPoint m_p;
  static const int s_diag_offset[4];

  operator int() const;

  SgNb4DiagIterator(const SgNb4DiagIterator &);

  SgNb4DiagIterator &operator=(const SgNb4DiagIterator &);
};


class SgNb8Iterator {
 public:
  SgNb8Iterator(GoPoint p)
      : m_next(0),
        m_p(p) {
    DBG_ASSERT(GoPointUtil::InBoardRange(p));
  }


  void operator++() {
    DBG_ASSERT(m_next < 8);
    ++m_next;
  }


  GoPoint operator*() const {
    return m_p + s_nb8_offset[m_next];
  }


  operator bool() const {
    return m_next < 8;
  }

  static int Direction(int i) {
    DBG_ASSERTRANGE(i, 0, 7);
    return s_nb8_offset[i];
  }

 private:
  int m_next;
  GoPoint m_p;
  static const int s_nb8_offset[8];

  operator int() const;

  SgNb8Iterator(const SgNb8Iterator &);

  SgNb8Iterator &operator=(const SgNb8Iterator &);
};

#endif
