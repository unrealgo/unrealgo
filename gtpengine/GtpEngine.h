
#ifndef GTPENGINE_H
#define GTPENGINE_H

#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <typeinfo>

#ifdef __GNUC__
#include <cstdlib>
#include <cxxabi.h>
#endif

#include "GtpInputStream.h"
#include "GtpOutputStream.h"

#ifndef GTPENGINE_PONDER
#define GTPENGINE_PONDER 1
#endif

#ifndef GTPENGINE_INTERRUPT
#define GTPENGINE_INTERRUPT 1
#endif

class GtpFailure {
public:
  GtpFailure();
  explicit GtpFailure(const std::string &response);
  GtpFailure(const GtpFailure &failure);
  ~GtpFailure() = default;

  std::string Response() const;
  std::ostream &ResponseStream();

private:
  std::ostringstream m_response;
};

template<typename TYPE>
GtpFailure operator<<(const GtpFailure &failure, const TYPE &type) {
  GtpFailure result(failure);
  result.ResponseStream() << type;
  return result;
}

template<typename TYPE>
GtpFailure operator<<(const GtpFailure &failure, TYPE &type) {
  GtpFailure result(failure);
  result.ResponseStream() << type;
  return result;
}

inline std::string GtpFailure::Response() const {
  return m_response.str();
}

inline std::ostream &GtpFailure::ResponseStream() {
  return m_response;
}


class GtpCommand {
public:
  GtpCommand();

  explicit GtpCommand(const std::string &line);

  const std::string &Arg(std::size_t number) const;
  const std::string &Arg() const;
  std::string ArgToLower(std::size_t number) const;
  template<typename T>
  T ArgT(std::size_t i) const;
  template<typename T>
  T Arg() const;

  template<typename T>
  T ArgMin(std::size_t i, const T &min) const;
  template<typename T>
  T ArgMinMax(std::size_t i, const T &min, const T &max) const;

  void CheckArgNone() const;
  void CheckNuArg(std::size_t number) const;
  void CheckNuArgLessEqual(std::size_t number) const;

  std::string ID() const;

  void Init(const std::string &line);
  std::string ArgLine() const;
  const std::string &Line() const;
  const std::string &Name() const;

  std::size_t NuArg() const;
  std::string RemainingLine(std::size_t number) const;

  std::string Response() const;
  std::ostringstream &ResponseStream();
  void SetResponse(const std::string &response);
  void SetResponseBool(bool value);

private:
  struct Argument {
    std::string m_value;
    std::size_t m_end;
    Argument(const std::string &value, std::size_t end);
  };

  static std::ostringstream s_dummy;
  std::string m_id;
  std::string m_line;
  std::ostringstream m_response;
  std::vector<Argument> m_arguments;

  template<typename T>
  static std::string TypeName();

  void ParseCommandId();

  void SplitLine(const std::string &line);
};

template<typename TYPE>
GtpCommand &operator<<(GtpCommand &cmd, const TYPE &type) {
  cmd.ResponseStream() << type;
  return cmd;
}

template<typename TYPE>
GtpCommand &operator<<(GtpCommand &cmd, TYPE &type) {
  cmd.ResponseStream() << type;
  return cmd;
}

inline GtpCommand::GtpCommand() {}

inline GtpCommand::GtpCommand(const std::string &line) {
  Init(line);
}

template<typename T>
T GtpCommand::ArgT(std::size_t i) const {
  std::string s = Arg(i);
  std::istringstream in(s);
  T result;
  in >> result;
  if (!in)
    throw GtpFailure() << "argument " << (i + 1) << " (" << s << ") must be of type " << TypeName<T>();
  return result;
}

template<typename T>
inline T GtpCommand::Arg() const {
  CheckNuArg(1);
  return ArgT<T>(0);
}

template<typename T>
T GtpCommand::ArgMin(std::size_t i, const T &min) const {
  T result = ArgT<T>(i);
  if (result < min)
    throw GtpFailure() << "argument " << (i + 1) << " (" << result
                       << ") must be greater or equal " << min;
  return result;
}

