
#include "platform/SgSystem.h"
#include "SgIncrementalStack.h"

#include "board/GoBWSet.h"
#include "board/GoPointSet.h"

void SgIncrementalStack::PushPts(int type, SgEmptyBlackWhite col,
                                 const GoPointSet &pts)
{
  int nu = 0;
  for (SgSetIterator it(pts); it; ++it) {
    PushPoint(*it);
    ++nu;
  }
  PushInt(col);
  PushInt(nu);
  PushInt(type);
}

void SgIncrementalStack::PushPt(int type, SgEmptyBlackWhite col, GoPoint pt)
{
  PushPoint(pt);
  PushInt(col);
  PushInt(1);// nu pts
  PushInt(type);
}

void SgIncrementalStack::PushPtrEvent(int type, void *ptr) {
  PushPtr(ptr);
  PushInt(type);
}

void SgIncrementalStack::StartMoveInfo() {
  PushInt(SG_NEXTMOVE);
}

void SgIncrementalStack::Clear() {
  m_stack.Clear();
}

void SgIncrementalStack::SubtractPoints(GoPointSet *set) {
  int nu = PopInt();
  SgEmptyBlackWhite col = PopInt();
  SuppressUnused(col);
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    set->Exclude(p);
  }
}

void SgIncrementalStack::AddPoints(GoPointSet *set) {
  int nu = PopInt();
  SgEmptyBlackWhite col = PopInt();
  SuppressUnused(col);
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    set->Include(p);
  }
}

void SgIncrementalStack::SubtractPoints(GoBWSet *set) {
  int nu = PopInt();
  SgBlackWhite col = PopInt();
  GoPointSet &s = (*set)[col];
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    s.Exclude(p);
  }
}

void SgIncrementalStack::AddPoints(GoBWSet *set) {
  int nu = PopInt();
  SgBlackWhite col = PopInt();
  GoPointSet &s = (*set)[col];
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    s.Include(p);
  }
}

void SgIncrementalStack::SubtractAndAddPoints(GoBWSet *subtractset,
                                              GoBWSet *addset) {
  int nu = PopInt();
  SgBlackWhite col = PopInt();
  GoPointSet &s1 = (*subtractset)[col];
  GoPointSet &s2 = (*addset)[col];
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    s1.Exclude(p);
    s2.Include(p);
  }
}

void SgIncrementalStack::SubtractAndAddPoints(GoPointSet *subtractset,
                                              GoBWSet *addset) {
  int nu = PopInt();
  SgBlackWhite col = PopInt();
  GoPointSet &s1 = (*subtractset);
  GoPointSet &s2 = (*addset)[col];
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    s1.Exclude(p);
    s2.Include(p);
  }
}

void SgIncrementalStack::SubtractAndAddPoints(GoBWSet *subtractset,
                                              GoPointSet *addset) {
  int nu = PopInt();
  SgBlackWhite col = PopInt();
  GoPointSet &s1 = (*subtractset)[col];
  GoPointSet &s2 = (*addset);
  for (int i = 1; i <= nu; ++i) {
    GoPoint p = PopPoint();
    s1.Exclude(p);
    s2.Include(p);
  }
}
