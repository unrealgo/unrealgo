
#ifndef TFRECORD_GONET_DLTFRECORDWRITER_H
#define TFRECORD_GONET_DLTFRECORDWRITER_H

namespace tensorflow {
class Features;
class Feature;
namespace io {
using namespace tensorflow;
using namespace std;

class PyRecordWriter;
class DlTFRecordWriter {
 public:
  int WriteExample(char* feature17, size_t f_len, float* policy, size_t p_size, float reward);
  int WriteExample(const string& feature_name, char* feature17, size_t f_len,
                   const string& policy_name, float* policy, size_t p_size,
                   const string& reward_name, float reward);
  explicit DlTFRecordWriter(const string& filename);
  ~DlTFRecordWriter();
  void Flush();

 protected:

  void AddByteList(Features& features, Feature& feature, const string& key, char* bytes, size_t len);
  void AddFloatList(Features& features, Feature& feature, const string& key, float* floats, size_t len);
  tensorflow::io::PyRecordWriter* m_recordWriter;
};
}
}

#endif //TFRECORD_GONET_DLTFRECORDWRITER_H
