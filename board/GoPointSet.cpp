
#include "platform/SgSystem.h"
#include "board/GoPointSet.h"

#include <algorithm>
#include <iostream>
#include "board/GoNbIterator.h"

using namespace std;


namespace {

int MaxDistance(GoPoint p1, GoPoint p2) {
  return max(abs(GoPointUtil::Row(p1) - GoPointUtil::Row(p2)),
             abs(GoPointUtil::Col(p1) - GoPointUtil::Col(p2)));
}

}


GoPointSet::PrecompAllPoints::PrecompAllPoints() {
  for (int size = GO_MIN_SIZE; size <= GO_MAX_SIZE; ++size) {
    m_allPoints[size].reset(new GoPointSet());
    for (int col = 1; col <= size; ++col)
      for (int row = 1; row <= size; ++row)
        m_allPoints[size]->Include(GoPointUtil::Pt(col, row));
  }
}


GoPointSet::PrecompAllPoints GoPointSet::s_allPoints;

GoPointSet GoPointSet::Border(int boardSize) const {
  return (BorderNoClip() & AllPoints(boardSize));
}

GoPointSet GoPointSet::BorderNoClip() const {
  GoPointSet bd = (*this >> GO_NORTH_SOUTH);
  bd |= (*this << GO_NORTH_SOUTH);
  bd |= (*this >> GO_WEST_EAST);
  bd |= (*this << GO_WEST_EAST);
  bd -= (*this);
  return bd;
}

GoPointSet GoPointSet::Component(GoPoint p) const {
  GoPointSet set1, set2;
  set1.Include(p);
  GoPointSet *a = &set1, *b = &set2;
  do {
    *b = *a | (a->BorderNoClip() & (*this));
    swap(a, b);
  } while (set1 != set2);
  return set1;
}

GoPointSet GoPointSet::ConnComp(GoPoint p) const {
  GoPointSet out, in = (*this);
  GoPoint stack[GO_MAXPOINT];
  out.Include(p);
  in.Exclude(p);
  int current = 0;
  stack[current] = p;
  while (current >= 0) {
    GoPoint q = stack[current--];
    for (SgNb4Iterator it(q); it; ++it) {
      GoPoint nb = *it;
      if (in.Contains(nb)) {
        out.Include(nb);
        in.Exclude(nb);
        stack[++current] = nb;
      }
    }
  }
  return out;
}

GoPointSet GoPointSet::ConnComp8(GoPoint p) const {
  DBG_ASSERT(Contains(p));
  GoPointSet out, in = (*this);
  GoPoint stack[GO_MAXPOINT];
  out.Include(p);
  in.Exclude(p);
  int current = 0;
  stack[current] = p;
  while (current >= 0) {
    GoPoint q = stack[current--];
    for (SgNb8Iterator it(q); it; ++it) {
      GoPoint nb = *it;
      if (in.Contains(nb)) {
        out.Include(nb);
        in.Exclude(nb);
        stack[++current] = nb;
      }
    }
  }
  DBG_ASSERT(SupersetOf(out));
  return out;
}

GoPointSet &GoPointSet::Exclude(const SgVector<GoPoint> &vector) {
  for (SgVectorIterator<GoPoint> it(vector); it; ++it)
    Exclude(*it);
  return (*this);
}

GoPointSet::GoPointSet(const SgVector<GoPoint> &vector) {
  Clear();
  for (SgVectorIterator<GoPoint> it(vector); it; ++it)
    Include(*it);
}

void GoPointSet::ToVector(SgVector<GoPoint> *list) const {
  list->Clear();
  for (SgSetIterator si(*this); si; ++si)
    list->PushBack(*si);
}

void GoPointSet::Write(ostream &out, int boardSize) const {
  for (int row = boardSize; row >= 1; --row) {
    for (int col = 1; col <= boardSize; ++col)
      out << (Contains(GoPointUtil::Pt(col, row)) ? '@' : '-');
    out << '\n';
  }
}

void GoPointSet::Grow(int boardSize) {
  GoPointSet bd = (*this >> GO_NORTH_SOUTH);
  bd |= (*this << GO_NORTH_SOUTH);
  bd |= (*this >> GO_WEST_EAST);
  bd |= (*this << GO_WEST_EAST);
  bd &= AllPoints(boardSize);
  *this |= bd;
}

void GoPointSet::Grow(GoPointSet *newArea, int boardSize) {
  *newArea = (*this >> GO_NORTH_SOUTH);
  *newArea |= (*this << GO_NORTH_SOUTH);
  *newArea |= (*this >> GO_WEST_EAST);
  *newArea |= (*this << GO_WEST_EAST);
  *newArea &= AllPoints(boardSize);
  *newArea ^= (*this);
  *this |= *newArea;
}

