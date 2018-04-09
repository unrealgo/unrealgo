/*
This file is part of UnrealGo.
Copyright (C) 2017 Kevin
UnrealGo is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
UnrealGo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with UnrealGo.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ATOM_FILEUTIL_H
#define ATOM_FILEUTIL_H

#include <fstream>
#include <boost/range.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>
#include <sys/stat.h>

#define GetCurrentDir getcwd
#endif

namespace fs = boost::filesystem;

namespace UnrealGo {
  fs::path GetFullPath(const std::string &directory, const std::string &name);

  std::string GetSubPath(const std::string &parent, const std::string &fullpath);

  std::string GetFullPathStr(const std::string &directory, const std::string &name);

  std::string ExtractDirectory(const std::string &fullPath);

  std::string ExtractFileName(const std::string &fullPath);

// get current working directory
  std::string GetCWD(void);

  void removeFilesStartWith(const std::string &directory, const std::string &prefix);

  void ListFilesStartWith(const std::string &pathPrefix, std::vector<std::string> &list);

  void ListFilesStartWith(const std::string &dir, const std::string &prefix, std::vector<std::string> &list,
                                 const std::vector<std::string> &excludeList);

  void Remove(std::vector<std::string> &nameList, const std::string &filter);

  int CreatePath(mode_t mode, const std::string &rootPath, std::string path);

  void SgWriteBinaryData(std::string &fileName, char *data, std::size_t bytes);

  void InsertLineAtBegin(const std::string &filename, const std::string &toInsert);

  void ReadLines(const std::string& filename, std::vector<std::string>& lines);

  void WriteLines(const std::string& filename, std::vector<std::string>& lines);

  bool FileExists(const std::string& fileName);

  std::string GetNativeFileName(const boost::filesystem::path &file);
}
#endif //ATOM_FILEUTIL_H
