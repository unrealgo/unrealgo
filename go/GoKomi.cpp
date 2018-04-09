
#include "platform/SgSystem.h"
#include "GoKomi.h"
#include "lib/StringUtil.h"

#include <sstream>

namespace {
std::string GetInvalidKomiErrorMessage(float komi) {
  std::ostringstream buffer;
  buffer << "Invalid komi value: " << komi;
  return buffer.str();
}
}

GoKomi::InvalidKomi::InvalidKomi(float komi)
    : SgException(GetInvalidKomiErrorMessage(komi)) {}

GoKomi::InvalidKomi::InvalidKomi(const std::string& komi)
    : SgException("Invalid komi value: " + komi) {}

GoKomi::GoKomi(const std::string& komi) {
  std::string trimmedString = komi;
  UnrealGo::StringUtil::trim(trimmedString);
  if (trimmedString.empty()) {
    unknown = true;
    value = 0;
    return;
  }

  std::istringstream buffer(komi);
  float value;
  buffer >> value;
  if (!buffer)
    throw InvalidKomi(komi);
  *this = GoKomi(value);
}

std::string GoKomi::ToString() const {
  if (unknown)
    return "";

  if (value % 2 == 0) {
    std::ostringstream buffer;
    buffer << (value / 2);
    return buffer.str();
  } else if (value == -1)
    return "-0.5";
  else {
    std::ostringstream buffer;
    buffer << (value / 2) << ".5";
    return buffer.str();
  }
}