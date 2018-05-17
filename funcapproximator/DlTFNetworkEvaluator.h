
#ifndef DL_TF_NETWORKEVALUATOR_H
#define DL_TF_NETWORKEVALUATOR_H

#include <tensorflow/core/framework/tensor.h>
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/protobuf/meta_graph.pb.h"

#include "config/BoardStaticConfig.h"
#include "funcapproximator/DataFormat.h"

const int BD_SIZE = GO_DEFINE_MAX_SIZE;
const int NUM_MAPS = 17;
const int FEATURE_SIZE = BD_SIZE * BD_SIZE * NUM_MAPS;
const int MAX_BATCHES = 16; // fixed for tf metagraph

struct TF_Tensor;
namespace tensorflow {
enum GraphType {
  GT_META,
  GT_PB,
  GT_UNKNOWN
};

class GraphDef;
class MetaGraphDef;
class Session;

using namespace tensorflow;
using namespace std;

template <typename T>
class DlTFNetworkEvaluator {
 public:
  explicit DlTFNetworkEvaluator(const string& graphPath, DataFormat _df=DF_HWC);
  DlTFNetworkEvaluator(const string& graphPath, const string& feature_input,
                        const vector<string>& outputs, DataFormat _df=DF_HWC);
  ~DlTFNetworkEvaluator();
  void CreateTensor(int batches);
  void Evaluate(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE],
                double actions_[][GO_MAX_MOVES],
                double value_[],
                int numBatches = MAX_BATCHES);

  void Evaluate(char feature[][BD_SIZE][BD_SIZE][NUM_MAPS],
                double actions_[][GO_MAX_MOVES],
                double value_[],
                int batch_size = MAX_BATCHES);


  void printFeature(T data[]);

  void TransformFeature(char feature[][NUM_MAPS][BD_SIZE][BD_SIZE], int numBatches = MAX_BATCHES);
  void TransformFeature(char feature[][BD_SIZE][BD_SIZE][NUM_MAPS], int numBatches = MAX_BATCHES);
  void WritePbText(const std::string& path);
  bool LoadGraph(const string& graphPath);
  bool UpdateCheckPoint(const string& checkpointPath); // load the checkpoint, model parameters

  bool MetaGraphLoaded();
  tensorflow::MetaGraphDef& GetMetaGraph();

  void SetNetworkInput(std::string input);
  void SetNetworkOutput(std::vector<std::string>& output);

 protected:
  bool LoadPbGraph(const string& graphPath); // load graph
  bool LoadMetaGraph(const string& metaGraphPath); // load graph
  bool UpdateCheckPointForMetaGraph(const string& checkpointPath);
  bool UpdateCheckPointForPbGraph(const string& checkpointPath);

 protected:
  DataFormat m_dataFormat;
  bool allow_growth;
  bool graph_loaded;
  GraphType graph_type;
  std::string checkpoint_path;
  std::string input_name;
  tensorflow::GraphDef graph_def;
  tensorflow::MetaGraphDef meta_graph_def;
  std::unique_ptr<Session> m_session;
  std::vector<std::string> m_outputs;
  Tensor m_input_tensors[MAX_BATCHES];
};
}

#endif //DL_TF_NETWORKEVALUATOR_H