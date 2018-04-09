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

#ifndef _ATOMZIP_H
#define _ATOMZIP_H

#include <string>
#include <vector>

namespace UnrealGo {
  namespace ZIP {
    int compress_zip_files(const std::vector<std::string> &file_list, const std::string &zip_out);

    int compress_zip_files(const std::vector<std::string> &file_list, const char *zip_out);
  }
};

#endif //_ATOMZIP_H
