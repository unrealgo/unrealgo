
#ifndef SG_POINTSET_H
#define SG_POINTSET_H

#include <algorithm>
#include <bitset>
#include <cstring>
#include <iosfwd>
#include <bits/unique_ptr.h>
#include "lib/Array.h"
#include "board/GoPoint.h"
#include "board/GoRect.h"
#include "lib/SgVector.h"


class GoPointSet {
 public:
  GoPointSet();
  ~GoPointSet();
  explicit GoPointSet(const SgVector<GoPoint> &vector);
  GoPointSet &operator-=(const GoPointSet &other);
  GoPointSet &operator&=(const GoPointSet &other);
  GoPointSet &operator|=(const GoPointSet &other);
  GoPointSet &operator^=(const GoPointSet &other);
  bool operator==(const GoPointSet &other) const;
  bool operator!=(const GoPointSet &other) const;
  
  bool operator[](GoPoint p) const;
  bool Disjoint(const GoPointSet &s) const;
  
  bool Adjacent(const GoPointSet &s) const;
  bool Adjacent8To(GoPoint p) const;
  
  bool AdjacentOnlyTo(const GoPointSet &s, int boardSize) const;
  bool AdjacentTo(GoPoint p) const;
  
  bool AllAdjacentTo(const GoPointSet &s) const;
  static const GoPointSet &AllPoints(int boardSize);
  int Size() const;
  bool IsSize(int size) const;
  bool MinSetSize(int size) const;
  bool MaxSetSize(int size) const;
  bool IsEmpty() const;
  bool NonEmpty() const;
  
  GoPointSet Border(int boardSize) const;
  
  GoPointSet Border8(int boardSize) const;
  
  GoPointSet BorderNoClip() const;
  
  GoPoint Center() const;
  
  bool CheckedContains(GoPoint p, bool doRangeCheck = true,
                       bool onBoardCheck = false) const;
  GoPointSet &Clear();
  
  GoPointSet Component(GoPoint p) const;
  
  GoPointSet Component8(GoPoint p) const;
  
  GoPointSet ConnComp(GoPoint p) const;
  
  GoPointSet ConnComp8(GoPoint p) const;
  
  bool Contains(GoPoint p) const;
  
  bool ContainsPoint(GoPoint p) const;
  GoRect EnclosingRect() const;
  GoPointSet &Exclude(GoPoint p);
  GoPointSet &Exclude(const SgVector<GoPoint> &vector);
  
  void Grow(int boardSize);
  
  void Grow(GoPointSet *newArea, int boardSize);
  
  void Grow8(int boardSize);
  GoPointSet &Include(GoPoint p);
  
  bool IsConnected() const;
  
  bool Is8Connected() const;
  
  GoPointSet Kernel(int boardSize) const;
  
  bool MaxOverlap(const GoPointSet &other, int max) const;
  
  bool MinOverlap(const GoPointSet &s, int min) const;
  bool NewMark(GoPoint p);
  bool Overlaps(const GoPointSet &other) const;
  
  GoPoint PointOf() const;
  
  bool SubsetOf(const GoPointSet &other) const;
  
  bool SupersetOf(const GoPointSet &other) const;
  void Swap(GoPointSet &other) throw();
  GoPointSet &Toggle(GoPoint p);
  void ToVector(SgVector<GoPoint> *vector) const;
  void Write(std::ostream &out, int boardSize) const;
  
  bool IsCloseTo(GoPoint p) const;

 private:
  
  class PrecompAllPoints {
   public:
    PrecompAllPoints();

    const GoPointSet &Get(int boardSize) {
      DBG_ASSERT(boardSize >= GO_MIN_SIZE);
      DBG_ASSERT(boardSize <= GO_MAX_SIZE);
      return *m_allPoints[boardSize];
    }

   private:
    GoArray<std::unique_ptr<GoPointSet>, GO_MAX_SIZE + 1> m_allPoints;
  };
  friend class SgSetIterator;
  std::bitset<GO_MAXPOINT> m_a;
  static PrecompAllPoints s_allPoints;
  GoPointSet operator>>(int n) const;
  GoPointSet operator<<(int n) const;

};


