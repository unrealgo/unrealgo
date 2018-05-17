
#include <fstream>
#include <sstream>
#include <StringUtil.h>
#include <iostream>
#include "DlCheckPoint.h"
#include "../network/AtomCurl.h"
#include "../lib/FileUtil.h"
#include "../network/AtomHash.h"
#include "../network/AtomZIP.h"
#include "../msg/MinioStub.h"
#include "../network/AtomUNZIP.h"
#include "platform/SgDebug.h"

DlCheckPoint::CheckPointInfo::CheckPointInfo() : sha1(""), name(""), url("") {
}

DlCheckPoint::CheckPointInfo::CheckPointInfo(const std::string& splitLine, const std::string& delimiter) {
  Parse(splitLine, delimiter);
}

void DlCheckPoint::CheckPointInfo::Parse(const std::string& splitLine, const std::string& delimiter) {
  std::vector<std::string> splits;
  UnrealGo::StringUtil::Split(splitLine, delimiter, splits);

  if (!splits.empty()) {
    sha1 = splits[0];
    if (splits.size() >= 2) {
      name = splits[1];
      if (splits.size() >= 3) {
        url = splits[2];
      }
    }
  }
}

/*checkpoint/checkpoint
  model_checkpoint_path: "model.ckpt-26711"
  all_model_checkpoint_paths: "model.ckpt-26711"
 */
std::string DlCheckPoint::getLatestCheckPointPrefix() {
  std::string line;
  std::string checkpoint;
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string
      checkpointdatafile = UnrealGo::GetFullPathStr(minio_path, DlConfig::GetInstance().get_latestckfile_subpath());
  std::ifstream file(checkpointdatafile, std::ios::in);
  while (getline(file, line)) {
    UnrealGo::StringUtil::trim(line);
    if (line[0] != '#') {
      std::string::size_type pos = line.find(':');
      std::string key = line.substr(0, pos);
      UnrealGo::StringUtil::trim(key);
      if (key == "model_checkpoint_path") {
        checkpoint = line.substr(pos + 1);
        UnrealGo::StringUtil::trim(checkpoint);
        UnrealGo::StringUtil::trim(checkpoint, '"');
        break;
      }
    }
  }
  file.close();
  return checkpoint;
}

static size_t CheckPointCB(char* data, size_t size, size_t nmemb, void* out_) {
  auto* checkPoint = (DlCheckPoint::CheckPointInfo*)out_;
  if (checkPoint == nullptr)
    return 0;

  std::string text = data;
  std::istringstream iss(text);
  std::string line;
  std::vector<std::string> strList;
  while (std::getline(iss, line)) {
    strList.push_back(line);
  }
  checkPoint->name = strList[0];
  checkPoint->sha1 = strList[1];
  checkPoint->url = strList[2];

  return size * nmemb;
}

void DlCheckPoint::InitMetagraph() {
  std::string subpath = DlConfig::GetInstance().get_meta_subpath();
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string absPath = UnrealGo::GetFullPathStr(minio_path, subpath);
  std::vector<std::string> fileList;
  std::string zipFilePath = absPath + ".zip";
  fileList.emplace_back(absPath);
  UnrealGo::ZIP::compress_zip_files(fileList, zipFilePath);
  UnrealGo::MinioStub::SetObjPolicy(subpath, "readonly");
}

bool DlCheckPoint::DownloadMetagraph() {
  std::string url = DlConfig::GetInstance().get_metagraph_url() + ".zip";
  AtomCurl atomCurl;
  int ret = atomCurl.DownloadExtractZip(url.c_str());
  if (ret != CURLE_OK) {
    url = DlConfig::GetInstance().get_metagraph_url();
    if (atomCurl.DownloadExtractZip(url.c_str()) != CURLE_OK) {
      url = DlConfig::GetInstance().get_default_graph();
      SgDebug() << "Using local graph: " << url << "\n";
      if (!UnrealGo::StringUtil::EndsWith(url, ".zip")) {
        if (!UnrealGo::FileExists(url))
          return false;
      } else {
        if (UnrealGo::UNZIP::extract_zip_file(url.c_str()) != 0)
          return false;
      }
    }
  }
  UnrealGo::StringUtil::removeSuffix(url, ".zip");
  DlConfig::GetInstance().setMetagraph(UnrealGo::ExtractFileName(url));
  return true;
}

