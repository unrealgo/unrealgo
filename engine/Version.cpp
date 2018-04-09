
#include <sstream>
#include "Version.h"

#ifndef APP_NAME
#define APP_NAME "UnrealGo"
#endif

#ifndef APP_VERSION
#define APP_VERSION "1.0"
#endif

namespace UnrealGo {
std::string GetVersion() {
  std::ostringstream os;

#ifdef APP_VERSION
  os << APP_VERSION;
#else
  os << "(" __DATE__ ")";
#endif

#ifdef SVNREV
  os << "(" SVNREV ")";
#endif

#ifndef NDEBUG
  os << " (dbg)";
#endif

  return os.str();
}

std::string License(const std::string& prefix) {
  std::ostringstream os;
  // os << "\n" <<
  os << prefix <<
     APP_NAME << " " << GetVersion() << "\n" <<
     "Copyright (C) 2017-2018 by Kelvin Liu.\n"
         APP_NAME " comes with NO WARRANTY to the extent permitted by law.\n"
         "This program is free software; you can redistribute it and/or modify it\n"
         "under the terms of the GNU Lesser General Public License as published by the\n"
         "Free Software Foundation - version 3.\n";

  return os.str();
}
}