inline GoPointSet operator-(const GoPointSet &L, const GoPointSet &R) {
  return (GoPointSet(L) -= R);
}


inline GoPointSet operator&(const GoPointSet &L, const GoPointSet &R) {
  return (GoPointSet(L) &= R);
}


inline GoPointSet operator|(const GoPointSet &L, const GoPointSet &R) {
  return (GoPointSet(L) |= R);
}


inline GoPointSet operator^(const GoPointSet &L, const GoPointSet &R) {
  return (GoPointSet(L) ^= R);
}

inline GoPointSet::GoPointSet() {}

inline GoPointSet::~GoPointSet() {}

inline void GoPointSet::Swap(GoPointSet &other) throw() {
  std::swap(m_a, other.m_a);
}

inline GoPointSet &GoPointSet::operator-=(const GoPointSet &other) {
  m_a &= ~other.m_a;
  return (*this);
}

inline GoPointSet &GoPointSet::operator&=(const GoPointSet &other) {
  m_a &= other.m_a;
  return (*this);
}

inline GoPointSet &GoPointSet::operator|=(const GoPointSet &other) {
  m_a |= other.m_a;
  return (*this);
}

inline GoPointSet &GoPointSet::operator^=(const GoPointSet &other) {
  m_a ^= other.m_a;
  return (*this);
}

inline bool GoPointSet::operator==(const GoPointSet &other) const {
  return m_a == other.m_a;
}

inline bool GoPointSet::operator!=(const GoPointSet &other) const {
  return m_a != other.m_a;
}

inline const GoPointSet &GoPointSet::AllPoints(int boardSize) {
  return s_allPoints.Get(boardSize);
}

inline bool GoPointSet::Overlaps(const GoPointSet &other) const {
  return (m_a & other.m_a).any();
}

inline bool GoPointSet::MaxOverlap(const GoPointSet &other, int max) const {
  GoPointSet s(*this);
  s &= other;
  return s.Size() <= max;
}

inline bool GoPointSet::MinOverlap(const GoPointSet &s, int min) const {
  return !MaxOverlap(s, min - 1);
}

inline bool GoPointSet::Disjoint(const GoPointSet &s) const {
  return !Overlaps(s);
}

inline bool GoPointSet::AdjacentTo(GoPoint p) const {
  DBG_ASSERT_BOARDRANGE(p);
  return Contains(p + GO_NORTH_SOUTH)
      || Contains(p - GO_NORTH_SOUTH)
      || Contains(p + GO_WEST_EAST)
      || Contains(p - GO_WEST_EAST);
}

inline bool GoPointSet::Adjacent8To(GoPoint p) const {
  DBG_ASSERT_BOARDRANGE(p);
  return Contains(p + GO_NORTH_SOUTH) || Contains(p - GO_NORTH_SOUTH) || Contains(p + GO_WEST_EAST)
      || Contains(p - GO_WEST_EAST) || Contains(p + GO_NORTH_SOUTH + GO_WEST_EAST)
      || Contains(p + GO_NORTH_SOUTH - GO_WEST_EAST) || Contains(p - GO_NORTH_SOUTH + GO_WEST_EAST)
      || Contains(p - GO_NORTH_SOUTH - GO_WEST_EAST);
}

inline bool GoPointSet::SubsetOf(const GoPointSet &other) const {
  return (m_a & ~other.m_a).none();
}

inline bool GoPointSet::SupersetOf(const GoPointSet &other) const {
  return (other.m_a & ~m_a).none();
}

inline int GoPointSet::Size() const {
  return static_cast<int>(m_a.count());
}

inline bool GoPointSet::IsEmpty() const {
  return m_a.none();
}

inline bool GoPointSet::NonEmpty() const {
  return !IsEmpty();
}

inline GoPointSet &GoPointSet::Exclude(GoPoint p) {
  DBG_ASSERT_BOARDRANGE(p);
  m_a.reset(p);
  return (*this);
}

inline GoPointSet &GoPointSet::Include(GoPoint p) {
  DBG_ASSERT_BOARDRANGE(p);
  m_a.set(p);
  return (*this);
}

inline GoPointSet &GoPointSet::Clear() {
  m_a.reset();
  return *this;
}

