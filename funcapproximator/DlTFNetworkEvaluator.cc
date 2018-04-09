// https://medium.com/jim-fleming/loading-a-tensorflow-graph-with-the-c-api-4caaff88463f
/*
 * examples:
 * tensorflow/examples/multibox_detector/main.cc:  if (tensorflow::StringPiece(file_name).ends_with(".png")) {
 * tensorflow/examples/label_image/main.cc:  if (tensorflow::StringPiece(file_name).ends_with(".png"))
 */

#include <tensorflow/c/c_api_internal.h>
#include <tensorflow/core/graph/graph_def_builder.h>
#include <tensorflow/cc/ops/const_op.h>
#include "tensorflow/cc/saved_model/tag_constants.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "DlTFNetworkEvaluator.h"
#include "DlTensorUtil.h"
#include "DlGraphUtil.h"
#include "../lib/ArrayUtil.h"
#include "../config/BoardStaticConfig.h"

using namespace tensorflow;
using namespace tensorflow::gtl;

DlTFNetworkEvaluator::DlTFNetworkEvaluator(const string& graphPath) :
    allow_growth(true),
    graph_loaded(false),
    graph_type(GT_UNKNOWN),
    checkpoint_path("bootstrap-ckpt"),
    input_name("feature_input"),
    m_outputs({"resnet/tower_0/policy_head/policy_predict",
               "resnet/tower_0/value_head/reward_predict"}) {
//  m_outputs.emplace_back("policy_head/policy");
//  m_outputs.emplace_back("value_head/reward");

  LoadGraph(graphPath);
}

DlTFNetworkEvaluator::DlTFNetworkEvaluator(const string& graphPath, const string& feature_input,
                     const vector<string>& outputs) :
    allow_growth(true),
    graph_loaded(false),
    graph_type(GT_UNKNOWN),
    checkpoint_path("bootstrap-ckpt"),
    input_name(feature_input),
    m_outputs(outputs) {
  LoadGraph(graphPath);
}


DlTFNetworkEvaluator::~DlTFNetworkEvaluator() {
  m_session->Close();
}

void DlTFNetworkEvaluator::WritePbText(const std::string& path) {
  DlGraphUtil::WriteToFile(meta_graph_def.graph_def(), path);
}

bool DlTFNetworkEvaluator::LoadGraph(const string& graphPath) {
  if (tensorflow::StringPiece(graphPath).ends_with(".meta"))
    return LoadMetaGraph(graphPath);
  else if (tensorflow::StringPiece(graphPath).ends_with(".pb"))
    return LoadPbGraph(graphPath);
  return false;
}

bool DlTFNetworkEvaluator::LoadPbGraph(const string& graphPath) {
  graph_type = GT_PB;

  Status status = ReadBinaryProto(tensorflow::Env::Default(), graphPath, &graph_def);
  if (!status.ok()) {
    throw runtime_error("Failed to load compute graph.");
    // return false;
  }

  DlGraphUtil::WriteToFile(graph_def, "graph.txt");

  auto sessOpt = SessionOptions();
  sessOpt.config.set_allow_soft_placement(true);
  auto* gpuOptions = new GPUOptions();
  gpuOptions->set_allow_growth(allow_growth);
  sessOpt.config.set_allocated_gpu_options(gpuOptions);
  m_session.reset(NewSession(sessOpt));
  if (m_session == nullptr) {
    throw runtime_error("Could not create Tensorflow session.");
  }

  status = m_session->Create(graph_def);
  if (!status.ok()) {
    return false;
  }
  graph_loaded = true;
  return true;
}

