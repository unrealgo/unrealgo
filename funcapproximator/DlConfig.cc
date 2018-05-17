
#include <bits/ios_base.h>
#include <ios>
#include <fstream>
#include <map>
#include "DlConfig.h"
#include "../config/BoardStaticConfig.h"
#include "../lib/StringUtil.h"
#include "../lib/FileUtil.h"

DlConfig* DlConfig::instance = nullptr;

DlConfig& DlConfig::GetInstance() {
  if (instance == nullptr) {
    instance = new DlConfig("dlcfg-" + UnrealGo::StringUtil::Int2Str(GO_MAX_SIZE));
  }
  return *instance;
}

DlConfig::DlConfig(const std::string& filename) : m_config_file(filename), valueTrans(TR_UNKNOWN) {
  if (UnrealGo::FileExists(filename))
    parse();
  // setMetagraph("bootstrap-ckpt.meta");
}

void DlConfig::parse() {
  std::string line;
  std::ifstream file(m_config_file, std::ios::in);
  while (getline(file, line)) {
    UnrealGo::StringUtil::trim(line);
    if (line[0] != '#') {
      std::string::size_type pos = line.find('=');
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      UnrealGo::StringUtil::trim(key);
      UnrealGo::StringUtil::trim(value);
      config_map.insert(std::make_pair(key, value));
    }
  }
  file.close();
}

std::string DlConfig::get_bestcheckpointlist_subpath() {
  return get("bestcheckpointlistsubpath", "checkpoint/bestcheckpoints.txt");
}

std::string DlConfig::get_bestcheckpointhashlist_subpath() {
  return get("bestcheckpointhashlistsubpath", "train-data/bestckpthashlist.txt");
}

std::string DlConfig::get_bestcheckpoint_fullpath() {
  std::string checkpoint;
  std::string minio_path = get_minio_path();
  if (!minio_path.empty()) {
    std::string bestcheckpointPath = UnrealGo::StringUtil::JoinPath(minio_path, get_bestcheckpointlist_subpath());
    std::ifstream file(bestcheckpointPath, std::ios::in);
    getline(file, checkpoint);
    file.close();
  }
  if (checkpoint.empty())
    checkpoint = get("checkpointprefix");
  return checkpoint;
}

void DlConfig::setMetagraph(const std::string& metagraph) {
  this->current_metagraph = metagraph;
}

std::string DlConfig::get_metagraph() {
  if (current_metagraph.empty()) {
    current_metagraph = get_metagraph_path();
  }
  return current_metagraph;
}

// for both local and server
std::string DlConfig::get_metagraph_path() {
  return get("meta_graph", "bootstrap-ckpt.meta");
}

std::string DlConfig::get_traindata_dir() {
  return get("train_data_dir", UnrealGo::StringUtil::JoinPath(get_minio_path(), "train-data"));
}

std::string DlConfig::get_minio_path() {
  const char* minio = getenv("MINIO_PATH");
  if (!minio) {
    return get("minio_path");
  } else {
    return std::string(minio);
  }
}

std::string DlConfig::get_default_graph() {
  std::string defaultMetaGraph = default_meta_graph();
  if (UnrealGo::FileExists(defaultMetaGraph))
    return defaultMetaGraph;
  if (UnrealGo::FileExists("default_graph.pb.zip"))
    return "default_graph.pb.zip";
  return "default_graph.pb";
}

std::string DlConfig::default_meta_graph() {
  return get("meta_graph", "bootstrap-ckpt.meta");
}

std::string DlConfig::default_checkpoint() {
  return get("checkpoint", "bootstrap-ckpt");
}

std::string DlConfig::default_ckpt_prefix() {
  return "bootstrap-ckpt";
}

std::string DlConfig::get_meta_subpath() {
  std::string defaultPath = "gonet-model/meta/";
  defaultPath.append(default_meta_graph());
  return defaultPath;
}