void GoPointSet::Grow8(int boardSize) {
  GoPointSet bd = (*this >> GO_NORTH_SOUTH);
  bd |= (*this << GO_NORTH_SOUTH);
  bd |= (*this >> GO_WEST_EAST);
  bd |= (*this << GO_WEST_EAST);
  bd |= (*this >> (GO_NORTH_SOUTH + GO_WEST_EAST));
  bd |= (*this << (GO_NORTH_SOUTH + GO_WEST_EAST));
  bd |= (*this >> (GO_NORTH_SOUTH - GO_WEST_EAST));
  bd |= (*this << (GO_NORTH_SOUTH - GO_WEST_EAST));
  bd &= AllPoints(boardSize);
  *this |= bd;
}

GoPointSet GoPointSet::Border8(int boardSize) const {
  GoPointSet bd = (*this >> GO_NORTH_SOUTH);
  bd |= (*this << GO_NORTH_SOUTH);
  bd |= (*this >> GO_WEST_EAST);
  bd |= (*this << GO_WEST_EAST);
  bd |= (*this >> (GO_NORTH_SOUTH + GO_WEST_EAST));
  bd |= (*this << (GO_NORTH_SOUTH + GO_WEST_EAST));
  bd |= (*this >> (GO_NORTH_SOUTH - GO_WEST_EAST));
  bd |= (*this << (GO_NORTH_SOUTH - GO_WEST_EAST));
  bd -= (*this);
  bd &= AllPoints(boardSize);
  return bd;
}

bool GoPointSet::IsSize(int size) const {
  DBG_ASSERT(size >= 0);
  return (Size() == size);
}

bool GoPointSet::MinSetSize(int size) const {
  return Size() >= size;
}

bool GoPointSet::MaxSetSize(int size) const {
  return Size() <= size;
}

bool GoPointSet::Adjacent(const GoPointSet &s) const {
  return BorderNoClip().Overlaps(s);
}

bool GoPointSet::AllAdjacentTo(const GoPointSet &s) const {
  return SubsetOf(s.BorderNoClip());
}

bool GoPointSet::AdjacentOnlyTo(const GoPointSet &s, int boardSize) const {
  return Border(boardSize).SubsetOf(s);
}

bool GoPointSet::IsCloseTo(GoPoint p) const {
  const int MAX_CLOSE_DISTANCE = 3;
  if (Contains(p))
    return true;

  for (SgSetIterator it(*this); it; ++it) {
    if (MaxDistance(*it, p) <= MAX_CLOSE_DISTANCE)
      return true;
  }
  return false;
}

GoPointSet GoPointSet::Kernel(int boardSize) const {
  GoPointSet k = AllPoints(boardSize) - (*this);
  GoPointSet bd = (k >> GO_NORTH_SOUTH);
  bd |= (k << GO_NORTH_SOUTH);
  bd |= (k >> GO_WEST_EAST);
  bd |= (k << GO_WEST_EAST);
  return (*this) - bd;
}

GoPoint GoPointSet::PointOf() const {
  SgSetIterator it(*this);
  if (it)
    return *it;
  else
    return GO_NULLPOINT;
}

GoPoint GoPointSet::Center() const {
  GoRect rect = EnclosingRect();
  if (rect.IsEmpty())
    return GO_NULLPOINT;

  GoPoint idealCenter = rect.Center();

  if (Contains(idealCenter))
    return idealCenter;

  int minDist = 4 * GO_MAX_SIZE;
  GoPoint center = GO_NULLPOINT;
  for (SgSetIterator it(*this); it; ++it) {
    int dist =
        MaxDistance(*it, idealCenter)
            + GoPointUtil::Distance(*it, idealCenter);
    if (dist < minDist) {
      center = *it;
      minDist = dist;
    }
  }
  return center;
}

GoRect GoPointSet::EnclosingRect() const {
  GoRect r;
  for (SgSetIterator it(*this); it; ++it)
    r.Include(*it);
  return r;
}

bool GoPointSet::IsConnected() const {
  GoPoint p = PointOf();
  if (p == GO_NULLPOINT)
    return true;
  else
    return Component(p) == *this;
}

bool GoPointSet::Is8Connected() const {
  GoPoint p = PointOf();
  if (p == GO_NULLPOINT)
    return true;
  else
    return ConnComp8(p) == *this;
}

