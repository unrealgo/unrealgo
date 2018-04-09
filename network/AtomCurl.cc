
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "AtomCurl.h"
#include "CCZLib.h"
#include "AtomUNZIP.h"
#include "platform/SgDebug.h"
#include "../lib/StringUtil.h"

AtomCurl::AtomCurl() {
  errorBuffer[0] = 0;
  curl_global_init(CURL_GLOBAL_DEFAULT);
}


typedef unsigned long uint64_t;
typedef struct DownloadParamsType {
  char dnld_remote_fname[4096];
  char dnld_url[4096];
  FILE *dnld_stream;
  FILE *dbg_stream;
  uint64_t dnld_file_sz;
  int statusCode;

  DownloadParamsType();
} DownloadParamsType;

DownloadParamsType::DownloadParamsType() : dnld_stream(nullptr), dbg_stream(nullptr), dnld_file_sz(0), statusCode(-1) {
  dnld_remote_fname[0] = 0;
  dnld_url[0] = 0;
}

static int get_oname_from_cd(char const *const cd, char *oname) {
  char const *const cdtag = "Content-disposition:";
  char const *const key = "filename=";
  int ret = 0;
  const char *val = nullptr;

  /* Example Content-Disposition: filename=name1367; charset=funny; option=strange */
  /* If filename is present */
  val = strcasestr(cd, key);
  if (!val) {
    printf("No key-value for \"%s\" in \"%s\"", key, cdtag);
    goto bail;
  }

  /* Move to value */
  val += strlen(key);

  /* Copy value as oname */
  while (*val != '\0' && *val != ';' && *val != '\r' && *val != '\n') {
    //printf (".... %c\n", *val);
    *oname++ = *val++;
  }
  *oname = '\0';

  bail:
  return ret;
}

static int get_oname_from_url(char const *url, char *oname) {
  int ret = CURLE_OK;
  char const *u = url;

  /* Remove "http(s)://" */
  u = strstr(u, "://");
  if (u) {
    u += strlen("://");
  }

  u = strrchr(u, '/');

  /* Remove last '/' */
  u++;

  /* Copy value as oname */
  while (*u != '\0') {
    //printf (".... %c\n", *u);
    *oname++ = *u++;
  }
  *oname = '\0';

  return ret;
}

static int parseStatusCode(const char* header) {
  const char* pc = header;
  while((*pc) == ' ')
    ++pc;
  if (strncasecmp(pc, "http/", 5) == 0) {
    pc += 5;
    while (*pc && *pc != ' ')
      ++ pc;
    while((*pc) == ' ')
      ++pc;
    int code = 0;
    while (*pc && isdigit(*pc)) {
      code = code*10 + (*pc - '0');
      ++ pc;
    }
    return code;
  }
  return -1;
}

static size_t dnld_header_parse(void *hdr, size_t size, size_t nmemb, void *userdata) {
  const size_t cb = size * nmemb;
  auto *hdr_str = (const char *) hdr;
  auto *dlParams = (DownloadParamsType *) userdata;
  char const *const cdtag = "Content-disposition:";

  if (strstr(hdr_str, "HTTP") != nullptr) {
    dlParams->statusCode = parseStatusCode(hdr_str);
    if (dlParams->statusCode != 200)
      return 0;
  }

  /* Example:
   * ...
   * Content-Type: text/html
   * Content-Disposition: filename=name1367; charset=funny; option=strange
   */
  if (strstr(hdr_str, "Content-disposition:")) {
    printf("has c-d: %s\n", hdr_str);
  }

  if (!strncasecmp(hdr_str, cdtag, strlen(cdtag))) {
    printf("Found c-d: %s\n", hdr_str);
    int ret = get_oname_from_cd(hdr_str + strlen(cdtag), dlParams->dnld_remote_fname);
    if (ret) {
      printf("ERR: bad remote name");
    }
  }

  return cb;
}

static FILE *get_dnld_stream(char const *const fname) {
//  char const *const pre = "/tmp/";
//  char out[4096];
//  snprintf(out, sizeof(out), "%s/%s", pre, fname);

  FILE *fp = fopen(fname, "wb");
  if (!fp) {
    printf("Could not create file %s\n", fname);
  }

  return fp;
}

