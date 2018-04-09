
#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow/core/example/feature_util.h"
#include "tensorflow/python/lib/io/py_record_writer.h"
#include "tensorflow/c/c_api_internal.h"
#include "tensorflow/core/lib/io/compression.h"

#include "DlTFRecordWriter.h"

using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;
using namespace tensorflow;
using namespace std;
using namespace tensorflow::io;

DlTFRecordWriter::DlTFRecordWriter(const string& filename) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  TF_Status status;
  m_recordWriter = PyRecordWriter::New(filename, compression::kNone, &status);
}

DlTFRecordWriter::~DlTFRecordWriter() {

  TF_Status status;
  m_recordWriter->Flush(&status);
  m_recordWriter->Close(&status);
  delete m_recordWriter;

  google::protobuf::ShutdownProtobufLibrary();
}

void DlTFRecordWriter::Flush() {
  TF_Status status;
  m_recordWriter->Flush(&status);
}

void DlTFRecordWriter::AddByteList(Features& features, Feature& feature, const string& key, char* bytes, size_t len) {
  auto* bytesList = new BytesList;
  bytesList->add_value(bytes, len);
  feature.set_allocated_bytes_list(bytesList);
  features.mutable_feature()->insert(
      google::protobuf::Map<::std::string, ::tensorflow::Feature>::value_type(key, feature));
}

void
DlTFRecordWriter::AddFloatList(Features& features, Feature& feature, const string& key, float* floats, size_t len) {
  auto* floatList = new FloatList;
  google::protobuf::RepeatedField<float>* field = floatList->mutable_value();
  for (size_t i = 0; i < len; i++)
    field->Add(floats[i]);
  feature.set_allocated_float_list(floatList);
  features.mutable_feature()->insert(
      google::protobuf::Map<::std::string, ::tensorflow::Feature>::value_type(key, feature));
}

int DlTFRecordWriter::WriteExample(char* feature17, size_t f_len, float* policy, size_t p_size, float reward) {
  Example example;
  auto* features = new Features;
  Feature feature;
  AddByteList(*features, feature, "feature", feature17, f_len);
  feature.clear_bytes_list();

  AddFloatList(*features, feature, "policy", policy, p_size);
  feature.clear_float_list();

  AddFloatList(*features, feature, "reward", &reward, 1);
  feature.clear_float_list();

  example.set_allocated_features(features);
  m_recordWriter->WriteRecord(example.SerializeAsString());

  return 0;
}

int DlTFRecordWriter::WriteExample(
    const string& feature_name, char* feature17, size_t f_len,
    const string& policy_name, float* policy, size_t p_size,
    const string& reward_name, float reward) {
  Example example;
  auto* features = new Features;
  Feature feature;
  AddByteList(*features, feature, feature_name, feature17, f_len);
  feature.clear_bytes_list();

  AddFloatList(*features, feature, policy_name, policy, p_size);
  feature.clear_float_list();

  AddFloatList(*features, feature, reward_name, &reward, 1);
  feature.clear_float_list();

  example.set_allocated_features(features);
  m_recordWriter->WriteRecord(example.SerializeAsString());

  return 0;
}



//
//void WriteTFRecords(const string &filename) {
//  GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//  TF_Status status;
//  PyRecordWriter *writer = PyRecordWriter::New(filename, compression::kNone, &status);
//  SgRandom sgRandom;
//  char feature[6137];
//  float policy[362];
//  float value;
//  size_t pLen = 362;
//  memset(feature, 1, sizeof(feature));
//
//  for (int i = 0; i < 256; i++) {
//    sgRandom.generateDirichlet(policy, 0.5, pLen);
//    value = sgRandom.Int(2) == 0 ? 1 : -1;
//    WriteExample(writer, feature, sizeof(feature), policy, pLen, value);
//  }
//
//  writer->Flush(&status);
//  writer->Close(&status);
//
//  google::protobuf::ShutdownProtobufLibrary();
//}