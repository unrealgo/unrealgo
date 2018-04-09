/*
This file is part of UnrealGo.
Copyright (C) 2017 Kevin
UnrealGo is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
UnrealGo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with UnrealGo.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UNREALGO_ATOMZMQ_H
#define UNREALGO_ATOMZMQ_H

#include <zmq.hpp>
#include "msg/command.pb.h"

namespace ZmqUtil {
  void sendData(zmq::socket_t &socket, const std::string &data);

  void send_command(zmq::socket_t &socket, int cmd, const std::string &data);

  void receive_command(zmq::socket_t &socket, UnrealGo::Command &command, zmq::message_t &reply);
}
#endif //UNREALGO_ATOMZMQ_H