
#include <zmq_addon.hpp>
#include "ZmqUtil.h"

void ZmqUtil::sendData(zmq::socket_t &socket, const std::string &data) {
  zmq::message_t request(data.size());
  memcpy(request.data(), data.c_str(), data.size());
  socket.send(request);

  zmq::message_t reply;
  socket.recv(&reply);
}

void ZmqUtil::send_command(zmq::socket_t &socket, int cmd, const std::string &data) {
  UnrealGo::Command command;
  command.set_type(cmd);
  command.set_data(data);
  zmq::message_t request(command.ByteSizeLong());
  command.SerializeToArray(request.data(), command.ByteSize());
  socket.send(request);

  zmq::message_t reply;
  socket.recv(&reply);
  // std::cout << "ACK Received " << std::endl;
}

void ZmqUtil::receive_command(zmq::socket_t &socket, UnrealGo::Command &command, zmq::message_t &reply) {
  zmq::message_t request;
  socket.recv(&request);
  // too short for server to receive response
  command.ParseFromArray(request.data(), (int) request.size());
  socket.send(reply);
}
