
#include "FileUtil.h"

fs::path UnrealGo::GetFullPath(const std::string &directory, const std::string &name) {
  fs::path dir(directory);
  fs::path file(name);
  return dir / file;
}

std::string UnrealGo::GetSubPath(const std::string &parent, const std::string &fullpath) {
  size_t pos = fullpath.find(parent);
  if (pos == 0) {
    std::string subPath = fullpath.substr(parent.length());
    if (subPath.find('/') == 0)
      return subPath.substr(1);
    return subPath;
  }
  return fullpath;
}

std::string UnrealGo::GetFullPathStr(const std::string &directory, const std::string &name) {
  std::string path = GetFullPath(directory, name).string();
  return path;
}

std::string UnrealGo::ExtractDirectory(const std::string &fullPath) {
  boost::filesystem::path path(fullPath);
  return path.parent_path().string();
}

std::string UnrealGo::ExtractFileName(const std::string &fullPath) {
  const size_t lastSlashIndex = fullPath.find_last_of("/\\");
  return fullPath.substr(lastSlashIndex + 1);
}

// get current working directory
std::string UnrealGo::GetCWD(void) {
  char buff[FILENAME_MAX];
  GetCurrentDir(buff, FILENAME_MAX);
  std::string current_working_dir(buff);
  return current_working_dir;
}

void UnrealGo::removeFilesStartWith(const std::string &directory, const std::string &prefix) {
  std::vector<boost::filesystem::directory_entry> removeList;
  for (auto &entry : boost::make_iterator_range(boost::filesystem::directory_iterator(directory), {})) {
    if (entry.path().filename().string().find(prefix) == 0)
      removeList.push_back(entry);
    // std::cout << entry << "\n";
  }

  for (auto &entry : removeList)
    boost::filesystem::remove(entry);
}

void UnrealGo::ListFilesStartWith(const std::string &pathPrefix, std::vector<std::string> &list) {
  boost::filesystem::path path(pathPrefix);
  boost::filesystem::path directory = path.parent_path();
  if (directory.empty())
    directory = ".";
  std::string prefix = path.filename().string();
  // std::cout << prefix << std::endl;

  for (boost::filesystem::directory_entry &entry : boost::filesystem::directory_iterator(directory)) {
    std::string filename = entry.path().filename().c_str();
    if (filename.find(prefix) == 0)
      list.push_back(filename);
  }
}

static bool ListContains(const std::vector<std::string>& containList, const std::string& str) {
  for (const std::string& ele : containList) {
    if (ele == str)
      return true;
  }
  return false;
}

void UnrealGo::ListFilesStartWith(const std::string &dir, const std::string &prefix, std::vector<std::string> &list,
                                 const std::vector<std::string> &excludeList) {
  boost::filesystem::path directory(dir);
  if (directory.empty())
    directory = ".";

  for (boost::filesystem::directory_entry &entry : boost::filesystem::directory_iterator(directory)) {
    std::string filename = entry.path().filename().c_str();
    if (filename.find(prefix) == 0 && !ListContains(excludeList, filename))
      list.push_back(GetFullPathStr(directory.string(), filename));
  }
}

void UnrealGo::Remove(std::vector<std::string> &nameList, const std::string &filter) {
  std::vector<std::string> tmpList;
  for (std::string &name : nameList) {
    if (name != filter)
      tmpList.emplace_back(name);
  }

  std::swap(tmpList, nameList);
}

int UnrealGo::CreatePath(mode_t mode, const std::string &rootPath, std::string path) {
  struct stat st;
  for (std::string::iterator iter = path.begin(); iter != path.end();) {
    std::string::iterator newIter = std::find(iter, path.end(), '/');
    std::string newPath = rootPath + "/" + std::string(path.begin(), newIter);

    if (stat(newPath.c_str(), &st) != 0) {
      if (mkdir(newPath.c_str(), mode) != 0 && errno != EEXIST) {
        //std::cout << "cannot create folder [" << newPath << "] : " << strerror(errno) << std::endl;
        return -1;
      }
    } else if (!S_ISDIR(st.st_mode)) {
      errno = ENOTDIR;
      //std:: cout << "path [" << newPath << "] not a dir " << std::endl;
      return -1;
    }
    //else
    //    std::cout << "path [" << newPath << "] already exists " << std::endl;

    iter = newIter;
    if (newIter != path.end())
      ++iter;
  }
  return 0;
}

void UnrealGo::SgWriteBinaryData(std::string &fileName, char *data, std::size_t bytes) {
  std::fstream file(fileName, std::ios::out | std::ios::binary | std::ios::app);
  file.write(data, bytes);
  file.flush();
  file.close();
}

void UnrealGo::InsertLineAtBegin(const std::string &filename, const std::string &toInsert) {
  std::fstream inFile(filename, std::ios::in);
  std::string line;
  std::vector<std::string> textLines;

  while (getline(inFile, line)) {
    textLines.push_back(line);
  }
  inFile.close();

  if (textLines.empty() || toInsert != textLines[0]) {
    std::ofstream outFile(filename, std::ios::out);
    outFile << toInsert << std::endl;
    for (auto &l : textLines) {
      outFile << l << std::endl;
    }
    outFile.close();
  }
}

void UnrealGo::ReadLines(const std::string& filename, std::vector<std::string>& lines) {
  std::fstream inFile(filename, std::ios::in);
  std::string line;
  while (getline(inFile, line)) {
    lines.push_back(line);
  }
  inFile.close();
}

void UnrealGo::WriteLines(const std::string& filename, std::vector<std::string>& lines) {
  std::ofstream outFile(filename, std::ios::out);
  for (auto &l : lines) {
    outFile << l << std::endl;
  }
  outFile.close();
}


bool UnrealGo::FileExists(const std::string& fileName) {
  struct stat buffer;
  return (stat(fileName.c_str(), &buffer) == 0);
}

std::string UnrealGo::GetNativeFileName(const boost::filesystem::path &file) {
  boost::filesystem::path normalizedFile = file;
  normalizedFile.normalize();
  return normalizedFile.string();
}