std::string DlConfig::get_checkeval_subpath() {
  return get("checkevalsubpath", "checkpoint/checkpointevalresult.txt");
}

std::string DlConfig::get_checkdata_bucket() {
  return get("checkdatabucket", "gonet-model");
}

std::string DlConfig::get_checkdata_subpath() {
  return get("checkdatasubpath", "gonet-model/checkpoint");
}

std::string DlConfig::get_bestckinfo_subpath() {
  return get("bestckinfosubpath", "checkpoint/bestcheckpoint.info");
}

std::string DlConfig::get_latestckfile_subpath() {
  return get("latestckfilesubpath", "gonet-model/checkpoint/checkpoint");
}

std::string DlConfig::get_latestckinfo_subpath() {
  return get("latestckinfosubpath", "checkpoint/latestcheckpoint.info");
}

std::string DlConfig::get_base_url() {
  return "http://" + get_host() + ":" + get_minioserver_port() + "/";
}

std::string DlConfig::get_bestcheckpointinfo_url() {
  // return get("bestcheckpointinfoturl");
  return UnrealGo::StringUtil::JoinPath(get_base_url(), get_bestckinfo_subpath());
}

std::string DlConfig::get_latestcheckpointinfo_url() {
  // return get("latestcheckpointinfourl");
  return UnrealGo::StringUtil::JoinPath(get_base_url(), get_latestckinfo_subpath());
}

std::string DlConfig::get_metagraph_url() {
  return UnrealGo::StringUtil::JoinPath(get_base_url(), get_meta_subpath());
}

std::string DlConfig::get(const std::string& key) {
  // std::map<std::string, std::string>::iterator it = config_map.find(key);
  auto it = config_map.find(key);
  if (it != config_map.end())
    return it->second;
  return "";
}

std::string DlConfig::get(const std::string& key, const std::string& defaultValue) {
  std::string ret = get(key);
  if (ret.empty())
    ret = defaultValue;
  return ret;
}

std::string DlConfig::get_evalstatserver_port() {
  return get("evalstatserverport", "6000");
}

std::string DlConfig::get_minioserver_port() {
  return get("svr_port", "8888");
}

std::string DlConfig::get_minioproxyserver_port() {
  return get("minioproxyserverport", "5560");
}

std::string DlConfig::get_minioproxyserver_socket() {
  return "tcp://localhost:" + get_minioproxyserver_port();
}

std::string DlConfig::get_host() {
  return get("host", "unrealgo.com");
}

std::string DlConfig::get_evalstatslisten_socket() {
  return "tcp://*:" + get_evalstatserver_port();
}

std::string DlConfig::get_evalstatconnect_socket() {
  return "tcp://" + get_host() + ":" + get_evalstatserver_port();
}

std::string DlConfig::get_deeptrainerlisten_socket() {
  return get("deeptrainernetworklistensocket", "tcp://*:5556");
}

std::string DlConfig::get_network_input() {
  return get("nn_input", "input");
}

void DlConfig::get_network_outputs(std::vector<std::string>& outputs) {
  std::string values = get("nn_outputs", "");
  if (!values.empty()) {
    UnrealGo::StringUtil::Split(values, ":", outputs);
  }
}

bool DlConfig::reuse_search_tree() {
  std::string value = get("reuse_searchtree", "false");
  if (value == "false")
    return false;
  return true;
}

ValueTransformType DlConfig::get_value_transform() {
  if (valueTrans == TR_UNKNOWN) {
    std::string value = get("value_transform", "0");
    int type = std::stoi(value);
    switch (type) {
      case 0:
        valueTrans = TR_IDENTITY;
        break;
      case 1:
        valueTrans = TR_FLIP_SIGN;
        break;
      case 2:
        valueTrans = TR_FLIP_PROB;
        break;
      default:
        valueTrans = TR_IDENTITY;
    }
  }
  return valueTrans;
}