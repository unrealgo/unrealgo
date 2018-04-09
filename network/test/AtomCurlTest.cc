#include <iostream>
#include "../AtomCurl.h"

int main(int argc, char **argv) {
  AtomCurl curl;
  char srcName[1024];
  const char* url = "http://unrealgo.com:8888/gonet-model/meta/4-model.ckpt.meta";
  int ret = curl.Download(url, srcName, true);
  std::cout << ret << std::endl;
  return 0;
}