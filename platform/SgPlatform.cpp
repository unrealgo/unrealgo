
#include "SgPlatform.h"

#ifdef WIN32
// MinGW already defines NOMINMAX
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <lib/FileUtil.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#include "lib/StringUtil.h"

boost::filesystem::path s_programDir;
boost::filesystem::path s_topSourceDir;

const boost::filesystem::path &SgPlatform::GetProgramDir() {
  return s_programDir;
}

void SgPlatform::SetProgramDir(const boost::filesystem::path &dir) {
  s_programDir = dir;
}

const boost::filesystem::path &SgPlatform::GetTopSourceDir() {
  return s_topSourceDir;
}

void SgPlatform::SetTopSourceDir(const boost::filesystem::path &dir) {
  s_topSourceDir = dir;
}

std::string SgPlatform::GetDataFileNativePath(const std::string &filename) {
  const boost::filesystem::path &filePath = SgPlatform::GetTopSourceDir()
      / "data" / filename;
  return UnrealGo::GetNativeFileName(filePath);
}


std::size_t SgPlatform::TotalMemory() {
#if defined WIN32
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  if (! GlobalMemoryStatusEx(&status))
      return 0;
  size_t totalVirtual = static_cast<size_t>(status.ullTotalVirtual);
  size_t totalPhys = static_cast<size_t>(status.ullTotalPhys);
  return std::min(totalVirtual, totalPhys);
#elif defined _SC_PHYS_PAGES
  long pages = sysconf(_SC_PHYS_PAGES);
  if (pages <= 0)
    return 0;

  long pageSize = sysconf(_SC_PAGE_SIZE);
  if (pageSize <= 0)
    return 0;

  return static_cast<size_t>(pages) * static_cast<size_t>(pageSize);
#elif defined HW_PHYSMEM
  // Mac OSX, BSD
  unsigned int mem;
  size_t len = sizeof mem;
  int mib[2] = { CTL_HW, HW_PHYSMEM };
  if (sysctl(mib, 2, &mem, &len, 0, 0) != 0 || len != sizeof mem)
      return 0;
  else
      return mem;
#else
  return 0;
#endif
}


/** Get a default value for lock-free mode.
    Lock-free mode works only on IA-32/Intel-64 architectures or if the macro
    ENABLE_CACHE_SYNC from configure script is defined. The
    architecture is determined by using the macro HOST_CPU from
    configure script. On Windows, an Intel architecture is always assumed. */
bool SgPlatform::GetLockFreeDefault() {
#if defined(WIN32) || defined(ENABLE_CACHE_SYNC)
  return true;
#elif defined(HOST_CPU)
  std::string hostCpu(HOST_CPU);
  return hostCpu == "i386" || hostCpu == "i486" || hostCpu == "i586"
      || hostCpu == "i686" || hostCpu == "x86_64";
#else
  return false;
#endif
}
