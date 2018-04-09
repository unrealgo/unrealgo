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

#ifndef UNREALGO_MINIOSTUB_H
#define UNREALGO_MINIOSTUB_H

#include <string>

namespace UnrealGo {
namespace MinioStub {
  void Upload(const std::string& bucket, const std::string& obj, const std::string& localFilePath);
  void SetObjPolicy(const std::string& bucket, const std::string& obj, const std::string& policy);
  void SetObjPolicy(const std::string& path, const std::string& policy);
}
}
#endif //UNREALGO_MINIOSTUB_H
