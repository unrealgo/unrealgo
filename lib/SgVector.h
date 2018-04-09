
#ifndef SG_VECTOR_H
#define SG_VECTOR_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <vector>

#include "platform/SgSystem.h"

template<typename T>
class SgVector {
 public:
  SgVector()
      : m_vec() {}

  T& operator[](int index) {
    return m_vec[index];
  }

  const T& operator[](int index) const {
    return m_vec[index];
  }

  SgVector<T>& operator=(const SgVector<T>& v);

  bool operator==(const SgVector<T>& rhs) const {
    return m_vec == rhs.m_vec;
  }

  bool operator!=(const SgVector& rhs) const {
    return !(*this == rhs);
  }

  const T& Back() const {
    DBG_ASSERT(NonEmpty());
    return m_vec.back();
  }

  T BackAndPop() {
    DBG_ASSERT(NonEmpty());
    T back = m_vec.back();
    PopBack();
    return back;
  }

  void Clear() {
    m_vec.clear();
  }

  void Concat(SgVector<T>* tail);
  bool Contains(const T& elt) const;
  void DeleteAt(int index);
  bool Exclude(const T& elt);
  void Exclude(const SgVector<T>& vector);

  const T& Front() const {
    DBG_ASSERT(NonEmpty());
    return m_vec.front();
  }

  int Index(const T& elt) const;

  void Include(const T& elt) {
    if (!Contains(elt))
      PushBack(elt);
  }

  bool Insert(const T& elt);

  bool IsEmpty() const {
    return m_vec.empty();
  }

  bool IsLength(int length) const {
    return Length() == length;
  }

  bool IsSorted(bool ascending = true) const;
  bool IsSortedAndUnique(bool ascending = true) const;

  int Length() const {
    return static_cast<int>(m_vec.size());
  }

  void LimitListLength(int limit);

  bool MaxLength(int length) const {
    return Length() <= length;
  }

  void Merge(const SgVector<T>& vector);

  bool MinLength(int length) const {
    return Length() >= length;
  }

  bool NonEmpty() const {
    return !IsEmpty();
  }

  T PopFront();
  void PopBack();
  void PushFront(const T& elt);

  void PushBack(const T& elt) {
    DBG_ASSERT(Length() < std::numeric_limits<int>::max());
    m_vec.push_back(elt);
  }

  void PushBackList(const SgVector<T>& vector);
  bool RemoveDuplicates();

  void Reverse() {
    reverse(m_vec.begin(), m_vec.end());
  }

  void SetTo(const T& elt) {
    Clear();
    PushBack(elt);
  }

  bool SetsAreEqual(const SgVector<T>& other) const;
  void SetTo(const T* array, int count);
  void Sort();
  void SortedRemoveDuplicates();

  void SwapWith(SgVector<T>* vector) {
    std::swap(m_vec, vector->m_vec);
  }

  const T& TopNth(int index) const {
    DBG_ASSERT(NonEmpty());
    DBG_ASSERT(index >= 1);
    DBG_ASSERT(index <= static_cast<int>(m_vec.size()));
    return m_vec[m_vec.size() - index];
  }

  void Union(const SgVector<T>& set);
  bool UniqueElements() const;

  std::vector<T>& Vector() {
    return m_vec;
  }

  const std::vector<T>& Vector() const {
    return m_vec;
  }

 private:
  std::vector<T> m_vec;
};
template<typename T>
class SgVectorIterator {
 public:
  SgVectorIterator(const SgVector<T>& vec)
      : m_vec(vec),
        m_it(m_vec.Vector().begin()) {}

  SgVectorIterator(const SgVectorIterator& it)
      : m_vec(it.m_vec) {}

  virtual ~SgVectorIterator() {}

  SgVectorIterator& operator++() {
    ++m_it;
    return *this;
  }

  const T& operator*() const {
    DBG_ASSERT(*this);
    return *m_it;
  };

  operator bool() const {
    return m_it != m_vec.Vector().end(); // @todo cache end.
  }

 private:
  const SgVector<T>& m_vec;
  typename std::vector<T>::const_iterator m_it;
  operator int() const;
  SgVectorIterator& operator=(const SgVectorIterator&);
};