inline GoPointSet &GoPointSet::Toggle(GoPoint p) {
  DBG_ASSERT(p < static_cast<int>(m_a.size()));
  m_a.flip(p);
  return (*this);
}

inline bool GoPointSet::Contains(GoPoint p) const {
  return m_a.test(p);
}

inline bool GoPointSet::CheckedContains(GoPoint p, bool doRangeCheck,
                                        bool onBoardCheck) const {
  if (doRangeCheck) {
    if (onBoardCheck)
      DBG_ASSERT_BOARDRANGE(p);
    else
      DBG_ASSERTRANGE(p, GoPointUtil::Pt(0, 0),
                     GoPointUtil::Pt(GO_MAX_SIZE + 1, GO_MAX_SIZE + 1));
  }
  return m_a.test(p);
}

inline bool GoPointSet::ContainsPoint(GoPoint p) const {
  return CheckedContains(p, true, true);
}

inline bool GoPointSet::operator[](GoPoint p) const {
  return Contains(p);
}

inline bool GoPointSet::NewMark(GoPoint p) {
  if (Contains(p))
    return false;
  else {
    Include(p);
    return true;
  }
}

inline GoPointSet GoPointSet::operator>>(int n) const {
  GoPointSet result(*this);
  result.m_a >>= n;
  return result;
}

inline GoPointSet GoPointSet::operator<<(int n) const {
  GoPointSet result(*this);
  result.m_a <<= n;
  return result;
}


class SgSetIterator {
 public:
  
  SgSetIterator(const GoPointSet &set);
  
  void operator++();
  
  GoPoint operator*() const;
  
  operator bool() const;

 private:
  
  operator int() const;
  const GoPointSet &m_set;
  int m_index;
  void FindNext();
  int Size() const;
};

inline SgSetIterator::SgSetIterator(const GoPointSet &set)
    : m_set(set),
      m_index(0) {
  FindNext();
}

inline void SgSetIterator::operator++() {
  DBG_ASSERT(m_index < Size());
  FindNext();
}

inline GoPoint SgSetIterator::operator*() const {
  DBG_ASSERT(m_index <= Size());
  DBG_ASSERT_BOARDRANGE(m_index);
  DBG_ASSERT(m_set.m_a.test(m_index));
  return m_index;
}

inline SgSetIterator::operator bool() const {
  return m_index < Size();
}

inline void SgSetIterator::FindNext() {
  int size = Size();
  do {
    ++m_index;
  } while (m_index < size && !m_set.m_a.test(m_index));
}

inline int SgSetIterator::Size() const {
  return static_cast<int>(m_set.m_a.size());
}


class SgSimpleSet {
 public:
  SgSimpleSet();
  void Include(GoPoint p);
  void Exclude(GoPoint p);
  bool Contains(GoPoint p) const;
  void Clear();
  bool IsEmpty() const;
  bool NonEmpty() const;
  void GetPoints(GoPointSet *points) const;
  bool NewMark(GoPoint p);

 private:
  
  bool m_mark[GO_MAXPOINT];
};

inline SgSimpleSet::SgSimpleSet() {
  Clear();
}

inline void SgSimpleSet::Include(GoPoint p) {
  DBG_ASSERT_BOARDRANGE(p);
  m_mark[p] = true;
}

inline void SgSimpleSet::Exclude(GoPoint p) {
  DBG_ASSERT_BOARDRANGE(p);
  m_mark[p] = false;
}

inline bool SgSimpleSet::Contains(GoPoint p) const {
  return m_mark[p];
}

inline void SgSimpleSet::Clear() {
  std::memset(m_mark, 0, GO_MAXPOINT * sizeof(bool));
}

inline bool SgSimpleSet::IsEmpty() const {
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p)
    if (Contains(p))
      return false;
  return true;
}

inline bool SgSimpleSet::NonEmpty() const {
  return !IsEmpty();
}

inline void SgSimpleSet::GetPoints(GoPointSet *points) const {
  points->Clear();
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p)
    if (Contains(p))
      points->Include(p);
}

inline bool SgSimpleSet::NewMark(GoPoint p) {
  if (Contains(p))
    return false;
  else {
    Include(p);
    return true;
  }
}

#endif
