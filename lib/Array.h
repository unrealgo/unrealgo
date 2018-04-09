//----------------------------------------------------------------------------

#ifndef SG_ARRAY_H
#define SG_ARRAY_H

#include <algorithm>
#include <cstring>


template<typename T, int SIZE>
class GoArray;

template<typename T, int SIZE>
class GoArrayAssign {
 public:
  static void Assign(T *dest, const T *src);
};
template<int SIZE>
class GoArrayAssign<int, SIZE> {
 public:
  static void Assign(int *dest, const int *src);
};
template<int SIZE>
class GoArrayAssign<bool, SIZE> {
 public:
  static void Assign(bool *dest, const bool *src);
};
template<typename T, int SIZE>
class GoArrayAssign<T *, SIZE> {
 public:
  static void Assign(T **dest, T *const *src);
};

template<typename T, int SIZE>
void GoArrayAssign<T, SIZE>::Assign(T *dest, const T *src) {
  DBG_ASSERT(dest != src);
  T *p = dest;
  const T *pp = src;
  for (int i = SIZE; i--; ++p, ++pp)
    *p = *pp;
}

template<int SIZE>
void GoArrayAssign<int, SIZE>::Assign(int *dest, const int *src) {
  DBG_ASSERT(dest != src);
  std::memcpy(dest, src, SIZE * sizeof(int));
}

template<int SIZE>
void GoArrayAssign<bool, SIZE>::Assign(bool *dest, const bool *src) {
  DBG_ASSERT(dest != src);
  std::memcpy(dest, src, SIZE * sizeof(bool));
}

template<typename T, int SIZE>
void GoArrayAssign<T *, SIZE>::Assign(T **dest, T *const *src) {
  DBG_ASSERT(dest != src);
  std::memcpy(dest, src, SIZE * sizeof(T *));
}

template<typename T, int SIZE>
class GoArray {
 public:
  class Iterator {
   public:
    Iterator(const GoArray &array);
    const T &operator*() const;
    void operator++();
    operator bool() const;

   private:
    const T *m_end;
    const T *m_current;
  };
  
  class NonConstIterator {
   public:
    NonConstIterator(GoArray &array);
    T &operator*() const;
    void operator++();
    operator bool() const;

   private:
    const T *m_end;
    T *m_current;
  };
  GoArray();
  GoArray(const GoArray &array);
  explicit GoArray(const T &val);
  GoArray &operator=(const GoArray &array);
  T &operator[](int index);
  const T &operator[](int index) const;
  GoArray &operator*=(T val);
  void Fill(const T &val);
  
  T MinValue() const;
  T MaxValue() const;
  std::size_t Bytes() const;

 private:
  friend class Iterator;
  friend class NonConstIterator;
  T m_array[SIZE];
};

template<typename T, int SIZE>
GoArray<T, SIZE>::Iterator::Iterator(const GoArray &array)
    : m_end(array.m_array + SIZE),
      m_current(array.m_array) {}

template<typename T, int SIZE>
const T &GoArray<T, SIZE>::Iterator::operator*() const {
  DBG_ASSERT(*this);
  return *m_current;
}

template<typename T, int SIZE>
void GoArray<T, SIZE>::Iterator::operator++() {
  ++m_current;
}

template<typename T, int SIZE>
GoArray<T, SIZE>::Iterator::operator bool() const {
  return m_current < m_end;
}

template<typename T, int SIZE>
GoArray<T, SIZE>::NonConstIterator::NonConstIterator(GoArray &array)
    : m_end(array.m_array + SIZE),
      m_current(array.m_array) {}

template<typename T, int SIZE>
T &GoArray<T, SIZE>::NonConstIterator::operator*() const {
  DBG_ASSERT(*this);
  return *m_current;
}

template<typename T, int SIZE>
void GoArray<T, SIZE>::NonConstIterator::operator++() {
  ++m_current;
}

template<typename T, int SIZE>
GoArray<T, SIZE>::NonConstIterator::operator bool() const {
  return m_current < m_end;
}

template<typename T, int SIZE>
GoArray<T, SIZE>::GoArray() {}

template<typename T, int SIZE>
GoArray<T, SIZE>::GoArray(const GoArray &array) {
  DBG_ASSERT(&array != this);
  *this = array;
}

template<typename T, int SIZE>
GoArray<T, SIZE>::GoArray(const T &val) {
  Fill(val);
}

template<typename T, int SIZE>
GoArray<T, SIZE> &GoArray<T, SIZE>::operator=(const GoArray &array) {
  DBG_ASSERT(&array != this);
  GoArrayAssign<T, SIZE>::Assign(m_array, array.m_array);
  return *this;
}

template<typename T, int SIZE>
T &GoArray<T, SIZE>::operator[](int index) {
  DBG_ASSERT(index >= 0);
  DBG_ASSERT(index < SIZE);
  return m_array[index];
}

template<typename T, int SIZE>
const T &GoArray<T, SIZE>::operator[](int index) const {
  DBG_ASSERT(index >= 0);
  DBG_ASSERT(index < SIZE);
  return m_array[index];
}

template<typename T, int SIZE>
GoArray<T, SIZE> &GoArray<T, SIZE>::operator*=(T val) {
  T *p = m_array;
  for (int i = SIZE; i--; ++p)
    *p *= val;
  return *this;
}

template<typename T, int SIZE>
void GoArray<T, SIZE>::Fill(const T &val) {
  T *v = m_array;
  for (int i = SIZE; i--; ++v)
    *v = val;
}

template<typename T, int SIZE>
T GoArray<T, SIZE>::MaxValue() const {
  return *std::max_element(m_array, m_array + SIZE);
}

template<typename T, int SIZE>
std::size_t GoArray<T, SIZE>::Bytes() const {
  return SIZE * sizeof(T);
}

template<typename T, int SIZE>
T GoArray<T, SIZE>::MinValue() const {
  return *std::min_element(m_array, m_array + SIZE);
}


#endif
