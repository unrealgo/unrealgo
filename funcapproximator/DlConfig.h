
#ifndef TFRECORD_GONET_DLCONFIG_H
#define TFRECORD_GONET_DLCONFIG_H

#include <map>
#include <vector>

enum ValueTransformType {
  TR_IDENTITY,      // v = v
  TR_FLIP_SIGN, // v = -1*v
  TR_FLIP_PROB, // v = 1-v
  TR_UNKNOWN
};

class DlConfig {

 public:
  static DlConfig& GetInstance();

 protected:
  static DlConfig* instance;
  std::string get_metagraph_path();

 public:
  explicit DlConfig(const std::string& filename);

  void setMetagraph(const std::string& metagraph);
  void parse();
  std::string get(const std::string& key, const std::string& defaultValue);
  std::string get(const std::string& key);
  std::string get_bestcheckpoint_fullpath();
  std::string get_bestcheckpointlist_subpath();
  std::string get_bestcheckpointhashlist_subpath();
  std::string get_metagraph();
  std::string get_traindata_dir();
  std::string get_minio_path();
  std::string default_meta_graph();
  std::string default_checkpoint();
  std::string default_ckpt_prefix();
  std::string get_meta_subpath();
  std::string get_checkeval_subpath();
  std::string get_checkdata_bucket();
  std::string get_checkdata_subpath();
  std::string get_bestckinfo_subpath();
  std::string get_latestckinfo_subpath();
  std::string get_latestckfile_subpath();
  std::string get_base_url();
  std::string get_bestcheckpointinfo_url();
  std::string get_latestcheckpointinfo_url();
  std::string get_metagraph_url();
  std::string get_default_graph();
  std::string get_minioserver_port();
  std::string get_evalstatserver_port();
  std::string get_minioproxyserver_port();
  std::string get_minioproxyserver_socket();
  std::string get_host();
  std::string get_evalstatslisten_socket();
  std::string get_evalstatconnect_socket();
  std::string get_deeptrainerlisten_socket();
  std::string get_network_input();
  void get_network_outputs(std::vector<std::string>& outputs);
  bool reuse_search_tree();

  ValueTransformType get_value_transform();

 private:
  std::string m_config_file;
  std::map<std::string, std::string> config_map;
  std::string current_metagraph;

  ValueTransformType valueTrans;
};

#endif //TFRECORD_GONET_DLCONFIG_H
