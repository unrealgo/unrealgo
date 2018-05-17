
#include <tensorflow/c/c_api_internal.h>
#include <tensorflow/cc/framework/scope.h>
#include <google/tensorflow/tensorflow/cc/ops/math_ops.h>
#include <google/tensorflow/tensorflow/cc/ops/array_ops.h>
#include "platform/SgSystem.h"
#include "DlTensorUtil.h"

using namespace tensorflow;

template
class DlTensorUtil<float>;
template
class DlTensorUtil<char>;
template
class DlTensorUtil<bool>;

static void Deallocator(void* data, size_t, void* arg) {
  SuppressUnused(data);
  SuppressUnused(arg);
  // TODO fixme is it necessary here? sometimes "Process finished with exit code 139 (interrupted by signal 11: SIGSEGV" is met
//  tensorflow::cpu_allocator()->DeallocateRaw(data);
//  *reinterpret_cast<bool *>(arg) = true;
}

static Tensor RunGraph(GraphDef& graph, Tensor& input, const string& out) {
#ifdef LOG_TENSORFLOW
  std::cout << graph.node(0).name() << std::endl;
  std::cout << graph.DebugString() << std::endl;
  std::cout << "Node Size:" << graph.node_size() << std::endl;
#endif

  std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
      {graph.node(0).name(), input},
  };
  std::vector<Tensor> out_tensors;
  auto sessOpt = SessionOptions();
  sessOpt.config.set_allow_soft_placement(true);
  auto* gpuOptions = new GPUOptions();
  gpuOptions->set_allow_growth(true);
  sessOpt.config.set_allocated_gpu_options(gpuOptions);
  std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession(sessOpt));
  Status status = (session->Create(graph));
  status = (session->Run({inputs}, {out}, {}, &out_tensors));

  if (status.ok())
    return out_tensors[0];
  return Tensor();
}

static Status RunGraph(GraphDef& graph, Tensor& input, const string& out_name, std::vector<Tensor>* out_tensors) {

#ifdef LOG_TENSORFLOW
  std::cout << graph.node(0).name() << std::endl;
  std::cout << graph.DebugString() << std::endl;
  std::cout << "Node Size:" << graph.node_size() << std::endl;
#endif

  std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
      {graph.node(0).name(), input},
  };
  auto sessOpt = SessionOptions();
  sessOpt.config.set_allow_soft_placement(true);
  auto* gpuOptions = new GPUOptions();
  gpuOptions->set_allow_growth(true);
  sessOpt.config.set_allocated_gpu_options(gpuOptions);
  std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession(sessOpt));
  Status status = (session->Create(graph));
  status = (session->Run({inputs}, {out_name}, {}, out_tensors));

  return status;
}

// https://github.com/tensorflow/fold/blob/master/tensorflow_fold/llgtm/backend/tf_evaluator.cc
// https://github.com/tensorflow/tensorflow/issues/8033
template<class T>
TF_Tensor* DlTensorUtil<T>::Feature2TF_Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type) {
  size_t len = 1;
  for (int i = 0; i < nDims; i++)
    len *= dims[i];
  len *= sizeof(T);
  bool deallocator_called = false;
  TF_Tensor* ts = TF_NewTensor(type, dims, nDims, feature, len, &Deallocator, &deallocator_called);
  return ts;
}

template<class T>
Tensor DlTensorUtil<T>::Feature2Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type) {
  TF_Tensor* tftensor = Feature2TF_Tensor(feature, dims, nDims, type);
  Tensor input_tensor = tensorflow::TensorCApi::MakeTensor(tftensor->dtype, tftensor->shape, tftensor->buffer);
  return input_tensor;
}

template<class T>
void DlTensorUtil<T>::Feature2Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type, Tensor& out_tensor) {
  TF_Tensor* tftensor = Feature2TF_Tensor(feature, dims, nDims, type);
  out_tensor = tensorflow::TensorCApi::MakeTensor(tftensor->dtype, tftensor->shape, tftensor->buffer);
}

template<class T>
Status DlTensorUtil<T>::CastTensor(Tensor& input, DataType dataType, std::vector<Tensor>* out_tensors) {
  string out_name = "Cast";
  auto root = tensorflow::Scope::NewRootScope();
// Now try to figure out what kind of file it is and decode it.
  auto uint8_caster = tensorflow::ops::Cast(root.WithOpName(out_name), input, dataType);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  return RunGraph(graph, input, out_name, out_tensors);
}

template<class T>
Tensor DlTensorUtil<T>::CastTensor(Tensor& input, DataType dataType) {
  string out_name = "Cast";
  auto root = tensorflow::Scope::NewRootScope();
// Now try to figure out what kind of file it is and decode it.
  auto uint8_caster = tensorflow::ops::Cast(root.WithOpName(out_name), input, dataType);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  if (status.ok())
    return RunGraph(graph, input, out_name);
  return Tensor();
}

