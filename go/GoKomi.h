
#ifndef GO_KOMI_H
#define GO_KOMI_H

#include <string>
#include "platform/SgException.h"

class GoKomi {
 public:
  class InvalidKomi : public SgException {
   public:
    explicit InvalidKomi(float komi);
    explicit InvalidKomi(const std::string& komi);
  };
  GoKomi();
  explicit GoKomi(float komi);
  explicit GoKomi(const std::string& komi);
  GoKomi& operator=(const GoKomi& komi) = default;
  bool operator==(const GoKomi& komi) const;
  bool operator!=(const GoKomi& komi) const;
  bool IsUnknown() const;
  float ToFloat() const;
  std::string ToString() const;

 private:
  bool unknown;
  int value;
};

inline std::ostream& operator<<(std::ostream& out, const GoKomi& komi) {
  out << komi.ToString();
  return out;
}

inline GoKomi::GoKomi()
    : unknown(true),
      value(0) {}

inline GoKomi::GoKomi(float komi)
    : unknown(false),
      value(static_cast<int>(komi > 0 ? komi * 2.f + 0.25f : komi * 2.f - 0.25f)) {}

inline bool GoKomi::operator==(const GoKomi& komi) const {
  return (unknown == komi.unknown && value == komi.value);
}

inline bool GoKomi::operator!=(const GoKomi& komi) const {
  return !(*this == komi);
}

inline bool GoKomi::IsUnknown() const {
  return unknown;
}

inline float GoKomi::ToFloat() const {
  if (unknown)
    return value;
  else
    return 0.5f * float(value);
}

#endif

