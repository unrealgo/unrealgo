
#ifndef ATOM_STRINGUTIL_H
#define ATOM_STRINGUTIL_H

#include <vector>

namespace UnrealGo {
  namespace StringUtil {
    void Split(const std::string &line, const std::string &delimiter, std::vector <std::string> &splits);
    std::string Int2Str(int value, int digits = 0);
    std::string JoinPath(const std::string& dir, const std::string& name);
    std::string &trim(std::string &s);
    std::string &trim(std::string &s, char ch);
    std::string& removeSuffix(std::string& url, const std::string& suffix);
    bool EndsWith(const std::string& str, const std::string& suffix);
    int EndsWith(const char *str, const char *suffix);

    std::vector<std::string> SplitArguments(std::string s);
  }
}
#endif //ATOM_STRINGUTIL_H
