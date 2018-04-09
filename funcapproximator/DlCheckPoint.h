
#ifndef UNREALGO_DLCHECKPOINT_H
#define UNREALGO_DLCHECKPOINT_H

#include "DlConfig.h"

namespace DlCheckPoint {

struct CheckPointInfo {
  std::string sha1;
  std::string name;
  std::string url;
  CheckPointInfo();
  explicit CheckPointInfo(const std::string& splitLine, const std::string& delimiter);
  void Parse(const std::string& splitLine, const std::string& delimiter);
};

std::string getLatestCheckPointPrefix();
void InitMetagraph();
bool DownloadMetagraph();
bool CheckDefaultCheckPoint(CheckPointInfo& out_);
bool DownloadCheckPoint(const std::string& infoUrl, CheckPointInfo& out_);
void GetCheckPointInfo(const std::string& infoPath, CheckPointInfo& out_);
void GetBestCheckPointInfo(CheckPointInfo& out_);
void GetLatestCheckPointInfo(CheckPointInfo& out_);

void WriteCheckpointInfo(DlConfig& config,
                         const CheckPointInfo& bestInfo,
                         const std::string& outSubPath,
                         bool overrideIfExists = true);
void WriteCheckpointInfo(DlConfig& config,
                         const std::string& checkpointName,
                         const std::string& outSubPath,
                         bool overrideIfExists = true);
void WriteBestCheckpointInfo(const std::string& checkpointPrefix, bool overrideIfExists = true);
void WriteBestCheckpointInfo(const CheckPointInfo& bestInfo, bool overrideIfExists = true);
void WriteLatestCheckpointInfo(const std::string& checkpointPrefix, bool overrideIfExists = true);
void WriteLatestCheckpointInfo(const CheckPointInfo& info, bool overrideIfExists = true);
void WriteLatestCheckPointInfo();
void UpdateBestCheckPointList(const std::string& checkpoint);
void UpdateBestCheckPointList(const CheckPointInfo& checkpoint);

}

#endif //UNREALGO_DLCHECKPOINT_H
