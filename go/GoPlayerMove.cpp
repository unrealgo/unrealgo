

#include "platform/SgSystem.h"
#include "GoPlayerMove.h"

#include <iostream>

std::ostream& operator<<(std::ostream& out, GoPlayerMove mv) {
  out << (mv.Color() == SG_BLACK ? "B " : "W ")
      << GoWritePoint(mv.Point());
  return out;
}
