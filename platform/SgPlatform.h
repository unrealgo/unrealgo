
#ifndef SG_PLATFORM_H
#define SG_PLATFORM_H

#include <boost/filesystem/path.hpp>
#include <cstddef>

namespace SgPlatform {
  std::string GetDataFileNativePath(const std::string &filename);
  const boost::filesystem::path &GetProgramDir();
  const boost::filesystem::path &GetTopSourceDir();
  void SetProgramDir(const boost::filesystem::path &dir);
  void SetTopSourceDir(const boost::filesystem::path &dir);
  std::size_t TotalMemory();

  bool GetLockFreeDefault();
}

#endif // SG_PLATFORM_H
