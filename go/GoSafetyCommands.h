

#ifndef GO_SAFETYCOMMANDS_H
#define GO_SAFETYCOMMANDS_H

#include <string>
#include "GtpEngine.h"
#include "board/GoBWSet.h"

class GoBoard;
class GoSafetyCommands {
 public:
  explicit GoSafetyCommands(const GoBoard& bd);
  void AddGoGuiAnalyzeCommands(GtpCommand& cmd);
  void CmdDameStatic(GtpCommand& cmd);
  void CmdGfx(GtpCommand& cmd);
  void CmdSafe(GtpCommand& cmd);
  void CmdWinner(GtpCommand& cmd);
  void Register(GtpEngine& engine);

 private:
  const GoBoard& m_bd;
  GoBWSet GetSafe(int& totalRegions, const std::string& type);
  void Register(GtpEngine& e, const std::string& command,
                GtpCallback<GoSafetyCommands>::Method method);
};

#endif
