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

char image[MAX_BATCHES][NUM_MAPS][BD_SIZE][BD_SIZE];

void printValue(double value[], int size) {
  for (int i = 0; i < size; ++i)
    std::cout << value[i] << " ";
  std::cout << std::endl;
}

void Test(DlTFNetworkEvaluator<bool>& evaluator, int batchSize = 1) {
  double policy[MAX_BATCHES][GO_MAX_MOVES];
  double value[MAX_BATCHES];
  for (int times = 0; times < 2; ++times) {
    for (int batchID = 0; batchID < batchSize; ++batchID)
      for (int i = 0; i < NUM_MAPS; i++)
        for (int j = 0; j < BD_SIZE; j++)
          for (int k = 0; k < BD_SIZE; k++)
            image[batchID][i][j][k] = (rand() % 128 == 0) ? 0:1;

    evaluator.Evaluate(image, policy, value, batchSize);
    printValue(value, batchSize);
  }
}

int main(int argc, char** argv) {
  DlTFNetworkEvaluator<bool> evaluator("unreal0.ckpt.meta", "inputs",
                                       {"policy", "value"}, DF_HWC);
  if (evaluator.UpdateCheckPoint("unreal0.ckpt"))
    std::cout << "checkpoint loaded successfully!" << std::endl;

  Test(evaluator); // Verified
  Test(evaluator, 4);
  Test(evaluator, 8);
  Test(evaluator, 16);

  evaluator.WritePbText("unreal0.pb.txt");
  return 0;
}