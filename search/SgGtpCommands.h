
#ifndef SG_GTPCOMMANDS_H
#define SG_GTPCOMMANDS_H

#include "GtpEngine.h"

class SgGtpCommands {
 public:
  explicit SgGtpCommands(GtpEngine &engine, const char *programPath = 0);
  virtual ~SgGtpCommands();

  virtual void CmdCompareFloat(GtpCommand &);
  virtual void CmdCompareInt(GtpCommand &);
  virtual void CmdCpuTime(GtpCommand &);
  virtual void CmdCpuTimeReset(GtpCommand &);
  virtual void CmdDebugger(GtpCommand &);
  virtual void CmdEcho(GtpCommand &);
  virtual void CmdEchoErr(GtpCommand &);
  virtual void CmdExec(GtpCommand &);
  virtual void CmdGetRandomSeed(GtpCommand &);
  virtual void CmdParam(GtpCommand &);
  virtual void CmdPid(GtpCommand &);
  virtual void CmdSetRandomSeed(GtpCommand &);
  virtual void CmdQuiet(GtpCommand &);

  void AddGoGuiAnalyzeCommands(GtpCommand &cmd);
  void Register(GtpEngine &engine);

 private:
  const char *m_programPath;
  GtpEngine &m_engine;
  std::map<std::string, double> m_cpuTimes;
};

#endif // SG_GTPCOMMANDS_H