bool DlTFNetworkEvaluator::LoadMetaGraph(const string& metaGraphPath) {
  graph_type = GT_META;

  auto sessOpt = SessionOptions();
  sessOpt.config.set_allow_soft_placement(true);
  auto* gpuOptions = new GPUOptions();
  gpuOptions->set_allow_growth(allow_growth);
  sessOpt.config.set_allocated_gpu_options(gpuOptions);
  m_session.reset(NewSession(sessOpt));
  if (m_session == nullptr) {
    throw runtime_error("Could not create Tensorflow session.");
  }

  tensorflow::Status status;
  // Read in the protobuf graph we exported
  status = ReadBinaryProto(Env::Default(), metaGraphPath, &meta_graph_def);
  if (!status.ok()) {
    throw runtime_error("Error reading graph definition from " + metaGraphPath + ": " + status.ToString());
  }

  DlGraphUtil::PruneNodesReachableFrom(const_cast<GraphDef&>(meta_graph_def.graph_def()), "input_producer/Const");

  // DlGraphUtil::OptimizeGraph(const_cast<GraphDef&>(meta_graph_def.graph_def()), m_outputs, input_name);
  // std::vector<std::string> targets({"input"});
  // DlGraphUtil::PruneForReverseReachability(const_cast<GraphDef&>(meta_graph_def.graph_def()), targets);
//  std::string inputName = "input";
//  DlGraphUtil::PruneNodesUnreachableFrom(const_cast<GraphDef&>(meta_graph_def.graph_def()), inputName);

  // DlGraphUtil::WriteToFile(meta_graph_def.graph_def(), "graph.txt");
  DlGraphUtil::RemoveNodeWithNamePrefix(meta_graph_def.graph_def(), "input_variable");
  DlGraphUtil::ClearNodeInputWithNamePrefix(meta_graph_def.graph_def(), "input_variable");
  DlGraphUtil::ClearNodeInputWithNamePrefix(meta_graph_def.graph_def(), "^input_variable");
  DlGraphUtil::RemoveNodeWithName(meta_graph_def.graph_def(), "save/Assign_6");
  DlGraphUtil::ClearNodeInputWithName(meta_graph_def.graph_def(), "save/Assign_6");
  DlGraphUtil::ClearNodeInputWithName(meta_graph_def.graph_def(), "^save/Assign_6");
  // std::cout << "pruned graph size:" << meta_graph_def.graph_def().node_size() << std::endl;
  // DlGraphUtil::WriteToFile(meta_graph_def.graph_def(), "graph.processed.txt");

  // Add the graph to the session
  status = m_session->Create(meta_graph_def.graph_def());
  if (!status.ok()) {
    throw runtime_error("Error creating graph: " + status.ToString());
  }
#ifdef LOG_TENSORFLOW
  std::cout << "DlTFNetworkEvaluator:" << meta_graph_def.graph_def().DebugString() << std::endl;
#endif

  graph_loaded = true;

  return true;
}

bool DlTFNetworkEvaluator::UpdateCheckPointForMetaGraph(const string& _ck_path) {
  if (!_ck_path.empty())
    checkpoint_path = _ck_path;
  if (checkpoint_path.empty())
    return false;

  Tensor checkpointPathTensor(DT_STRING, TensorShape());
  checkpointPathTensor.scalar<std::string>()() = checkpoint_path;
  /*
   * https://stackoverflow.com/questions/39468640/tensorflow-freeze-graph-py-the-name-save-const0-refers-to-a-tensor-which-doe
   * The default name scope for a tf.train.Saver is "save/" and
   * the placeholder is actually a tf.constant() whose name defaults to "Const:0",
   * which explains why the flag defaults to "save/Const:0"
   */
  std::string filename_tensor_name = meta_graph_def.saver_def().filename_tensor_name(); // "save/Const:0"
  std::string restore_op_name = meta_graph_def.saver_def().restore_op_name();
  // TODO optimize?
  //std::string filename_tensor_name = "save/Const:0";
  //std::string restore_op_name = "save/restore_all";
  Status status = m_session->Run(
      {{filename_tensor_name, checkpointPathTensor},},
      {},
      {restore_op_name},
      nullptr);
  if (!status.ok()) {
    // throw runtime_error("Error loading checkpoint from " + checkpointPath + ": " + status.ToString());
    return false;
  }

  return true;
}

// https://stackoverflow.com/questions/37508771/how-to-save-and-restore-a-tensorflow-graph-and-its-state-in-c/37671613#37671613
bool DlTFNetworkEvaluator::UpdateCheckPointForPbGraph(const string& checkpointPath) {
  Tensor checkpointPathTensor(DT_STRING, TensorShape());
  checkpointPathTensor.scalar<std::string>()() = checkpointPath;
  /*
   * https://stackoverflow.com/questions/39468640/tensorflow-freeze-graph-py-the-name-save-const0-refers-to-a-tensor-which-doe
   * The default name scope for a tf.train.Saver is "save/" and
   * the placeholder is actually a tf.constant() whose name defaults to "Const:0",
   * which explains why the flag defaults to "save/Const:0"
   */
  //std::string filename_tensor_name; // = graph_def.filename_tensor_name(); // "save/Const:0"
  //std::string restore_op_name; // = meta_graph_def.saver_def().restore_op_name();
  std::string filename_tensor_name = "save/Const:0";
  std::string restore_op_name = "save/restore_all";
  Status status = m_session->Run(
      {{filename_tensor_name, checkpointPathTensor},},
      {},
      {restore_op_name},
      nullptr);
  if (!status.ok()) {
    return false;
  }

  return true;
}

