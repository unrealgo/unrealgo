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

#ifndef CONCURRENT_ATOMHASH_H
#define CONCURRENT_ATOMHASH_H

#include <string>
#include "openssl/sha.h"

static const int SHA256HEXLEN = (SHA256_DIGEST_LENGTH * 2 + 1);
static const int SHA1HEXLEN = (SHA_DIGEST_LENGTH * 2 + 1);

namespace UnrealGo {
  namespace Hash {
    int sha256(const char *path, char output[SHA256HEXLEN]);

    int sha1(const char *path, char output[SHA256HEXLEN]);

    int sha1(const std::string &path, char output[SHA256HEXLEN]);
  }
}
#endif //CONCURRENT_ATOMHASH_H