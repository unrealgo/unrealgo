
#ifndef SG_INCREMENTALSTACK_H
#define SG_INCREMENTALSTACK_H

#include "board/GoBoardColor.h"
#include "lib/SgVector.h"
#include "board/GoPoint.h"

class GoPointSet;
class GoBWSet;

enum SgIncrementalStackEvent {
  SG_NEW_POINTS = 1000,
  SG_ADD_EMPTY = 1001,
  SG_NEW_SAFE = 1002,
  SG_NEW_UNSAFE = 1003,
  SG_UNSAFE_TO_SAFE = 1004,
  SG_CAPTURES = 1005,
  SG_INCREMENTAL_MOVE = 1006,
  SG_NEXTMOVE = 1007,
  SG_UNSAFE_TO_HALF_SAFE = 1008,
  SG_CAPTURE_HALF_SAFE = 1009
};

class SgIncrementalStack {
 public:
  SgIncrementalStack() {}

  void Clear();

  bool IsEmpty() const {
    return m_stack.IsEmpty();
  }

  void PushPts(int type, SgEmptyBlackWhite col, const GoPointSet &pts);
  void PushPt(int type, SgEmptyBlackWhite col, GoPoint pt);

  void PushPtr(void *ptr) {
    m_stack.PushBack(IntOrPtr(ptr));
  }

  void PushPtrEvent(int type, void *ptr);

  void PushInt(int i) {
    m_stack.PushBack(IntOrPtr(i));
  }


  void PushPoint(GoPoint p) {
    m_stack.PushBack(IntOrPtr(p));
  }

  void StartMoveInfo();

  SgIncrementalStackEvent PopEvent() {
    return static_cast<SgIncrementalStackEvent>(PopInt());
  }

  void *PopPtr() {
    void *p = m_stack.Back().m_ptr;
    m_stack.PopBack();
    return p;
  }

  int PopInt() {
    int i = m_stack.Back().m_int;
    m_stack.PopBack();
    return i;
  }


  GoPoint PopPoint() {
    return PopInt();
  }

  void SubtractPoints(GoPointSet *set);
  void AddPoints(GoPointSet *set);
  void SubtractPoints(GoBWSet *set);
  void AddPoints(GoBWSet *set);
  void SubtractAndAddPoints(GoBWSet *subtractset, GoBWSet *addset);
  void SubtractAndAddPoints(GoPointSet *subtractset, GoBWSet *addset);
  void SubtractAndAddPoints(GoBWSet *subtractset, GoPointSet *addset);

 private:

  union IntOrPtr {
    IntOrPtr() {}

    IntOrPtr(int i) {
      m_int = i;
    }

    IntOrPtr(void *ptr) {
      m_ptr = ptr;
    }

    int m_int;
    void *m_ptr;
  };

  SgVector<IntOrPtr> m_stack;
};

//----------------------------------------------------------------------------

#endif // SG_INCREMENTALSTACK_H
