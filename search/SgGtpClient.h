
#ifndef SG_GTPCLIENT_H
#define SG_GTPCLIENT_H

#include <iostream>
#include <string>
#include "platform/SgException.h"


class SgGtpFailure
    : public SgException {
 public:
  explicit SgGtpFailure(const std::string &message);
};



class SgGtpClient {
 public:
  SgGtpClient(std::istream &in, std::ostream &out, bool verbose = false);
  virtual ~SgGtpClient();
  std::string Send(const std::string &command);

 private:
  bool m_verbose;
  std::istream &m_in;
  std::ostream &m_out;
};

#endif // SG_GTPCLIENT_H
