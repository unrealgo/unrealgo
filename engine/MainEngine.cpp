
#include "MainEngine.h"
#include "GoGtpCommandUtil.h"
#include "Version.h"

MainEngine::MainEngine(int fixedBoardSize, const char *programPath, bool noHandicap)
        : GoGtpEngine(fixedBoardSize, programPath, false, noHandicap),
          m_uctCommands(m_playerList[m_playerType], Game(),
                        GetDefaultRules()),
          m_safetyCommands(Board()) {
  m_uctCommands.Register(*this);
  m_safetyCommands.Register(*this);
  Register(APP_NAME "-license", &MainEngine::CmdLicense, this);
  Register("license", &MainEngine::CmdLicense, this);

  SetPlayer(new GoUctPlayerType(Board()));
  SetPlayer(new UctDeepPlayer(const_cast<GoGame &>(Game()), GetDefaultRules()), PT_DeepUctPlayer);
  SetPlayerType(PT_DeepUctPlayer);
  InitStatistics();
}

void MainEngine::CmdAnalyzeCommands(GtpCommand &cmd) {
  GoGtpEngine::CmdAnalyzeCommands(cmd);
  m_uctCommands.AddGoGuiAnalyzeCommands(cmd);
  m_safetyCommands.AddGoGuiAnalyzeCommands(cmd);
  cmd << "string/" APP_NAME " License/" APP_NAME "-license\n";
  std::string response = cmd.Response();
  cmd.SetResponse(GoGtpCommandUtil::SortResponseAnalyzeCommands(response));
}

void MainEngine::CmdLicense(GtpCommand &cmd) {
  cmd << UnrealGo::License("\n");
}

void MainEngine::CmdName(GtpCommand &cmd) {
  cmd << APP_NAME;
}

void MainEngine::CmdVersion(GtpCommand &cmd) {
  cmd << UnrealGo::GetVersion();
}
