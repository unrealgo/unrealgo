
#ifndef GOUCT_COMMANDS_H
#define GOUCT_COMMANDS_H

#include <string>
#include <UctDeepTrainer.h>
#include <UctEvalStatServer.h>
#include "GtpEngine.h"
#include "GoUctPlayoutPolicy.h"
#include "GoUctGlobalSearch.h"
#include "GoUctPlayer.h"

class GoBoard;
class GoGame;
class GoPlayer;
class GoUctBoard;
class GoUctSearch;
typedef GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>, GoUctPlayoutPolicyFactory<GoUctBoard> >
    GoUctGlobalSearchWithUctBoardType;
typedef GoUctPlayer<GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>,
                                      GoUctPlayoutPolicyFactory<GoUctBoard> >,
                    GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > > GoUctPlayerType;
typedef UctValueType (*MeanMapperFunction)(UctValueType);
enum GoUctCompareMoveType {

  GOUCT_COMPAREMOVE_CORRECTED,

  GOUCT_COMPAREMOVE_POLICY
};
class GoUctCommands {
 public:

  GoUctCommands(GoPlayer *&player, const GoGame &game, GoRules &rules);
  void AddGoGuiAnalyzeCommands(GtpCommand &cmd);
  void CmdAdditiveKnowledge(GtpCommand &cmd);
  void CmdApproximateTerritory(GtpCommand &cmd);
  void CmdBounds(GtpCommand &cmd);
  void CmdDeterministicMode(GtpCommand &);
  void CmdEstimatorStat(GtpCommand &cmd);
  void CmdFinalScore(GtpCommand &);
  void CmdFinalStatusList(GtpCommand &);
  void CmdGfx(GtpCommand &cmd);
  void CmdIsPolicyCorrectedMove(GtpCommand &cmd);
  void CmdIsPolicyMove(GtpCommand &cmd);
  void CmdMaxMemory(GtpCommand &cmd);
  void CmdMoves(GtpCommand &cmd);
  void CmdNodeInfo(GtpCommand &cmd);
  void CmdParamGlobalSearch(GtpCommand &cmd);
  void CmdParamPolicy(GtpCommand &cmd);
  void CmdParamPlayer(GtpCommand &cmd);
  void CmdParamDeepPlayer(GtpCommand &cmd);
  void CmdParamRootFilter(GtpCommand &cmd);
  void CmdParamSearch(GtpCommand &cmd);
  void CmdParamTreeFilter(GtpCommand &cmd);
  void CmdPolicyCorrectedMoves(GtpCommand &cmd);
  void CmdPolicyMoves(GtpCommand &cmd);
  void CmdPolicyMovesSimple(GtpCommand &cmd);
  void CmdPriorKnowledge(GtpCommand &cmd);
  void CmdRaveValues(GtpCommand &cmd);
  void CmdRootFilter(GtpCommand &cmd);
  void CmdSaveGames(GtpCommand &cmd);
  void CmdSaveTree(GtpCommand &cmd);
  void CmdScore(GtpCommand &cmd);
  void CmdSequence(GtpCommand &cmd);
  void CmdStatPlayer(GtpCommand &cmd);
  void CmdStatPlayerClear(GtpCommand &cmd);
  void CmdStatPolicy(GtpCommand &cmd);
  void CmdStatPolicyClear(GtpCommand &cmd);
  void CmdStatSearch(GtpCommand &cmd);
  void CmdStatTerritory(GtpCommand &cmd);
  void CmdValue(GtpCommand &cmd);
  void CmdValueBlack(GtpCommand &cmd);
  void CmdStartTrainPipeline(GtpCommand &cmd);
  void CmdStartEvalCheckPoint(GtpCommand &cmd);
  void CmdStartEvalNoIPC(GtpCommand &cmd);
  void CmdStartEvalFromClient(GtpCommand &cmd);
  void CmdStartSelfPlayAsClient(GtpCommand &cmd);
  void CmdStartSelfPlayAsServer(GtpCommand &cmd);
  void CmdStartStatServer(GtpCommand &cmd);
  void CmdInitCheckPoint(GtpCommand &cmd);
  void Register(GtpEngine &engine);
  UctDeepTrainer *GetDeepTrainer();
  UctEvalStatServer *GetEvalStatSvr();

 private:
  const GoBoard &m_bd;
  GoPlayer *&m_player;
  UctDeepTrainer m_trainer;
  UctEvalStatServer m_statSvr;
  const GoGame &m_game;
  void CompareMove(GtpCommand &cmd, GoUctCompareMoveType type);
  void DisplayKnowledge(GtpCommand &cmd, bool additiveKnowledge);
  void DisplayMoveInfo(GtpCommand &cmd,
                       const std::vector<UctMoveInfo> &moves);
  UctValueType DisplayTerritory(GtpCommand &cmd, MeanMapperFunction f);
  GoPointSet DoFinalStatusSearch();
  inline const GoGame &Game();
  GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>,
                    GoUctPlayoutPolicyFactory<GoUctBoard> > &GlobalSearch();
  GoUctPlayerType &Player();
  GoUctPlayoutPolicy<GoUctBoard> &Policy(unsigned int threadId);
  void Register(GtpEngine &e, const std::string &command,
                GtpCallback<GoUctCommands>::Method method);
  GoUctSearch &Search();
  GoUctSearch &Search(GoPlayer &player);
  GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > &
  ThreadState(unsigned int threadId);
  void WritePolicyMoves(GtpCommand &cmd, bool writeGammas);
};

inline const GoGame &GoUctCommands::Game() {
  return m_game;
}

#endif
