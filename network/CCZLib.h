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

#ifndef _CCZLIB_H
#define _CCZLIB_H

#include <cstdio>
#include <zlib.h>

namespace CCZLib {

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
  int def(FILE *source, FILE *dest, int level);

  int def(const char *source, const char *dest, int level);

  int def(const char *sourceList[], const char *dest, int level);

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
  int inf(FILE *source, FILE *dest);

  int inf(const char *source, const char *dest);

  void Zerr(int ret);
}
#endif //_CCZLIB_H
