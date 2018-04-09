
#include <zmq.hpp>
#include "MinioStub.h"
#include "ZmqUtil.h"
#include "funcapproximator/DlConfig.h"

void UnrealGo::MinioStub::SetObjPolicy(const std::string& bucket, const std::string& obj, const std::string& policy) {
  zmq::context_t ctx;
  zmq::socket_t socket(ctx, ZMQ_REQ);
  socket.connect(DlConfig::GetInstance().get_minioproxyserver_socket());
  std::string command = "setpolicy:" + bucket + ":" + obj + ":" + policy;
  ZmqUtil::sendData(socket, command);
}

void UnrealGo::MinioStub::SetObjPolicy(const std::string& path, const std::string& policy) {
  size_t index = path.find('/');
  SetObjPolicy(path.substr(0, index), path.substr(index+1), policy);
}

void UnrealGo::MinioStub::Upload(const std::string& bucket, const std::string& obj, const std::string& localFilePath) {
  zmq::context_t ctx;
  zmq::socket_t socket(ctx, ZMQ_REQ);
  socket.connect(DlConfig::GetInstance().get_minioproxyserver_socket());
  std::string command = "upload:" + bucket + ":" + obj + ":" + localFilePath;
  ZmqUtil::sendData(socket, command);
}