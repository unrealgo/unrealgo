#ifndef MAIN_ENGINE_H
#define MAIN_ENGINE_H

#include "GoGtpEngine.h"
#include "GoSafetyCommands.h"
#include "GoUctCommands.h"

class MainEngine : public GoGtpEngine {
public:
  explicit MainEngine(int fixedBoardSize, const char *programPath = nullptr,
                           bool noHandicap = false);
  ~MainEngine() = default;
  void CmdAnalyzeCommands(GtpCommand &cmd) final;
  void CmdLicense(GtpCommand &cmd);
  void CmdName(GtpCommand &cmd) final;
  void CmdVersion(GtpCommand &cmd) final;

private:
  GoUctCommands m_uctCommands;
  GoSafetyCommands m_safetyCommands;
};

#endif