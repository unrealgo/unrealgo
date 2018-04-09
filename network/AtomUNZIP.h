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

#ifndef _ATOMUNZIP_H
#define _ATOMUNZIP_H

namespace UnrealGo {
  namespace UNZIP {
    int extract_zip_file(const char *zipfilename);
  }
};

#endif //_ATOMUNZIP_H
