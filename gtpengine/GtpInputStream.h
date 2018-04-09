
#ifndef GTP_INPUTSTREAM_H
#define GTP_INPUTSTREAM_H

#include <iostream>
#include <string>


class GtpInputStream {
public:
  explicit GtpInputStream(std::istream &in);

  virtual ~GtpInputStream();

  virtual bool EndOfInput();

  virtual bool GetLine(std::string &line);

private:
  std::istream &m_in;
};

#endif
