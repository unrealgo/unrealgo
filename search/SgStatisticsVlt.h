#ifndef SG_STATISTICSVLT_H
#define SG_STATISTICSVLT_H

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "platform/SgException.h"
#include "board/SgWrite.h"

template<typename VALUE, typename COUNT>
class UctStatisticsVltBase {
 public:
  UctStatisticsVltBase();
  UctStatisticsVltBase(VALUE val, COUNT count);
  void Add(VALUE val);
  void Remove(VALUE val);
  void Add(VALUE val, COUNT n);
  void Remove(VALUE val, COUNT n);
  void Clear();
  COUNT Count() const;
  void Initialize(VALUE val, COUNT count);
  bool IsDefined() const;
  VALUE Mean() const;
  void Write(std::ostream &out) const;
  void SaveAsText(std::ostream &out) const;
  void LoadFromText(std::istream &in);

 private:
  bool is_exact;
  volatile COUNT m_count;
  volatile VALUE m_mean;
};

template<typename VALUE, typename COUNT>
inline UctStatisticsVltBase<VALUE, COUNT>::UctStatisticsVltBase() : is_exact(std::numeric_limits<COUNT>::is_exact) {
  Clear();
}

template<typename VALUE, typename COUNT>
inline UctStatisticsVltBase<VALUE, COUNT>::UctStatisticsVltBase(VALUE val, COUNT count)
    : is_exact(std::numeric_limits<COUNT>::is_exact),
      m_count(count),
      m_mean(val) {}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::Add(VALUE val) {
  COUNT count = m_count;
  ++count;
  DBG_ASSERT(!is_exact || count > 0);

  val -= m_mean;
  m_mean += val / VALUE(count);
  m_count = count;
}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::Remove(VALUE val) {
  COUNT count = m_count;
  if (count > 1) {
    --count;
    m_mean += (m_mean - val) / VALUE(count);
    m_count = count;
  } else
    Clear();
}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::Remove(VALUE val, COUNT n) {
  COUNT count = m_count;
  if (count > n) {
    count -= n;
    m_mean += VALUE(n) * (m_mean - val) / VALUE(count);
    m_count = count;
  } else
    Clear();
}


template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::Add(VALUE val, COUNT n) {
  // Write order dependency: at least one class (SgUctSearch in lock-free
  // mode) uses UctStatisticsVltBase concurrently without locking and assumes
  // that m_mean is valid, if m_count is greater zero
  COUNT count = m_count;
  count += n;
  DBG_ASSERT(!is_exact || count > 0); // overflow
  val -= m_mean;
  m_mean += VALUE(n) * val / VALUE(count);
  m_count = count;
}

template<typename VALUE, typename COUNT>
inline void UctStatisticsVltBase<VALUE, COUNT>::Clear() {
  m_count = 0;
  m_mean = 0;
}

template<typename VALUE, typename COUNT>
inline COUNT UctStatisticsVltBase<VALUE, COUNT>::Count() const {
  return m_count;
}

template<typename VALUE, typename COUNT>
inline void UctStatisticsVltBase<VALUE, COUNT>::Initialize(VALUE val, COUNT count) {
  DBG_ASSERT(count > 0);
  m_count = count;
  m_mean = val;
}

template<typename VALUE, typename COUNT>
inline bool UctStatisticsVltBase<VALUE, COUNT>::IsDefined() const {
  // if (std::numeric_limits<COUNT>::is_exact)
  if (is_exact)
    return m_count > 0;
  else
    return m_count > std::numeric_limits<COUNT>::epsilon();
}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::LoadFromText(std::istream &in) {
  in >> m_count >> m_mean;
}

template<typename VALUE, typename COUNT>
inline VALUE UctStatisticsVltBase<VALUE, COUNT>::Mean() const {
  DBG_ASSERT(IsDefined());
  return m_mean;
}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::Write(std::ostream &out) const {
  if (IsDefined())
    out << Mean();
  else
    out << '-';
}

template<typename VALUE, typename COUNT>
void UctStatisticsVltBase<VALUE, COUNT>::SaveAsText(std::ostream &out) const {
  out << m_count << ' ' << m_mean;
}

#endif // SG_STATISTICSVLT_H
