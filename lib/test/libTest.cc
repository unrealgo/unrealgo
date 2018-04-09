
#include <iostream>
#include "../StringUtil.h"
#include "../FileUtil.h"

void testFile() {
  std::vector<std::string> lines;
  UnrealGo::ReadLines("0a233435araereotei5", lines);
  std::cout << lines.size() << std::endl;
}


int main(int argc, char** argv) {
  const char* str = "eaorgr.zip";
  std::cout << UnrealGo::StringUtil::EndsWith(str, ".zip") << std::endl;
  std::cout << UnrealGo::StringUtil::EndsWith("", ".zip") << std::endl;

  testFile();
  
  return 0;
}