bool DlTFNetworkEvaluator::UpdateCheckPoint(const string& checkpointPath) {
  if (graph_type == GT_META)
    return UpdateCheckPointForMetaGraph(checkpointPath);
  else if (graph_type == GT_PB)
    return UpdateCheckPointForPbGraph(checkpointPath);
  return false;
}

bool DlTFNetworkEvaluator::MetaGraphLoaded() {
  return graph_loaded;
}

tensorflow::MetaGraphDef& DlTFNetworkEvaluator::GetMetaGraph() {
  return meta_graph_def;
}

#ifdef CHECK_EVAL_RESULT
static void CheckPolicies(double policies_[][GO_MAX_MOVES], int batches) {
  for (int i = 0; i < batches; ++i) {
    double sum = 0;
    for (int j = 0; j < GO_MAX_MOVES; ++j) {
      sum += policies_[i][j];
    }
    // double epsilon = std::numeric_limits<double>::epsilon();
    double epsilon = 1e-6;
    if (fabs(sum - 1.0) > epsilon)
      LOG(WARNING) << "Invalid network evaluation^~:(";
  }
}

static void CheckRange(const double value_[], int length, double low, double high) {
  for (int i = 0; i < length; ++i) {
    if (value_[i] < low || value_[i] > high)
      LOG(WARNING) << "Value out of range^~:(";
  }
}
#endif

void DlTFNetworkEvaluator::CreateTensor(int batches) {
  auto* feature_batch = new float[batches * BD_SIZE * BD_SIZE * NUM_MAPS];
  int64_t dims[] = {batches, BD_SIZE, BD_SIZE, NUM_MAPS};
  DlTensorUtil<float>::Feature2Tensor(feature_batch, dims, sizeof(dims) / sizeof(int64_t), TF_FLOAT,
                                      m_input_tensors[batches - 1]);
}

void DlTFNetworkEvaluator::Evaluate(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE],
                                    double policies_[][GO_MAX_MOVES], double value_[], int numBatches) {
  if (m_input_tensors[numBatches - 1].dim_size(0) != numBatches)
    CreateTensor(numBatches);

  TransformFeature(feature, numBatches);
  std::vector<Tensor> outputs;
  string input_layer = input_name;
  Status run_status = m_session->Run({{input_layer, m_input_tensors[numBatches - 1]}},
      /*{"policy_head/policy", "value_head/reward"}*/m_outputs, {}, &outputs);

  if (run_status.ok() && outputs.size() >= 2) {
    DlTensorUtil<float>::GetValue(outputs[0], policies_[0], GO_MAX_MOVES * numBatches);
    DlTensorUtil<float>::GetValue(outputs[1], value_, numBatches);
#ifdef CHECK_EVAL_RESULT
    ///CheckPolicies(policies_, numBatches);
    ///CheckRange(value_, numBatches, -1, 1);
#endif
  } else {
    LOG(ERROR) << "Running model failed: " << run_status;
  }
}

void DlTFNetworkEvaluator::TransformFeature(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE], int numBatches) {
  int h = BD_SIZE;
  int w = BD_SIZE;
  for (int batchID = 0; batchID < numBatches; ++batchID) {
    for (int i = 0; i < NUM_MAPS; ++i) {
      for (int j = 0; j < h; ++j) {
        for (int k = 0; k < w; ++k) {
          float* data = m_input_tensors[numBatches - 1].flat<float>().data();
          int index = UnrealGo::ArrayUtil::GetOffset({numBatches, BD_SIZE, BD_SIZE, NUM_MAPS}, 4,
                                                     {batchID, j, k, i});
          data[index] = (float)feature[batchID][i][j][k];
        }
      }
    }
  }
}



