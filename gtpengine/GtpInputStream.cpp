
#include "GtpInputStream.h"

GtpInputStream::GtpInputStream(std::istream &in)
        : m_in(in) {
  m_in.tie(0);
}

GtpInputStream::~GtpInputStream() {}

bool GtpInputStream::EndOfInput() {
  return m_in.fail();
}

bool GtpInputStream::GetLine(std::string &line) {
  return !getline(m_in, line).fail();
}