static size_t write_cb(void *buffer, size_t sz, size_t nmemb, void *userdata) {
  size_t ret = 0;
  auto *dlParams = (DownloadParamsType *) userdata;
  //char* content = (char*) buffer;

  // bad code: 403 404 ...
  if (dlParams->statusCode != 200)
    return 0;
//  if (dlParams->statusCode == 404)
//    return 0;

  if (!dlParams->dnld_remote_fname[0] &&
          get_oname_from_url(dlParams->dnld_url, dlParams->dnld_remote_fname) != CURLE_OK)
    return 0;

  if (!dlParams->dnld_stream) {
    dlParams->dnld_stream = get_dnld_stream(dlParams->dnld_remote_fname);
  }

  ret = fwrite(buffer, sz, nmemb, dlParams->dnld_stream);
  if (ret == (sz * nmemb)) {
    dlParams->dnld_file_sz += ret;
  }
  return ret;
}


int AtomCurl::Download(char const *const url, char *outFileName, bool forceReDownload) {
  CURL *curl;
  int ret = -1;
  CURLcode code = CURLE_OK;
  DownloadParamsType dlParams;

  memset(&dlParams, 0, sizeof(dlParams));
  strncpy(dlParams.dnld_url, url, strlen(url));
  dlParams.statusCode = 200;

  curl = curl_easy_init();
  if (!curl) {
    goto bail;
  }

  code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set error buffer [%d]\n", code);
    return false;
  }

  code = curl_easy_setopt(curl, CURLOPT_URL, url);
  if (code) {
    printf("%s: failed with err %d\n", "URL", code);
    goto bail;
  }

  code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
    return false;
  }

  code = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, dnld_header_parse);
  if (code) {
    printf("%s: failed with err %d\n", "HEADER", code);
    goto bail;
  }
  code = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dlParams);
  if (code) {
    printf("%s: failed with err %d\n", "HEADER DATA", code);
    goto bail;
  }

  code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  if (code) {
    printf("%s: failed with err %d\n", "WR CB", code);
    goto bail;
  }
  code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dlParams);
  if (code) {
    printf("%s: failed with err %d\n", "WR Data", code);
    goto bail;
  }
//  int response_code_;
//  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code_);

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    //SgDebug() << "Failed to access: " << url << "\n";
    SgDebug() << "Failed to access file : " << url << "\n";
    ret = CURLE_HTTP_NOT_FOUND;
  } else {
    SgDebug() << "Remote name: " << dlParams.dnld_remote_fname << "\n";
    // printf("Remote name: %s\n", dlParams.dnld_remote_fname);
    if (dlParams.dnld_stream)
      fclose(dlParams.dnld_stream);
    strcpy(outFileName, dlParams.dnld_remote_fname);

    /* always cleanup */
    curl_easy_cleanup(curl);
    ret = CURLE_OK;
    printf("file size : %lu\n", dlParams.dnld_file_sz);
  }

  bail:
  return ret;
}


bool AtomCurl::Get(const char *url, void *writeData, CurlCallback callback) {
  CURLcode code;
  CURL *conn = curl_easy_init();
  DownloadParamsType dlParams;

  memset(&dlParams, 0, sizeof(dlParams));
  strncpy(dlParams.dnld_url, url, strlen(url));
  dlParams.statusCode = 200;

  if (conn == NULL) {
    fprintf(stderr, "Failed to create CURL connection\n");
    exit(EXIT_FAILURE);
  }

  code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set error buffer [%d]\n", code);
    return false;
  }

  code = curl_easy_setopt(conn, CURLOPT_URL, url);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
    return false;
  }

  code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
    return false;
  }

  code = curl_easy_setopt(conn, CURLOPT_HEADERFUNCTION, dnld_header_parse);
  if (code) {
    printf("%s: failed with err %d\n", "HEADER", code);
    return false;
  }
  code = curl_easy_setopt(conn, CURLOPT_HEADERDATA, &dlParams);
  if (code) {
    printf("%s: failed with err %d\n", "HEADER DATA", code);
    return false;
  }

  code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, callback);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
    return false;
  }

  code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, writeData);
  if (code != CURLE_OK) {
    fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
    return false;
  }

  code = curl_easy_perform(conn);
  curl_easy_cleanup(conn);
  return code == CURLE_OK;
}

static void removeExt(char *name) {
  size_t idx = strlen(name);
  while (idx > 0 && name[idx] != '.')
    --idx;
  name[idx] = '\0';
}

int AtomCurl::DownloadExtractZip(const char *url, bool forceReDownload) {
  char srcName[1024];
  if (Download(url, srcName, forceReDownload) == CURLE_OK) {
    int ret = CURLE_OK;
    if (UnrealGo::StringUtil::EndsWith(srcName, ".zip"))
      ret = UnrealGo::UNZIP::extract_zip_file(srcName);
    return ret;
  }
  return -1;
}