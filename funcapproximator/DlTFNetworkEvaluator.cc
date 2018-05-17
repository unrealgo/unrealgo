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
#include "DlConfig.h"

using namespace tensorflow;
using namespace tensorflow::gtl;

template class DlTFNetworkEvaluator<bool>;
template class DlTFNetworkEvaluator<float>;

template <typename T>
DlTFNetworkEvaluator<T>::DlTFNetworkEvaluator(const string& graphPath, DataFormat _df) :
    m_dataFormat(_df),
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

template <typename T>
DlTFNetworkEvaluator<T>::DlTFNetworkEvaluator(const string& graphPath, const string& feature_input,
                     const vector<string>& outputs, DataFormat _df) :
    m_dataFormat(_df),
    allow_growth(true),
    graph_loaded(false),
    graph_type(GT_UNKNOWN),
    checkpoint_path("bootstrap-ckpt"),
    input_name(feature_input),
    m_outputs(outputs) {
  LoadGraph(graphPath);
}

template <typename T>
void DlTFNetworkEvaluator<T>::SetNetworkInput(std::string input) {
  input_name = input;
}

template <typename T>
void DlTFNetworkEvaluator<T>::SetNetworkOutput(std::vector<std::string>& output) {
  m_outputs.clear();
  m_outputs = output;
}

template <typename T>
DlTFNetworkEvaluator<T>::~DlTFNetworkEvaluator() {
  m_session->Close();
}

template <typename T>
void DlTFNetworkEvaluator<T>::WritePbText(const std::string& path) {
  DlGraphUtil::WriteToFile(meta_graph_def.graph_def(), path);
}

template <typename T>
bool DlTFNetworkEvaluator<T>::LoadGraph(const string& graphPath) {
  if (tensorflow::StringPiece(graphPath).ends_with(".meta"))
    return LoadMetaGraph(graphPath);
  else if (tensorflow::StringPiece(graphPath).ends_with(".pb"))
    return LoadPbGraph(graphPath);
  return false;
}