static bool checkZipFile(DlCheckPoint::CheckPointInfo& info) {
  std::string zipName = info.name + ".zip";
  if (UnrealGo::FileExists(zipName)) {
    char sha1Buf[SHA1HEXLEN];
    UnrealGo::Hash::sha1(zipName, sha1Buf);
    if (info.sha1 == sha1Buf)
      return true;
  }
  return false;
}

bool DlCheckPoint::CheckDefaultCheckPoint(CheckPointInfo& out_) {
  std::string defaultName = DlConfig::GetInstance().default_checkpoint();
  if (UnrealGo::FileExists(defaultName + ".index") && UnrealGo::FileExists(defaultName + ".data-00000-of-00001")) {
    out_.name = defaultName;
    return true;
  }
  return false;
}

bool DlCheckPoint::DownloadCheckPoint(const std::string& infoUrl, CheckPointInfo& out_) {
  CheckPointInfo downloaded;
  AtomCurl atomCurl;
  bool get = atomCurl.Get(infoUrl.c_str(), &downloaded, &CheckPointCB);
  if (get) {
    if (out_.sha1 != downloaded.sha1) {
      out_ = downloaded;
      if (!out_.name.empty()) {
        if (checkZipFile(out_)) {
          std::string zipname = out_.name + ".zip";
          return UnrealGo::UNZIP::extract_zip_file(zipname.c_str()) == 0;
        } else {
          UnrealGo::removeFilesStartWith(".", out_.name);
          return atomCurl.DownloadExtractZip(out_.url.c_str()) == 0;
        }
      }
      return false; // indicate update checkpoint for UctSearch
    } else
      return true;
  }
  return false;
}

void DlCheckPoint::GetCheckPointInfo(const std::string& infoPath, CheckPointInfo& out_) {
  if (UnrealGo::FileExists(infoPath)) {
    std::vector<std::string> lines;
    UnrealGo::ReadLines(infoPath, lines);
    if (lines.size() >= 3) {
      out_.name = lines[0];
      out_.sha1 = lines[1];
      out_.url = lines[2];
    }
  }
}

void DlCheckPoint::GetBestCheckPointInfo(CheckPointInfo& out_) {
  std::string subpath = DlConfig::GetInstance().get_bestckinfo_subpath(); //"checkpoint/bestcheckpoint.info";
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string absPath = UnrealGo::GetFullPathStr(minio_path, subpath);
  GetCheckPointInfo(absPath, out_);
}

void DlCheckPoint::GetLatestCheckPointInfo(CheckPointInfo& out_) {
  std::string subpath = DlConfig::GetInstance().get_latestckinfo_subpath(); //"checkpoint/bestcheckpoint.info";
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string absPath = UnrealGo::GetFullPathStr(minio_path, subpath);
  GetCheckPointInfo(absPath, out_);
}

void DlCheckPoint::WriteCheckpointInfo(DlConfig& config,
                                       const CheckPointInfo& bestInfo,
                                       const std::string& outSubPath,
                                       bool overrideIfExists) {
  std::string minio_path = config.get_minio_path();
  std::string outPath = UnrealGo::GetFullPathStr(minio_path, outSubPath);
  if (UnrealGo::FileExists(outPath) && !overrideIfExists)
    return;
  std::ofstream outFile(outPath, std::ios::out);

  outFile << bestInfo.name << std::endl;
  outFile << bestInfo.sha1 << std::endl;
  outFile << bestInfo.url;

  outFile.close();
}

void DlCheckPoint::WriteCheckpointInfo(DlConfig& config,
                                       const std::string& checkpointName,
                                       const std::string& outSubPath,
                                       bool overrideIfExists) {
  std::string minio_path = config.get_minio_path();
  std::string outPath = UnrealGo::GetFullPathStr(minio_path, outSubPath);
  if (UnrealGo::FileExists(outPath) && !overrideIfExists)
    return;

  std::ofstream outFile(outPath, std::ios::out);
  std::string checkParentDir = UnrealGo::GetFullPathStr(minio_path, config.get_checkdata_subpath());

  outFile << checkpointName << std::endl;
  std::vector<std::string> ckList;
  std::string zipFileName = checkpointName + ".zip";
  std::vector<std::string> exList;
  exList.emplace_back(zipFileName);
  exList.emplace_back(checkpointName + ".meta");

  UnrealGo::ListFilesStartWith(checkParentDir, checkpointName, ckList, exList);
  std::string zipFilePath = UnrealGo::GetFullPathStr(checkParentDir, zipFileName);
  UnrealGo::ZIP::compress_zip_files(ckList, zipFilePath);
  UnrealGo::MinioStub::SetObjPolicy(UnrealGo::StringUtil::JoinPath(config.get_checkdata_subpath(), zipFileName),
                                    "readonly");

  char sha1Buf[SHA1HEXLEN];
  UnrealGo::Hash::sha1(zipFilePath, sha1Buf);
  outFile << sha1Buf << std::endl;

  std::string checkpointDataUrl = UnrealGo::StringUtil::JoinPath(config.get_base_url(),
                                                                 config.get_checkdata_subpath()); // + checkpointName + ".zip";
  checkpointDataUrl = UnrealGo::StringUtil::JoinPath(checkpointDataUrl, checkpointName + ".zip");
  outFile << checkpointDataUrl;
  outFile.close();

  UnrealGo::MinioStub::SetObjPolicy(outSubPath, "readonly");
}

