
#ifndef GTP_OUTPUTSTREAM_H
#define GTP_OUTPUTSTREAM_H

#include <iostream>
#include <string>

class GtpOutputStream {
public:
  explicit GtpOutputStream(std::ostream &out);
  virtual ~GtpOutputStream();

  virtual void Write(const std::string &line);
  virtual void Flush();

private:
  std::ostream &m_out;
};

#endif

