
#include "platform/SgSystem.h"
#include "platform/SgException.h"

using namespace std;

SgException::SgException() {}

SgException::SgException(const string &message)
    : m_message(message) {}

SgException::SgException(const boost::format &message)
    : m_message(message.str()) {}

SgException::~SgException() throw() {}

const char *SgException::what() const throw() {
  return m_message.c_str();
}

