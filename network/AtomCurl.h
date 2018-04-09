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

#ifndef UNREALGO_ATOMCURL_H
#define UNREALGO_ATOMCURL_H

#include <curl/curl.h>

typedef size_t (*CurlCallback)(char *data, size_t size, size_t nmemb, void *out_);

struct AtomCurl {
public:
  AtomCurl();

  bool Get(const char *url, void *writeData, CurlCallback callback);

  int Download(const char *url, char *outFileName, bool forceReDownload=true);

  int DownloadExtractZip(const char *url, bool forceReDownload=true);

protected:
  char errorBuffer[CURL_ERROR_SIZE];
};

#endif //UNREALGO_ATOMCURL_H
