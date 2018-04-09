//
// Created by levin on 18-2-15.
//

#ifndef UNREALGO_ARRAYUTIL_H
#define UNREALGO_ARRAYUTIL_H

namespace UnrealGo {
  namespace ArrayUtil {
    int GetOffset(const std::vector<int> &dims, int ndims, const std::vector<int> &indexes);
  }
}

inline int UnrealGo::ArrayUtil::GetOffset(const std::vector<int> &dims, int ndims, const std::vector<int> &indexes) {
  int index = 0;
  int subDim = 1;
  for (int i=0; i<ndims; ++i) {
    subDim *= dims[i];
  }
  for (int i=0; i<ndims; ++i) {
    subDim /= dims[i];
    index += indexes[i] * subDim;
  }
  return index;
}

#endif //UNREALGO_ARRAYUTIL_H