template <typename T>
bool DlTFNetworkEvaluator<T>::LoadPbGraph(const string& graphPath) {
  graph_type = GT_PB;

  Status status = ReadBinaryProto(tensorflow::Env::Default(), graphPath, &graph_def);
  if (!status.ok()) {
    throw runtime_error("Failed to load compute graph.");
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

template <typename T>
bool DlTFNetworkEvaluator<T>::LoadMetaGraph(const string& metaGraphPath) {
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

#ifdef PRUNE_GRAPH
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
#endif

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

template <typename T>
bool DlTFNetworkEvaluator<T>::UpdateCheckPointForMetaGraph(const string& _ck_path) {
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
template <typename T>
bool DlTFNetworkEvaluator<T>::UpdateCheckPointForPbGraph(const string& checkpointPath) {
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

template <typename T>
bool DlTFNetworkEvaluator<T>::UpdateCheckPoint(const string& checkpointPath) {
  if (graph_type == GT_META)
    return UpdateCheckPointForMetaGraph(checkpointPath);
  else if (graph_type == GT_PB)
    return UpdateCheckPointForPbGraph(checkpointPath);
  return false;
}

template <typename T>
bool DlTFNetworkEvaluator<T>::MetaGraphLoaded() {
  return graph_loaded;
}

template <typename T>
tensorflow::MetaGraphDef& DlTFNetworkEvaluator<T>::GetMetaGraph() {
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

template <typename T>
void DlTFNetworkEvaluator<T>::CreateTensor(int batches) {
  auto* feature_batch = new T[batches * BD_SIZE * BD_SIZE * NUM_MAPS];
  int64_t dims[] = {batches, BD_SIZE, BD_SIZE, NUM_MAPS};
  const bool T_is_BOOL = std::is_same<T, bool>::value;
  const bool T_is_FLOAT = std::is_same<T, float>::value;
  TF_DataType dataType = TF_BOOL;
  if (T_is_BOOL)
    dataType = TF_BOOL;
  else if (T_is_FLOAT)
    dataType = TF_FLOAT;

  DlTensorUtil<T>::Feature2Tensor(feature_batch, dims, sizeof(dims) / sizeof(int64_t), dataType,
                                      m_input_tensors[batches - 1]);
}

template <typename T>
void DlTFNetworkEvaluator<T>::Evaluate(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE],
                                    double policies_[][GO_MAX_MOVES], double value_[], int numBatches) {
  if (m_input_tensors[numBatches - 1].dim_size(0) != numBatches)
    CreateTensor(numBatches);

  TransformFeature(feature, numBatches);
  std::vector<Tensor> outputs;
  string input_layer = input_name;
  Status run_status = m_session->Run({{input_layer, m_input_tensors[numBatches - 1]}}, m_outputs, {}, &outputs);

  if (run_status.ok() && outputs.size() >= 2) {
    DlTensorUtil<float>::GetValue(outputs[0], policies_[0], GO_MAX_MOVES * numBatches);
    DlTensorUtil<float>::GetValue(outputs[1], value_, numBatches);
    ValueTransformType vt = DlConfig::GetInstance().get_value_transform();
    switch (vt) {
      case TR_FLIP_SIGN:
        for (int i=0; i<numBatches; ++i)
          value_[i] = - value_[i];
        break;
      case TR_FLIP_PROB:
        for (int i=0; i<numBatches; ++i)
          value_[i] = 1 - value_[i];
        break;
    }
#ifdef CHECK_EVAL_RESULT
    ///CheckPolicies(policies_, numBatches);
    ///CheckRange(value_, numBatches, -1, 1);
#endif
  } else {
    LOG(ERROR) << "Running model failed: " << run_status;
  }
}

template <typename T>
void DlTFNetworkEvaluator<T>::Evaluate(char feature[][BD_SIZE][BD_SIZE][NUM_MAPS],
                                       double policies_[][GO_MAX_MOVES], double value_[], int batch_size) {
  Tensor feature_tensor(DT_BOOL, TensorShape({batch_size, FEATURE_SIZE}));
  auto matrix = feature_tensor.matrix<bool>();
  for (int i = 0; i < batch_size; ++i) {
    for (int j = 0; j < FEATURE_SIZE; ++j) {
      matrix(i, j) = (bool)((char*)feature[i])[j];
    }
  }

  std::vector<Tensor> outputs;
  Status run_status = m_session->Run({{input_name, feature_tensor}}, m_outputs, {}, &outputs);

  if (run_status.ok() && outputs.size() >= 2) {
    auto policy_tensor = outputs[0].matrix<float>();
    auto value_tensor  = outputs[1].flat<float>();
    for (int i = 0; i < batch_size; ++i) {
      for (int j = 0; j < GO_MAX_MOVES; ++j) {
        policies_[i][j] = policy_tensor(i, j);
      }
      value_[i] = -value_tensor(i);
    }
//    DlTensorUtil<float>::GetValue(outputs[0], policies_[0], GO_MAX_MOVES * batch_size);
//    DlTensorUtil<float>::GetValue(outputs[1], value_, batch_size);
#ifdef CHECK_EVAL_RESULT
    CheckPolicies(policies_, batch_size);
    ///CheckRange(value_, batch_size, -1, 1);
#endif
  } else {
    LOG(ERROR) << "Running model failed: " << run_status;
  }
}

template <typename T>
void DlTFNetworkEvaluator<T>::printFeature(T data[]) {
  for (int i=0; i<BD_SIZE*BD_SIZE; ++i) {
    for (int j=0; j<NUM_MAPS; ++j) {
      printf("%d ", (int)data[i*NUM_MAPS+j]);
    }
    printf("\n");
  }
  printf("\n");
}

template <typename T>
void DlTFNetworkEvaluator<T>::TransformFeature(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE], int numBatches) {
  int h = BD_SIZE;
  int w = BD_SIZE;
  T* data = m_input_tensors[numBatches - 1].flat<T>().data();
  for (int batchID = 0; batchID < numBatches; ++batchID) {
    for (int depth = 0; depth < NUM_MAPS; ++depth) {
      for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
          int index = 0;
          if (m_dataFormat == DF_HWC)
            index = UnrealGo::ArrayUtil::GetOffset({numBatches, BD_SIZE, BD_SIZE, NUM_MAPS}, 4,
                                                     {batchID, i, j, depth});
          else if (m_dataFormat == DF_CHW)
            index = UnrealGo::ArrayUtil::GetOffset({numBatches, NUM_MAPS, BD_SIZE, BD_SIZE}, 4,
                                                   {batchID, depth, i, j});
          data[index] = (T)feature[batchID][depth][i][j];
        }
      }
    }
  }

//  if (m_dataFormat == DF_HWC)
//    printFeature(data);
}

template <typename T>
void DlTFNetworkEvaluator<T>::TransformFeature(char feature[][BD_SIZE][BD_SIZE][NUM_MAPS], int numBatches) {
  int h = BD_SIZE;
  int w = BD_SIZE;
  T* data = m_input_tensors[numBatches - 1].flat<T>().data();
  for (int batchID = 0; batchID < numBatches; ++batchID) {
    for (int depth = 0; depth < NUM_MAPS; ++depth) {
      for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
          int index = 0;
          if (m_dataFormat == DF_HWC)
            index = UnrealGo::ArrayUtil::GetOffset({numBatches, BD_SIZE, BD_SIZE, NUM_MAPS}, 4,
                                                   {batchID, i, j, depth});
          else if (m_dataFormat == DF_CHW)
            index = UnrealGo::ArrayUtil::GetOffset({numBatches, NUM_MAPS, BD_SIZE, BD_SIZE}, 4,
                                                   {batchID, depth, i, j});
          data[index] = (T)feature[batchID][depth][i][j];
        }
      }
    }
  }
}