template<typename T>
T GtpCommand::ArgMinMax(std::size_t i, const T &min, const T &max) const {
  T result = ArgMin(i, min);
  if (result > max)
    throw GtpFailure() << "argument " << (i + 1) << " (" << result
                       << ") must be less or equal " << max;
  return result;
}

inline void GtpCommand::CheckArgNone() const {
  CheckNuArg(0);
}

inline std::string GtpCommand::ID() const {
  return m_id;
}

inline const std::string &GtpCommand::Line() const {
  return m_line;
}

inline const std::string &GtpCommand::Name() const {
  return m_arguments[0].m_value;
}

inline std::size_t GtpCommand::NuArg() const {
  return m_arguments.size() - 1;
}

inline std::string GtpCommand::Response() const {
  return m_response.str();
}

inline std::ostringstream &GtpCommand::ResponseStream() {
  return m_response;
}

template<typename T>
std::string GtpCommand::TypeName() {
#ifdef __GNUC__
  int status;
  char *namePtr = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
  if (status == 0) {
    std::string result(namePtr);
    std::free(namePtr);
    return result;
  } else
    return typeid(T).name();
#else
  return typeid(T).name();
#endif
}

class GtpCallbackBase {
public:
  virtual ~GtpCallbackBase() throw();

  virtual void operator()(GtpCommand &) = 0;
};

template<class ENGINE>
class GtpCallback
        : public GtpCallbackBase {
public:
  typedef void (ENGINE::*Method)(GtpCommand &);

  GtpCallback(ENGINE *instance,
              typename GtpCallback<ENGINE>::Method method);

  ~GtpCallback() throw();

  void operator()(GtpCommand &) final;

private:
  ENGINE *m_instance;
  Method m_method;
};

template<class ENGINE>
GtpCallback<ENGINE>::GtpCallback(ENGINE *instance,
                                 typename GtpCallback<ENGINE>::Method method)
        : m_instance(instance),
          m_method(method) {}

template<class ENGINE>
GtpCallback<ENGINE>::~GtpCallback() throw() {
#ifndef NDEBUG
  m_instance = 0;
#endif
}

template<class ENGINE>
void GtpCallback<ENGINE>::operator()(GtpCommand &cmd) {
  (m_instance->*m_method)(cmd);
}

class GtpEngine {
public:
  virtual void CmdKnownCommand(GtpCommand &);
  virtual void CmdListCommands(GtpCommand &);
  virtual void CmdName(GtpCommand &);
  virtual void CmdProtocolVersion(GtpCommand &);
  virtual void CmdQuit(GtpCommand &);
  virtual void CmdVersion(GtpCommand &);

  GtpEngine();
  virtual ~GtpEngine();
  void ExecuteFile(const std::string &name, std::ostream &log = std::cerr);
  std::string ExecuteCommand(const std::string &cmd,
                             std::ostream &log = std::cerr);
  void MainLoop(GtpInputStream &in, GtpOutputStream &out);

  void Register(const std::string &name, GtpCallbackBase *callback);

  template<class T>
  void Register(const std::string &command,
                typename GtpCallback<T>::Method method, T *instance);

  bool IsRegistered(const std::string &command) const;
  void SetQuit();

  bool IsQuitSet() const;

#if GTPENGINE_PONDER
  virtual void Ponder();
  virtual void InitPonder();
  virtual void StopPonder();
#endif

#if GTPENGINE_INTERRUPT
  virtual void Interrupt();
#endif

protected:
  virtual void BeforeHandleCommand();
  virtual void BeforeWritingResponse();

private:
  typedef std::map<std::string, GtpCallbackBase *> CallbackMap;
  bool m_quit;
  CallbackMap m_callbacks;

  GtpEngine(const GtpEngine &engine);
  GtpEngine &operator=(const GtpEngine &engine) const;
  bool HandleCommand(GtpCommand &cmd, GtpOutputStream &out);
};

template<class T>
void GtpEngine::Register(const std::string &command,
                         typename GtpCallback<T>::Method method, T *instance) {
  Register(command, new GtpCallback<T>(instance, method));
}

#endif
