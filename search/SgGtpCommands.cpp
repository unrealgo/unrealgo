
#include "platform/SgSystem.h"
#include "SgGtpCommands.h"
#include <iomanip>

#if !WIN32
#include <unistd.h>
#endif

#include "platform/SgDebug.h"
#include "platform/SgTime.h"
#include "lib/SgRandom.h"

using namespace std;

namespace {
string ParseCpuTimeId(const GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(1);
  if (cmd.NuArg() > 0)
    return cmd.Arg(0);
  return "total";
}

SgTimeMode TimeModeArg(const GtpCommand &cmd, size_t number) {
  string arg = cmd.ArgToLower(number);
  if (arg == "cpu")
    return SG_TIME_CPU;
  if (arg == "real")
    return SG_TIME_REAL;
  throw GtpFailure() << "unknown time mode argument \"" << arg << '"';
}

string TimeModeToString(SgTimeMode mode) {
  switch (mode) {
    case SG_TIME_CPU:return "cpu";
    case SG_TIME_REAL:return "real";
    default:DBG_ASSERT(false);
      return "?";
  }
}
}


SgGtpCommands::SgGtpCommands(GtpEngine &engine, const char *programPath)
    : m_programPath(programPath),
      m_engine(engine) {}

SgGtpCommands::~SgGtpCommands() {}

void SgGtpCommands::AddGoGuiAnalyzeCommands(GtpCommand &cmd) {
  cmd << "param/search Param/sg_param\n";
}


void SgGtpCommands::CmdCompareFloat(GtpCommand &cmd) {
  float value = cmd.ArgT<float>(0);
  string response = m_engine.ExecuteCommand(cmd.RemainingLine(0));
  istringstream in(response);
  double responseValue;
  in >> responseValue;
  if (!in)
    throw GtpFailure() << "response '" << response << "' is not a float";
  cmd << (responseValue < value ? "-1" : "1");
}


void SgGtpCommands::CmdCompareInt(GtpCommand &cmd) {
  int value = cmd.ArgT<int>(0);
  string response = m_engine.ExecuteCommand(cmd.RemainingLine(0));
  istringstream in(response);
  int responseValue;
  in >> responseValue;
  if (!in)
    throw GtpFailure() << "response '" << response
                       << "' is not an integer";
  if (responseValue == value)
    cmd << "0";
  else if (responseValue < value)
    cmd << "-1";
  else
    cmd << "1";
}


void SgGtpCommands::CmdCpuTime(GtpCommand &cmd) {
  string id = ParseCpuTimeId(cmd);
  double timeNow = SgTime::Get(SG_TIME_CPU);
  double timeDiff = timeNow;
  if (m_cpuTimes.find(id) == m_cpuTimes.end()) {
    if (id != "total")
      throw GtpFailure() << "unknown cputime id " << id;
  } else
    timeDiff -= m_cpuTimes[id];
  cmd << fixed << setprecision(3) << timeDiff;
}


void SgGtpCommands::CmdCpuTimeReset(GtpCommand &cmd) {
  string id = ParseCpuTimeId(cmd);
  double timeNow = SgTime::Get(SG_TIME_CPU);
  m_cpuTimes[id] = timeNow;
}


void SgGtpCommands::CmdDebugger(GtpCommand &cmd) {
#if WIN32
  throw GtpFailure("command not implemented on Windows");
#else
  string type = cmd.Arg();
  const char *path = m_programPath;
  if (path == 0)
    throw GtpFailure("location of executable unknown");
  pid_t pid = getpid();
  ostringstream s;
  if (type == "gdb_kde")
    s << "konsole -e gdb " << path << ' ' << pid << " &";
  else if (type == "gdb_gnome")
    s << "gnome-terminal -e 'gdb " << path << ' ' << pid << "' &";
  else
    throw GtpFailure() << "unknown debugger: " << type;
  SgDebug() << "Executing: " << s.str() << '\n';
  int retval = system(s.str().c_str());
  if (retval != 0)
    throw GtpFailure() << "command returned " << retval;
#endif
}


void SgGtpCommands::CmdEcho(GtpCommand &cmd) {
  cmd << cmd.ArgLine();
}


void SgGtpCommands::CmdEchoErr(GtpCommand &cmd) {
  string line = cmd.ArgLine();
  cerr << line << endl;
  cmd << line;
}


void SgGtpCommands::CmdExec(GtpCommand &cmd) {
  m_engine.ExecuteFile(cmd.Arg(), SgDebug());
}

void SgGtpCommands::CmdGetRandomSeed(GtpCommand &cmd) {
  cmd.CheckArgNone();
  cmd << SgRandom::Seed();
}


void SgGtpCommands::CmdParam(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  if (cmd.NuArg() == 0) {
    cmd << "[list/cpu/real] time_mode "
        << TimeModeToString(SgTime::DefaultMode()) << '\n';
  } else if (cmd.NuArg() >= 1 && cmd.NuArg() <= 2) {
    string name = cmd.Arg(0);
    if (name == "time_mode")
      SgTime::SetDefaultMode(TimeModeArg(cmd, 1));
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void SgGtpCommands::CmdPid(GtpCommand &cmd) {
#if WIN32
  throw GtpFailure("command not implemented on Windows");
#else
  cmd.CheckArgNone();
  cmd << getpid();
#endif
}

void SgGtpCommands::CmdSetRandomSeed(GtpCommand &cmd) {
  SgRandom::SetSeed(cmd.Arg<int>());
}

void SgGtpCommands::CmdQuiet(GtpCommand &cmd) {
  if (cmd.Arg<bool>())
    SgDebugToNull();
  else
    SgSwapDebugStr(&cerr);
}

void SgGtpCommands::Register(GtpEngine &engine) {
  engine.Register("cputime", &SgGtpCommands::CmdCpuTime, this);
  engine.Register("cputime_reset", &SgGtpCommands::CmdCpuTimeReset, this);
  engine.Register("echo", &SgGtpCommands::CmdEcho, this);
  engine.Register("echo_err", &SgGtpCommands::CmdEchoErr, this);
  engine.Register("get_random_seed", &SgGtpCommands::CmdGetRandomSeed, this);
  engine.Register("pid", &SgGtpCommands::CmdPid, this);
  engine.Register("set_random_seed", &SgGtpCommands::CmdSetRandomSeed, this);
  engine.Register("sg_debugger", &SgGtpCommands::CmdDebugger, this);
  engine.Register("sg_compare_float", &SgGtpCommands::CmdCompareFloat, this);
  engine.Register("sg_compare_int", &SgGtpCommands::CmdCompareInt, this);
  engine.Register("sg_exec", &SgGtpCommands::CmdExec, this);
  engine.Register("sg_param", &SgGtpCommands::CmdParam, this);
  engine.Register("quiet", &SgGtpCommands::CmdQuiet, this);
}

