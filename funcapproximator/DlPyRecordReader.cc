
#include "tensorflow/python/lib/io/py_record_reader.h"

#include "tensorflow/c/tf_status_helper.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/io/record_reader.h"
#include "tensorflow/core/lib/io/zlib_compression_options.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/types.h"

namespace tensorflow {

class RandomAccessFile;
namespace io {

PyRecordReader::PyRecordReader() {}

PyRecordReader* PyRecordReader::New(const string& filename, uint64 start_offset,
                                    const string& compression_type_string,
                                    TF_Status* out_status) {
  std::unique_ptr<RandomAccessFile> file;
  Status s = Env::Default()->NewRandomAccessFile(filename, &file);
  if (!s.ok()) {
    Set_TF_Status_from_Status(out_status, s);
    return nullptr;
  }
  auto* reader = new PyRecordReader;
  reader->offset_ = start_offset;
  reader->file_ = file.release();

  RecordReaderOptions options =
      RecordReaderOptions::CreateRecordReaderOptions(compression_type_string);

  reader->reader_ = new RecordReader(reader->file_, options);
  return reader;
}

PyRecordReader::~PyRecordReader() {
  delete reader_;
  delete file_;
}

void PyRecordReader::GetNext(TF_Status* status) {
  if (reader_ == nullptr) {
    Set_TF_Status_from_Status(status,
                              errors::FailedPrecondition("Reader is closed."));
    return;
  }
  Status s = reader_->ReadRecord(&offset_, &record_);
  Set_TF_Status_from_Status(status, s);
}

void PyRecordReader::Close() {
  delete reader_;
  delete file_;
  file_ = nullptr;
  reader_ = nullptr;
}

}  // namespace io
}  // namespace tensorflow