void DlCheckPoint::WriteBestCheckpointInfo(const std::string& checkpointPrefix, bool overrideIfExists) {
  std::string outSubPath = DlConfig::GetInstance().get_bestckinfo_subpath(); //"checkpoint/bestcheckpoint.info";
  std::string checkpointName = UnrealGo::ExtractFileName(checkpointPrefix);
  WriteCheckpointInfo(DlConfig::GetInstance(), checkpointName, outSubPath, overrideIfExists);
}

void DlCheckPoint::WriteLatestCheckpointInfo(const std::string& checkpointPrefix, bool overrideIfExists) {
  std::string outSubPath = DlConfig::GetInstance().get_latestckinfo_subpath();
  std::string checkpointName = UnrealGo::ExtractFileName(checkpointPrefix);
  WriteCheckpointInfo(DlConfig::GetInstance(), checkpointName, outSubPath, overrideIfExists);
}

void DlCheckPoint::WriteBestCheckpointInfo(const CheckPointInfo& bestInfo, bool overrideIfExists) {
  std::string outSubPath = DlConfig::GetInstance().get_bestckinfo_subpath(); //"checkpoint/bestcheckpoint.info";
  WriteCheckpointInfo(DlConfig::GetInstance(), bestInfo, outSubPath, overrideIfExists);
}

void DlCheckPoint::WriteLatestCheckpointInfo(const CheckPointInfo& info, bool overrideIfExists) {
  std::string outSubPath = DlConfig::GetInstance().get_latestckinfo_subpath(); //"checkpoint/bestcheckpoint.info";
  WriteCheckpointInfo(DlConfig::GetInstance(), info, outSubPath, overrideIfExists);
}

void DlCheckPoint::WriteLatestCheckPointInfo() {
  std::string latestCheckPoint = DlCheckPoint::getLatestCheckPointPrefix();
  WriteLatestCheckpointInfo(latestCheckPoint);
}

void DlCheckPoint::UpdateBestCheckPointList(const std::string& checkpoint) {
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string bestcheckpointListPath =
      UnrealGo::GetFullPathStr(minio_path, DlConfig::GetInstance().get_bestcheckpointlist_subpath());
  std::string fullPath;
  if (checkpoint[0] == '/')
    fullPath = checkpoint;
  else
    fullPath = UnrealGo::StringUtil::JoinPath(UnrealGo::StringUtil::JoinPath(minio_path,
                                                                             DlConfig::GetInstance().get_checkdata_subpath()),
                                              checkpoint);
  UnrealGo::InsertLineAtBegin(bestcheckpointListPath, fullPath);
}

void DlCheckPoint::UpdateBestCheckPointList(const CheckPointInfo& checkpoint) {
  std::string minio_path = DlConfig::GetInstance().get_minio_path();
  std::string bestcheckpointListPath =
      UnrealGo::GetFullPathStr(minio_path, DlConfig::GetInstance().get_bestcheckpointlist_subpath());
  std::string fullPath = UnrealGo::StringUtil::JoinPath(UnrealGo::StringUtil::JoinPath(minio_path,
                                                                                       DlConfig::GetInstance().get_checkdata_subpath()),
                                                        checkpoint.name);
  UnrealGo::InsertLineAtBegin(bestcheckpointListPath, fullPath);

  std::string bestcheckpointhashPath =
      UnrealGo::GetFullPathStr(minio_path, DlConfig::GetInstance().get_bestcheckpointhashlist_subpath());
  UnrealGo::InsertLineAtBegin(bestcheckpointhashPath, checkpoint.sha1);
}