//void DlTFNetworkEvaluator::Evaluate(char feature[][BD_SIZE][BD_SIZE], int numFeatures,
//                                    float policies_[], int length, float &value_) {
//  int64_t dims[] = {numFeatures, BD_SIZE, BD_SIZE};
//  Tensor input_tensor;
//  DlTensorUtil<char>::Feature2Tensor(&(feature[0][0][0]), dims, sizeof(dims) / sizeof(int64_t), TF_INT8, input_tensor);
//  Input perm = {1, 2, 0};
//  input_tensor = DlTensorUtil<char>::TransposeTensor(input_tensor, perm);
//#ifdef LOG_TENSORFLOW
//  std::cout << "TensorDebug:" << input_tensor.DebugString() << std::endl;
//#endif
//
//  input_tensor = DlTensorUtil<float>::CastTensor(input_tensor, tensorflow::DT_FLOAT);
//#ifdef LOG_TENSORFLOW
//  std::cout << "TensorDebug:" << input_tensor.DebugString() << std::endl;
//#endif
//
//  input_tensor = DlTensorUtil<float>::ExpandTensorDims(input_tensor);
//#ifdef LOG_TENSORFLOW
//  std::cout << "TensorDebug:" << input_tensor.DebugString() << std::endl;
//#endif
//
//  std::vector<Tensor> outputs;
//  string input_layer = input_name;
//  Status run_status = m_session->Run({{input_layer, input_tensor}},
//                                     m_outputs, {}, &outputs);
//
//  DlTensorUtil<float>::GetValue(outputs[0], policies_, length);
//  DlTensorUtil<float>::GetValue(outputs[1], &value_, 1);
//
//  if (!run_status.ok()) {
//    LOG(ERROR) << "Running model failed: " << run_status;
//  }
//}

//void DlTFNetworkEvaluator::TransformFeature(char feature[][BD_SIZE][BD_SIZE], int numFeatures) {
//  int h = BD_SIZE;
//  int w = BD_SIZE;
//  for (int i = 0; i < numFeatures; ++i) {
//    for (int j = 0; j < h; ++j) {
//      for (int k = 0; k < w; ++k) {
//        // m_feature_batch[0][j][k][i] = (float) feature[i][j][k];
//        float* data = (TensorCApi::Buffer(m_inputTensor)->base<float>());
//        int index = UnrealGo::ArrayUtil::GetOffset({BD_SIZE, BD_SIZE, NUM_MAPS}, 4,
//                                                 {j, k, i});
//        data[index] = (float) feature[i][j][k];
//      }
//    }
//  }
//}


//void DlTFNetworkEvaluator::Evaluate(char feature[][BD_SIZE][BD_SIZE], int numFeatures,
//                                    double policies_[], int length, double &value_) {
//  TransformFeature(feature, numFeatures);
//  int64_t dims[] = {1, BD_SIZE, BD_SIZE, numFeatures};
//  Tensor input_tensor;
//  DlTensorUtil<float>::Feature2Tensor(&(m_feature_batch[0][0][0][0]), dims, sizeof(dims) / sizeof(int64_t), TF_FLOAT,
//                                      input_tensor);
//
//  std::vector<Tensor> outputs;
//  string input_layer = input_name;
//  Status run_status = m_session->Run({{input_layer, input_tensor}},
//                                     m_outputs, {}, &outputs);
//  DlTensorUtil<float>::GetValue(outputs[0], policies_, length);
//  DlTensorUtil<float>::GetValue(outputs[1], &value_, 1);
//  //DlTensorUtil<float>::PrintTensor(outputs[0]);
//  if (!run_status.ok()) {
//    LOG(ERROR) << "Running model failed: " << run_status;
//  }
//}



/*
void DlTFNetworkEvaluator::Evaluate(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE], int numBatches,
                                    float actions_[], int length, float &value_) {
  int64_t dims[] = {numBatches, NUM_MAPS, BD_SIZE, BD_SIZE};
  Tensor input_tensor;
  DlTensorUtil<char>::Feature2Tensor(&(feature[0][0][0][0]), dims, sizeof(dims) / sizeof(int64_t), TF_INT8,
                                     input_tensor);
  Input perm = {0, 2, 3, 1};
  input_tensor = DlTensorUtil<char>::TransposeTensor(input_tensor, perm);
#ifdef LOG_TENSORFLOW
  std::cout << "TensorDebug:" << input_tensor.DebugString() << std::endl;
#endif

  input_tensor = DlTensorUtil<float>::CastTensor(input_tensor, tensorflow::DT_FLOAT);
#ifdef LOG_TENSORFLOW
  std::cout << "TensorDebug:" << input_tensor.DebugString() << std::endl;
#endif

  std::vector<Tensor> outputs;
  string input_layer = input_name;
  Status run_status = m_session->Run({{input_layer, input_tensor}},
      */
/*{"policy_head/policy", "value_head/reward"}*//*
m_outputs, {}, &outputs);

  DlTensorUtil<float>::GetValue(outputs[0], actions_, length);
  DlTensorUtil<float>::GetValue(outputs[1], &value_, 1);

//  DlTensorUtil<float>::PrintTensor(outputs[0]);
  if (!run_status.ok()) {
    LOG(ERROR) << "Running model failed: " << run_status;
  }
}*/
