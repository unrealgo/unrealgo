
#include "tensorflow/python/lib/io/py_record_writer.h"
#include "tensorflow/c/tf_status_helper.h"
#include "tensorflow/core/lib/io/record_writer.h"

namespace tensorflow {
namespace io {

PyRecordWriter::PyRecordWriter() {}

PyRecordWriter* PyRecordWriter::New(const string& filename,
                                    const string& compression_type_string,
                                    TF_Status* out_status) {
  std::unique_ptr<WritableFile> file;
  Status s = Env::Default()->NewWritableFile(filename, &file);
  if (!s.ok()) {
    Set_TF_Status_from_Status(out_status, s);
    return nullptr;
  }
  PyRecordWriter* writer = new PyRecordWriter;
  writer->file_ = std::move(file);

  RecordWriterOptions options =
      RecordWriterOptions::CreateRecordWriterOptions(compression_type_string);

  writer->writer_.reset(new RecordWriter(writer->file_.get(), options));
  return writer;
}

PyRecordWriter::~PyRecordWriter() {
}

bool PyRecordWriter::WriteRecord(tensorflow::StringPiece record) {
  if (writer_ == nullptr) return false;
  Status s = writer_->WriteRecord(record);
  return s.ok();
}

void PyRecordWriter::Flush(TF_Status* out_status) {
  Status s = writer_->Flush();
  if (!s.ok()) {
    Set_TF_Status_from_Status(out_status, s);
    return;
  }
}

void PyRecordWriter::Close(TF_Status* out_status) {
  Status s = writer_->Close();
  if (!s.ok()) {
    Set_TF_Status_from_Status(out_status, s);
    return;
  }
  writer_.reset(nullptr);
  s = file_->Close();
  if (!s.ok()) {
    Set_TF_Status_from_Status(out_status, s);
    return;
  }
  file_.reset(nullptr);
}

}  // namespace io
}  // namespace tensorflow
