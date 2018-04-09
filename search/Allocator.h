
#ifndef UNREALGO_ALLOCATOR_H
#define UNREALGO_ALLOCATOR_H

#include "board/GoPoint.h"

class UctPolicyAllocator {
 public:
  UctPolicyAllocator();
  ~UctPolicyAllocator();
  void Clear();
  bool HasCapacity(std::size_t n) const;
  void SetMaxPolicies(std::size_t maxPolicies);
  const float *Start() const;
  float *Finish();
  const float *Finish() const;
  float *CreateOneBlock(float value = 0.0f);

 private:
  float *m_start;

  float *m_finish;
  float *m_endOfStorage;
  int m_blockSize;

  UctPolicyAllocator &operator=(const UctPolicyAllocator &tree) = delete;
};

inline void UctPolicyAllocator::Clear() {
  m_finish = m_start;
}

inline UctPolicyAllocator::UctPolicyAllocator() {
  m_start = 0;
  m_finish = 0;
  m_endOfStorage = 0;
  m_blockSize = GO_MAX_MOVES;
}
inline bool UctPolicyAllocator::HasCapacity(std::size_t n) const {
  return m_finish + n * m_blockSize < m_endOfStorage;
}

inline const float *UctPolicyAllocator::Start() const {
  return m_start;
}

inline float *UctPolicyAllocator::Finish() {
  return m_finish;
}

inline const float *UctPolicyAllocator::Finish() const {
  return m_finish;
}

inline float *UctPolicyAllocator::CreateOneBlock(float value) {
  DBG_ASSERT(HasCapacity(1));
  float *addr = m_finish;
  for (int i = 0; i < m_blockSize; i++)
    addr[i] = value;
  m_finish += m_blockSize;
  return addr;
}

#endif
