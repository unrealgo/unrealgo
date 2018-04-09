
#ifndef TFRECORD_GONET_DLTENSORUTIL_H
#define TFRECORD_GONET_DLTENSORUTIL_H

namespace tensorflow {

template<class T>
class DlTensorUtil {
 public:
  static TF_Tensor* Feature2TF_Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type);
  static Tensor Feature2Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type);
  static void Feature2Tensor(T feature[], int64_t dims[], int nDims, TF_DataType type, Tensor& out_tensor);
  static Status CastTensor(Tensor& input, DataType dataType, std::vector<Tensor>* out_tensors);
  static Status ExpandTensorDims(Tensor& input, std::vector<Tensor>* out_tensors);
  static Status TransposeTensor(Tensor& input, const Input& perm, std::vector<Tensor>* out_tensors);
  static Tensor CastTensor(Tensor& input, DataType dataType);
  static Tensor ExpandTensorDims(Tensor& input);
  static Tensor TransposeTensor(Tensor& input, const Input& perm);
  static void PrintTensor(const Tensor& tensor);
  static void GetValue(const Tensor& tensor, T* out_, int length);
  static void GetValue(const Tensor& tensor, double* out_, int length);
};
}

#endif //TFRECORD_GONET_DLTENSORUTIL_H
