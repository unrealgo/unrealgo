
#ifndef SG_PROCESS_H
#define SG_PROCESS_H

#include <iosfwd>
#include <string>

#if defined(__GNUC__) && !defined(__clang__)
#include <ext/stdio_filebuf.h> // GCC specific
#include <memory>

class SgProcess {
 public:
  SgProcess(const std::string &command);
  ~SgProcess();
  std::istream &Input();
  std::ostream &Output();

 private:
  std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > m_bufIn;
  std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > m_bufOut;
  std::unique_ptr<std::istream> m_in;
  std::unique_ptr<std::ostream> m_out;
};

inline std::istream &SgProcess::Input() {
  return *m_in;
}

inline std::ostream &SgProcess::Output() {
  return *m_out;
}

#else
class SgProcess
{
public:
    SgProcess(const std::string& command);
    ~SgProcess();
};
#endif // defined(__GNUC__) && ! defined(__clang__)

#endif // SG_PROCESS_H
