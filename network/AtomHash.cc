
#include <cstdio>
#include <cstdlib>
#include "AtomHash.h"

void byte2Hex(unsigned char _hash[], int digestLen, char hexBuf_[]) {
  int i = 0;

  for (i = 0; i < digestLen; i++) {
    sprintf(hexBuf_ + (i * 2), "%02x", _hash[i]);
  }

  hexBuf_[digestLen * 2] = 0;
}

int UnrealGo::Hash::sha256(const char *path, char output[SHA256HEXLEN]) {
  FILE *file = fopen(path, "rb");
  if (!file) return -1;

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  const int bufSize = 32768;
  auto *buffer = (char *) malloc(bufSize);
  size_t bytesRead = 0;
  if (!buffer) return -1;
  while ((bytesRead = fread(buffer, 1, bufSize, file))) {
    SHA256_Update(&ctx, buffer, bytesRead);
  }
  SHA256_Final(hash, &ctx);

  byte2Hex(hash, SHA256_DIGEST_LENGTH, output);
  fclose(file);
  free(buffer);
  return 0;
}

int UnrealGo::Hash::sha1(const std::string &path, char output[SHA256HEXLEN]) {
  return sha1(path.c_str(), output);
}

int UnrealGo::Hash::sha1(const char *path, char output[SHA1HEXLEN]) {
  FILE *file = fopen(path, "rb");
  if (!file) return -1;

  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA_CTX ctx;
  SHA1_Init(&ctx);
  const int bufSize = 32768;
  auto *buffer = (char *) malloc(bufSize);
  size_t bytesRead = 0;
  if (!buffer) return -1;
  while ((bytesRead = fread(buffer, 1, bufSize, file))) {
    SHA1_Update(&ctx, buffer, bytesRead);
  }
  SHA1_Final(hash, &ctx);

  byte2Hex(hash, SHA_DIGEST_LENGTH, output);
  fclose(file);
  free(buffer);
  return 0;
}
