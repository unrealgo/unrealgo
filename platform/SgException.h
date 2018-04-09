
#ifndef SG_EXCEPTION_H
#define SG_EXCEPTION_H

#include <exception>
#include <string>
#include <boost/format.hpp>

class SgException
    : public std::exception {
 public:
  SgException();
  explicit SgException(const std::string& message);
  explicit SgException(const boost::format& f);
  virtual ~SgException() throw();
  const char* what() const throw() final;

 private:
  std::string m_message;
};

#endif // SG_EXCEPTION_H