template<class T>
class SgVectorOf
    : public SgVector<void*> {
 public:

  T* operator[](int index) const {
    return static_cast<T*>(SgVector<void*>::operator[](index));
  }

  T* Back() const {
    return static_cast<T*>(SgVector<void*>::Back());
  }

  bool Contains(const T* element) const {
    DBG_ASSERT(element);
    return SgVector<void*>::Contains(GetVoidPtr(element));
  }

  void Include(const T* element) {
    DBG_ASSERT(element);
    if (!Contains(element))
      PushBack(element);
  }

  bool Exclude(const T* element) {
    return SgVector<void*>::Exclude(GetVoidPtr(element));
  }

  void Exclude(const SgVectorOf<T>& vector) {
    SgVector<void*>::Exclude(vector);
  }

  T* Front() const {
    return static_cast<T*>(SgVector<void*>::Front());
  }

  bool Insert(const T* element) {
    return SgVector<void*>::Insert(GetVoidPtr(element));
  }

  void PushFront(const T* element) {
    DBG_ASSERT(element);
    SgVector<void*>::PushFront(GetVoidPtr(element));
  }

  void PushBack(const T* element) {
    DBG_ASSERT(element);
    SgVector<void*>::PushBack(GetVoidPtr(element));
  }

  T* PopFront() {
    return static_cast<T*>(SgVector<void*>::PopFront());
  }

#if UNUSED
  bool Extract(const T* element)
  {
      return SgVector<void*>::Extract(GetVoidPtr(element));
  }

  // The following are defined below since they use SgVectorIteratorOf

  bool ContainsContent(const T& element) const;

  void RemoveDuplicateContent();
#endif
 private:
  static void* GetVoidPtr(const T* element) {
    return const_cast<void*>(static_cast<const void*>(element));
  }
};
template<class T>
class SgVectorIteratorOf
    : private SgVectorIterator<void*> {
 public:
  SgVectorIteratorOf(const SgVectorOf<T>& vector)
      : SgVectorIterator<void*>(static_cast<const SgVector<void*>&>(vector)) {}

  void operator++() {
    SgVectorIterator<void*>::operator++();
  }

  T* operator*() const {
    return static_cast<T*>(SgVectorIterator<void*>::operator*());
  }

  operator bool() const {
    return SgVectorIterator<void*>::operator bool();
  }

 private:
  operator int() const = delete;
};

template<typename T>
SgVector<T>& SgVector<T>::operator=(const SgVector<T>& v) {
  if (this != &v) {
    Clear();
    PushBackList(v);
  }
  return *this;
}

template<typename T>
void SgVector<T>::PushBackList(const SgVector<T>& v) {
  copy(v.m_vec.begin(), v.m_vec.end(), back_inserter(m_vec));
}

template<typename T>
void SgVector<T>::Concat(SgVector<T>* tail) {
  PushBackList(*tail);
  tail->Clear();
}

template<typename T>
bool SgVector<T>::Contains(const T& elt) const {
  typename std::vector<T>::const_iterator end = m_vec.end();
  typename std::vector<T>::const_iterator pos = find(m_vec.begin(),
                                                     end, elt);
  return pos != end;
}

template<typename T>
void SgVector<T>::DeleteAt(int index) {
  DBG_ASSERT(index >= 0);
  DBG_ASSERT(index < Length());
  m_vec.erase(m_vec.begin() + index);
}

template<typename T>
bool SgVector<T>::Exclude(const T& elt) {
  typename std::vector<T>::iterator end = m_vec.end();
  typename std::vector<T>::iterator pos = find(m_vec.begin(), end, elt);
  if (pos != end) {
    m_vec.erase(pos);
    return true;
  }
  return false;
}

template<typename T>
void SgVector<T>::Exclude(const SgVector<T>& vector) {
  for (SgVectorIterator<T> it(vector); it; ++it)
    Exclude(*it);
}

template<typename T>
int SgVector<T>::Index(const T& elt) const {
  typename std::vector<T>::const_iterator end = m_vec.end();
  typename std::vector<T>::const_iterator pos = find(m_vec.begin(), end, elt);
  if (pos == end)
    return -1;
  else
    return int(pos - m_vec.begin());
}

template<typename T>
bool SgVector<T>::Insert(const T& elt) {
  DBG_ASSERT(IsSorted());
  typename std::vector<T>::iterator location =
      lower_bound(m_vec.begin(), m_vec.end(), elt);

  if (location != m_vec.end()
      && *location == elt
      )
    return false;
  else {
    m_vec.insert(location, elt);
    DBG_ASSERT(IsSorted());
  }
  return true;
}

template<typename T>
bool SgVector<T>::IsSorted(bool ascending) const {
  typename std::vector<T>::const_iterator result;
  if (ascending)
    result = adjacent_find(m_vec.begin(), m_vec.end(), std::greater<T>());
  else
    result = adjacent_find(m_vec.begin(), m_vec.end(), std::less<T>());
  return result == m_vec.end();
}

template<typename T>
bool SgVector<T>::IsSortedAndUnique(bool ascending) const {
  typename std::vector<T>::const_iterator result;
  if (ascending)
    result = adjacent_find(m_vec.begin(), m_vec.end(),
                           std::greater_equal<T>());
  else
    result = adjacent_find(m_vec.begin(), m_vec.end(),
                           std::less_equal<T>());
  return result == m_vec.end();
}

template<typename T>
void SgVector<T>::LimitListLength(int limit) {
  if (Length() > limit)
    m_vec.resize(limit);
}

