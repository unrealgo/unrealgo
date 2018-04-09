
#include <string>
#include "platform/SgSystem.h"
#include "Allocator.h"

void UctPolicyAllocator::SetMaxPolicies(std::size_t maxPolicies) {
  if (m_start != 0) {
    std::free(m_start);
  }
  void *ptr = std::malloc(maxPolicies * m_blockSize * sizeof(float));
  if (ptr == 0)
    throw std::bad_alloc();
  m_start = static_cast<float *>(ptr);
  m_finish = m_start;
  m_endOfStorage = m_start + maxPolicies * m_blockSize;
}

UctPolicyAllocator::~UctPolicyAllocator() {
  if (m_start != 0) {
    std::free(m_start);
  }
}