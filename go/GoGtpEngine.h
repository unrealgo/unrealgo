

#ifndef GO_GTPENGINE_H
#define GO_GTPENGINE_H

#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>
#include "search/UctDeepTrainer.h"
#include "GoBoard.h"
#include "GoGame.h"
#include "GoPlayer.h"
#include "GoStaticLadder.h"
#include "SgTimeSettings.h"
#include "GtpEngine.h"
#include "SgGtpCommands.h"
#include "board/GoPointArray.h"
#include "MpiSynchronizer.h"

enum PlayerType {
  PT_UctPlayer,
  PT_DeepUctPlayer,
  PT_NUM
};
class GoGtpEngine
    : public GtpEngine {
 public:
  virtual void CmdAllLegal(GtpCommand&);
  virtual void CmdAllMoveValues(GtpCommand&);
  virtual void CmdAnalyzeCommands(GtpCommand&);
  virtual void CmdBoard(GtpCommand&);
  virtual void CmdSetBoardSize(GtpCommand&);
  virtual void CmdGetBoardSize(GtpCommand&);
  virtual void CmdClearBoard(GtpCommand&);
  virtual void CmdClock(GtpCommand&);
  virtual void CmdDistance(GtpCommand& cmd);
  virtual void CmdFinalScore(GtpCommand&);
  virtual void CmdFixedHandicap(GtpCommand&);
  virtual void CmdGameOver(GtpCommand&);
  virtual void CmdGenMove(GtpCommand&);
  virtual void CmdGenUctMove(GtpCommand&);
  virtual void CmdGenMoveCleanup(GtpCommand&);
  virtual void CmdGetKomi(GtpCommand&);
  virtual void CmdGGUndo(GtpCommand&);
  virtual void CmdInterrupt(GtpCommand&);
  virtual void CmdIsLegal(GtpCommand&);
  virtual void CmdKgsTimeSettings(GtpCommand& cmd);
  virtual void CmdKomi(GtpCommand&);
  virtual void CmdListStones(GtpCommand&);
  virtual void CmdLoadSgf(GtpCommand&);
  virtual void CmdName(GtpCommand&);
  virtual void CmdParam(GtpCommand&);
  virtual void CmdParamRules(GtpCommand&);
  virtual void CmdParamTimecontrol(GtpCommand&);
  virtual void CmdPlaceFreeHandicap(GtpCommand&);
  virtual void CmdPlay(GtpCommand&);
  virtual void CmdPlayAgainst(GtpCommand&);
  virtual void CmdPlayerBoard(GtpCommand&);
  virtual void CmdPlaySequence(GtpCommand&);
  virtual void CmdPointNumbers(GtpCommand&);
  virtual void CmdPointInfo(GtpCommand&);
  virtual void CmdQuit(GtpCommand& cmd);
  virtual void CmdRegGenMove(GtpCommand&);
  virtual void CmdRegGenMoveToPlay(GtpCommand&);
  virtual void CmdRules(GtpCommand&);
  virtual void CmdSaveSgf(GtpCommand&);
  virtual void CmdSentinelFile(GtpCommand&);
  virtual void CmdSetFreeHandicap(GtpCommand&);
  virtual void CmdSetInfo(GtpCommand&);
  virtual void CmdSetup(GtpCommand&);
  virtual void CmdSetupPlayer(GtpCommand&);
  virtual void CmdShowBoard(GtpCommand&);
  virtual void CmdStaticScore(GtpCommand&);
  virtual void CmdTimeLastMove(GtpCommand&);
  virtual void CmdTimeLeft(GtpCommand&);
  virtual void CmdTimeSettings(GtpCommand&);
  virtual void CmdUndo(GtpCommand&);
  explicit GoGtpEngine(int fixedBoardSize = 0, const char* programPath = 0,
                       bool noPlayer = false, bool noHandicap = false);
  ~GoGtpEngine();
  const GoGame& Game() const;
  const GoBoard& Board() const;
  void ShowBoard(GtpCommand& cmd) const;
  void DebugBoard() const;
  void SetPlayer(GoPlayer* player, PlayerType type = PT_UctPlayer);
  void SetPlayerType(PlayerType type = PT_UctPlayer);
  GoPlayer& Player() const;
  void DumpState(std::ostream& out) const;
  void SetAutoSave(const std::string& prefix);
  void SetStatisticsFile(const std::string& fileName);
  void SetAutoShowBoard(bool showBoard);
  void SetDebugToComment(bool debugToComment);
  void SetMaxClearBoard(int n);
  void SetNamedRules(const std::string& namedRules);
  void SetTimeLimit(double timeLimit);
  double TimeLimit();

#if GTPENGINE_PONDER

  void Ponder();
  void StopPonder();
  void InitPonder();

#endif

#if GTPENGINE_INTERRUPT

  void Interrupt();

#endif

  void SetMpiSynchronizer(const MpiSynchronizerHandle& m_handle);
  MpiSynchronizerHandle MpiSynchronizer();
  const MpiSynchronizerHandle MpiSynchronizer() const;

 protected:
  PlayerType m_playerType;
  GoPlayer* m_playerList[PT_NUM];
  void BeforeHandleCommand();
  void BeforeWritingResponse();
  void BoardChanged();
  void CheckLegal(std::string message, SgBlackWhite color, GoPoint move,
                  bool checkOnlyOccupied);
  void CheckMaxClearBoard();
  void CheckMoveStackOverflow() const;
  void OnGenMove(GtpCommand& cmd, SgBlackWhite color);
  GoPoint GenMove(SgBlackWhite color, bool ignoreClock);
  GoPoint GenUctMove(SgBlackWhite color, bool ignoreClock);
  GoPoint GenDeepUctMove(SgBlackWhite color, bool ignoreClock);
  void RespondNumberArray(GtpCommand& cmd, const GoPointArray<int>& array,
                          int scale);
  void Init(int size);
  void Play(SgBlackWhite color, GoPoint move);
  virtual std::vector<std::string> CreateStatisticsSlots();
  virtual void AddPlayStatistics();
  void AddStatistics(const std::string& key, const std::string& value);
  template<typename T>
  void AddStatistics(const std::string& key, const T& value);
  SgBlackWhite BlackWhiteArg(const GtpCommand& cmd,
                             std::size_t number) const;
  SgEmptyBlackWhite EmptyBlackWhiteArg(const GtpCommand& cmd,
                                       std::size_t number) const;
  GoPoint EmptyPointArg(const GtpCommand& cmd, std::size_t number) const;
  GoPoint MoveArg(const GtpCommand& cmd, std::size_t number) const;
  GoPoint PointArg(const GtpCommand& cmd) const;
  GoPoint PointArg(const GtpCommand& cmd, std::size_t number) const;
  SgVector<GoPoint> PointListArg(const GtpCommand& cmd,
                                 std::size_t number) const;
  SgVector<GoPoint> PointListArg(const GtpCommand& cmd) const;
  GoPoint StoneArg(const GtpCommand& cmd, std::size_t number) const;
  void RulesChanged();
  GoRules& GetDefaultRules();
  void InitStatistics();

 private:
  bool m_noPlayer;
  bool m_acceptIllegal;
  bool m_autoSave;
  bool m_autoShowBoard;
  bool m_debugToComment;
  bool m_useBook;
  bool m_isPonderPosition;
  int m_fixedBoardSize;
  int m_maxClearBoard;
  int m_numberClearBoard;
  double m_timeLastMove;
  double m_timeLimit;
  double m_overhead;
  SgTimeSettings m_timeSettings;
  GoRules m_defaultRules;
  GoGame m_game;
  SgGtpCommands m_sgCommands;
  std::string m_autoSaveFileName;
  std::string m_autoSavePrefix;
  boost::filesystem::path m_sentinelFile;
  std::string m_statisticsFile;
  std::vector<std::string> m_statisticsSlots;
  std::vector<std::string> m_statisticsValues;
  MpiSynchronizerHandle m_mpiSynchronizer;
  void ApplyTimeSettings();
  void AutoSave() const;
  void CheckBoardEmpty() const;
  void CreateAutoSaveFileName();
  void GameFinished();
  void PlaceHandicap(const SgVector<GoPoint>& stones);
  void SaveGame(const std::string& fileName) const;
  void SaveStatistics();
  void StartStatistics();
  void Undo(int n);
  static void WriteBoardInfo(GtpCommand& cmd, const GoBoard& bd);
};

template<typename T>
void GoGtpEngine::AddStatistics(const std::string& key, const T& value) {
  std::ostringstream s;
  s << value;
  AddStatistics(key, s.str());
}

inline const GoBoard& GoGtpEngine::Board() const {
  return m_game.Board();
}

inline const GoGame& GoGtpEngine::Game() const {
  return m_game;
}

inline void GoGtpEngine::SetDebugToComment(bool debugToComment) {
  m_debugToComment = debugToComment;
}

inline void GoGtpEngine::SetMaxClearBoard(int n) {
  m_maxClearBoard = n;
}

inline void GoGtpEngine::SetTimeLimit(double timeLimit) {
  m_timeLimit = timeLimit;
}

inline double GoGtpEngine::TimeLimit() {
  return m_timeLimit;
}

class GoGtpAssertionHandler
    : public AssertionHandlerInterface {
 public:
  GoGtpAssertionHandler(const GoGtpEngine& engine);
  void Run();

 private:
  const GoGtpEngine& m_engine;
};

#endif
