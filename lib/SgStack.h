//----------------------------------------------------------------------------
/** @file SgStack.h
    Stack class. */
//----------------------------------------------------------------------------

#ifndef SG_STACK_H
#define SG_STACK_H

#include <algorithm>

template<class T, int SIZE>
class SgStack {
 public:
  SgStack()
      : m_sp(0) {}

  ~SgStack() {}

  void Clear();
  void CopyFrom(const SgStack<T, SIZE> &other);
  bool IsEmpty() const;
  bool NonEmpty() const;
  T Pop();
  void Push(T data);
  void PushAll(const SgStack<T, SIZE> &other);
  int Size() const;
  void SwapWith(SgStack<T, SIZE> &other);
  const T &Top() const;
  const T &operator[](int index) const;

 private:
  int m_sp;
  T m_stack[SIZE];

  SgStack(const SgStack &) = delete;
  SgStack &operator=(const SgStack &) = delete;
};


template<typename T, int SIZE>
void SgStack<T, SIZE>::Clear() {
  m_sp = 0;
}

template<typename T, int SIZE>
void SgStack<T, SIZE>::CopyFrom(const SgStack<T, SIZE> &other) {
  for (int i = 0; i < other.Size(); ++i)
    m_stack[i] = other.m_stack[i];
  m_sp = other.m_sp;
}

template<typename T, int SIZE>
bool SgStack<T, SIZE>::IsEmpty() const {
  return m_sp == 0;
}

template<typename T, int SIZE>
bool SgStack<T, SIZE>::NonEmpty() const {
  return m_sp != 0;
}

template<typename T, int SIZE>
T SgStack<T, SIZE>::Pop() {
  DBG_ASSERT(0 < m_sp);
  return m_stack[--m_sp];
}

template<typename T, int SIZE>
void SgStack<T, SIZE>::Push(T data) {
  DBG_ASSERT(m_sp < SIZE);
  m_stack[m_sp++] = data;
}

template<typename T, int SIZE>
void SgStack<T, SIZE>::PushAll(const SgStack<T, SIZE> &other) {
  for (int i = 0; i < other.Size(); ++i)
    Push(other.m_stack[i]);
}

template<typename T, int SIZE>
int SgStack<T, SIZE>::Size() const {
  return m_sp;
}

template<typename T, int SIZE>
void SgStack<T, SIZE>::SwapWith(SgStack<T, SIZE> &other) {
  int nuSwap = std::min(Size(), other.Size());
  for (int i = 0; i < nuSwap; ++i)
    std::swap(m_stack[i], other.m_stack[i]);
  if (Size() < other.Size())
    for (int i = Size(); i < other.Size(); ++i)
      m_stack[i] = other.m_stack[i];
  else if (other.Size() < Size())
    for (int i = other.Size(); i < Size(); ++i)
      other.m_stack[i] = m_stack[i];
  std::swap(m_sp, other.m_sp);
}

template<typename T, int SIZE>
const T &SgStack<T, SIZE>::Top() const {
  DBG_ASSERT(0 < m_sp);
  return m_stack[m_sp - 1];
}

template<typename T, int SIZE>
const T &SgStack<T, SIZE>::operator[](int index) const {
  DBG_ASSERT(index >= 0);
  DBG_ASSERT(index < m_sp);
  return m_stack[index];
}

#endif // SG_STACK_H