template<class T>
Status DlTensorUtil<T>::ExpandTensorDims(Tensor& input, std::vector<Tensor>* out_tensors) {
  string out_name = "Expand";
  auto root = tensorflow::Scope::NewRootScope();
  auto dims_expander = tensorflow::ops::ExpandDims(root.WithOpName(out_name), input, 0);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  return RunGraph(graph, input, out_name, out_tensors);
}

template<class T>
Tensor DlTensorUtil<T>::ExpandTensorDims(Tensor& input) {
  string out_name = "Expand";
  auto root = tensorflow::Scope::NewRootScope();
  auto dims_expander = tensorflow::ops::ExpandDims(root.WithOpName(out_name), input, 0);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  if (status.ok())
    return RunGraph(graph, input, out_name);
  return Tensor();
}

template<class T>
Status DlTensorUtil<T>::TransposeTensor(Tensor& input, const Input& perm, std::vector<Tensor>* out_tensors) {
  string out_name = "Transpose";
  auto root = tensorflow::Scope::NewRootScope();
  auto dims_expander = tensorflow::ops::Transpose(root.WithOpName(out_name), input, perm);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  return RunGraph(graph, input, out_name, out_tensors);
}

template<class T>
Tensor DlTensorUtil<T>::TransposeTensor(Tensor& input, const Input& perm) {
  string out_name = "Transpose";
  auto root = tensorflow::Scope::NewRootScope();
  auto dims_expander = tensorflow::ops::Transpose(root.WithOpName(out_name), input, perm);
  tensorflow::GraphDef graph;
  Status status = (root.ToGraphDef(&graph));

  if (status.ok())
    return RunGraph(graph, input, out_name);
  return Tensor();
}

template<typename T>
void PrintOneDim(int dim_index, gtl::InlinedVector<int64, 4> shape, int64 limit,
                 int shape_size, T* data, int64* data_index, string* result) {
  if (*data_index >= limit) return;
  int64 element_count = shape[dim_index];
  // We have reached the right-most dimension of the tensor.
  if (dim_index == shape_size - 1) {
    for (int64 i = 0; i < element_count; i++) {
      if (*data_index >= limit) return;
      if (i > 0) strings::StrAppend(result, " ");
      strings::StrAppend(result, data[(*data_index)++]);
    }
    return;
  }
  // Loop every element of one dim.
  for (int64 i = 0; i < element_count; i++) {
    bool flag = false;
    if (*data_index < limit) {
      strings::StrAppend(result, "[");
      flag = true;
    }
    // As for each element, print the sub-dim.
    PrintOneDim(dim_index + 1, shape, limit, shape_size, data, data_index,
                result);
    if (*data_index < limit || flag) {
      strings::StrAppend(result, "]");
      flag = false;
    }
  }
}

template<typename T>
string SummarizeArray(int64 limit, int64 num_elts,
                      const TensorShape& tensor_shape, const char* data) {
  string ret;
  auto* array = reinterpret_cast<const T*>(data);
  const gtl::InlinedVector<int64, 4> shape = tensor_shape.dim_sizes();
  if (shape.empty()) {
    for (int64 i = 0; i < limit; ++i) {
      if (i > 0) strings::StrAppend(&ret, " ");
      strings::StrAppend(&ret, array[i]);
    }
    if (num_elts > limit) strings::StrAppend(&ret, "...");
    return ret;
  }
  int64 data_index = 0;
  const int shape_size = tensor_shape.dims();
  PrintOneDim(0, shape, limit, shape_size, array, &data_index, &ret);

  if (num_elts > limit) strings::StrAppend(&ret, "...");
  return ret;
}

template<class T>
void DlTensorUtil<T>::PrintTensor(const Tensor& tensor) {
  for (int i = 0; i < tensor.dims(); i++)
    std::cout << tensor.dim_size(i) << " ";
  std::cout << std::endl;

  int64 size = tensor.NumElements();
  float sum = 0;
  std::cout << SummarizeArray<float>(size, size, tensor.shape(), tensor.tensor_data().data()) << std::endl;
  std::cout << sum << std::endl;
}

template<class T>
void DlTensorUtil<T>::GetValue(const Tensor& tensor, T* out_, int length) {
  const T* array = reinterpret_cast<const T*>(tensor.tensor_data().data());
  for (int i = 0; i < length; i++)
    out_[i] = array[i];
}

template<class T>
void DlTensorUtil<T>::GetValue(const Tensor& tensor, double* out_, int length) {
  const T* array = reinterpret_cast<const T*>(tensor.tensor_data().data());
  for (int i = 0; i < length; i++)
    out_[i] = array[i];
}