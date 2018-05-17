#include "tensorflow/cc/ops/io_ops.h"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

#include <tensorflow/c/tf_status_helper.h>
#include "tensorflow/core/lib/core/coding.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/hash/crc32c.h"
#include "tensorflow/core/lib/io/record_reader.h"
#include "tensorflow/core/lib/io/record_writer.h"
#include "tensorflow/core/lib/random/simple_philox.h"
#include "tensorflow/core/example/feature_util.h"

#include "tensorflow/core/lib/io/compression.h"
#include "tensorflow/c/c_api_internal.h"
#include "tensorflow/python/lib/io/py_record_writer.h"
#include "tensorflow/python/lib/io/py_record_reader.h"
#include "tensorflow/core/protobuf/meta_graph.pb.h"
#include "../DlTFRecordWriter.h"
#include "../DlTFNetworkEvaluator.h"
#include "../DlTensorUtil.h"
#include "../DlConfig.h"

using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;
using namespace tensorflow;
using namespace tensorflow::ops;
using namespace std;
using namespace tensorflow::io;

int main(int argc, char** argv) {
  DlTFNetworkEvaluator<bool> evaluator("bootstrap-ckpt.meta", DF_HWC);
  evaluator.WritePbText("bootstrap-ckpt.pb.txt");
  return 0;
}