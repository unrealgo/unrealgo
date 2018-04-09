
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include "StringUtil.h"

using namespace std;

namespace UnrealGo {
namespace StringUtil {

void Split(const std::string &line, const std::string &delimiter,
                                std::vector <std::string> &splits) {
  size_t pos = 0;
  std::string token;
  std::string inLine = line;
  while ((pos = inLine.find(delimiter)) != std::string::npos) {
    token = inLine.substr(0, pos);
    splits.emplace_back(token);
    inLine.erase(0, pos + delimiter.length());
  }
  splits.emplace_back(inLine);
}

std::string Int2Str(int value, int digits) {
  std::string result;
  while (digits-- > 0 || value) {
    result += char('0' + value % 10);
    value /= 10;
  }
  if (value < 0) {
    result += '-';
  }
  std::reverse(result.begin(), result.end());
  return result;
}

std::string JoinPath(const std::string& dir, const std::string& name) {
  if (name[0] == '/')
    return name;
  std::string sep;
  if (dir.back() != '/')
    sep = "/";
  return dir + sep + name;
}

std::string& trim(std::string &s) {
  if (s.empty()) {
    return s;
  }
  s.erase(0, s.find_first_not_of(' '));
  s.erase(s.find_last_not_of(' ') + 1);
  return s;
}

std::string& trim(std::string &s, char ch) {
  if (s.empty()) {
    return s;
  }
  s.erase(0, s.find_first_not_of(ch));
  s.erase(s.find_last_not_of(ch) + 1);
  return s;
}

std::string& removeSuffix(std::string& url, const std::string& suffix) {
  if (url.rfind(".zip") != std::string::npos)
    url = url.substr(0, url.rfind(suffix));
  return url;
}

bool EndsWith(const std::string& str, const std::string& suffix) {
  size_t pos = str.rfind(suffix);
  return (pos == str.size() - suffix.size());
}

int EndsWith(const char *str, const char *suffix)
{
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix >  lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

vector<string> SplitArguments(string s) {
  vector<string> result;
  bool escape = false;
  bool inString = false;
  std::ostringstream token;
  for (size_t i = 0; i < s.size(); ++i) {
    char c = s[i];
    if (c == '"' && !escape) {
      if (inString) {
        result.push_back(token.str());
        token.str("");
      }
      inString = !inString;
    } else if (isspace(c) && !inString) {
      if (!token.str().empty()) {
        result.push_back(token.str());
        token.str("");
      }
    } else
      token << c;
    escape = (c == '\\' && !escape);
  }
  if (!token.str().empty())
    result.push_back(token.str());
  return result;
}

}
}