template<typename T>
void SgVector<T>::Merge(const SgVector<T>& vector) {
  DBG_ASSERT(IsSortedAndUnique());
  DBG_ASSERT(vector.IsSortedAndUnique());
  if ((this == &vector) || vector.IsEmpty())
    return;
  else if (IsEmpty() || vector.Front() > Back())
    PushBackList(vector);
  else {
    const int oldSize = Length();
    PushBackList(vector);
    inplace_merge(m_vec.begin(), m_vec.begin() + oldSize, m_vec.end());
    SortedRemoveDuplicates();
  }
  DBG_ASSERT(IsSortedAndUnique());
}

template<typename T>
T SgVector<T>::PopFront() {
  DBG_ASSERT(NonEmpty());
  T elt = Front();
  m_vec.erase(m_vec.begin());
  return elt;
}

template<typename T>
void SgVector<T>::PopBack() {
  DBG_ASSERT(NonEmpty());
  m_vec.pop_back();
}

template<typename T>
void SgVector<T>::PushFront(const T& elt) {
  m_vec.insert(m_vec.begin(), elt);
}

template<typename T>
bool SgVector<T>::SetsAreEqual(const SgVector<T>& other) const {
  if (!IsLength(other.Length()))
    return false;

  for (SgVectorIterator<T> it1(*this); it1; ++it1) {
    if (!other.Contains(*it1))
      return false;
  }
  for (SgVectorIterator<T> it2(other); it2; ++it2) {
    if (!Contains(*it2))
      return false;
  }
  return true;
}

template<typename T>
void SgVector<T>::SetTo(const T* array, int count) {
  m_vec.assign(array, array + count);
  DBG_ASSERT(IsLength(count));
}

template<typename T>
void SgVector<T>::Sort() {
  sort(m_vec.begin(), m_vec.end());
}

template<typename T>
void SgVector<T>::Union(const SgVector<T>& set) {
  for (SgVectorIterator<T> it(set); it; ++it)
    Include(*it);
}

template<typename T>
bool SgVector<T>::RemoveDuplicates() {
  // @todo n^2; could be made much faster with tags
  SgVector<T> uniqueVector;
  for (SgVectorIterator<T> it(*this); it; ++it)
    if (!uniqueVector.Contains(*it))
      uniqueVector.PushBack(*it);
  SwapWith(&uniqueVector); // avoid copying
  DBG_ASSERT(UniqueElements());
  return uniqueVector.Length() != Length();
}

template<typename T>
void SgVector<T>::SortedRemoveDuplicates() {
  DBG_ASSERT(IsSorted());
  if (IsEmpty())
    return;
  int prev = 0;
  bool shifted = false;
  for (int i = 1; i < Length(); ++i) {
    if (m_vec[i] != m_vec[prev]) {
      ++prev;
      if (shifted)
        m_vec[prev] = m_vec[i];
    } else shifted = true;
  }
  if (shifted)
    LimitListLength(prev + 1);
  DBG_ASSERT(IsSortedAndUnique());
}

template<typename T>
bool SgVector<T>::UniqueElements() const {
  // @todo n^2; could be made much faster with tags
  if (MinLength(2)) {
    if (IsSorted())
      return IsSortedAndUnique();
    else
      for (int i = 0; i < Length() - 1; ++i)
        for (int j = i + 1; j < Length(); ++j)
          if (m_vec[i] == m_vec[j])
            return false;
  }
  return true;
}

template<typename T>
class SgVectorPairIterator {
 public:
  SgVectorPairIterator(const SgVector<T>& vector);

  virtual ~SgVectorPairIterator() {}

  bool NextPair(T& elt1, T& elt2);

 private:
  const SgVector<T>& m_vector;
  int m_index1;
  int m_index2;
};

template<typename T>
SgVectorPairIterator<T>::SgVectorPairIterator(const SgVector<T>& vector)
    : m_vector(vector), m_index1(0), m_index2(1) {

}

template<typename T>
bool SgVectorPairIterator<T>::NextPair(T& elt1, T& elt2) {
  if (m_index1 >= m_vector.Length() - 1)
    return false;
  elt1 = m_vector[m_index1];
  elt2 = m_vector[m_index2];
  if (++m_index2 == m_vector.Length()) {
    ++m_index1;
    m_index2 = m_index1 + 1;
  }
  return true;
}

template<class T>
class SgVectorPairIteratorOf
    : public SgVectorPairIterator<void*> {
 public:
  SgVectorPairIteratorOf(const SgVectorOf<T>& list)
      : SgVectorPairIterator<void*>
            (static_cast<const SgVector<void*>&>(list)) {}

  bool NextPair(T*& elt1, T*& elt2) {
    void* voidPtr1;
    void* voidPtr2;
    if (SgVectorPairIterator<void*>::NextPair(voidPtr1, voidPtr2)) {
      elt1 = static_cast<T*>(voidPtr1);
      elt2 = static_cast<T*>(voidPtr2);
      return true;
    }
    return false;
  }
};

template<typename T>
void FreeAll(SgVectorOf<T>& objects) {
  for (SgVectorIteratorOf<T> it(objects); it; ++it)
    delete *it;
  objects.Clear();
}

#endif // SG_VECTOR_H
