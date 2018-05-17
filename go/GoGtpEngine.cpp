
#include "platform/SgSystem.h"
#include "GoGtpEngine.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include "GoBoardRestorer.h"
#include "GoGtpCommandUtil.h"
#include "GoInfluence.h"
#include "GoNodeUtil.h"
#include "SgGameReader.h"
#include "SgGameWriter.h"

#if GTPENGINE_PONDER
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#endif

using boost::filesystem::exists;
using boost::filesystem::path;
using boost::filesystem::remove;
using GoPointUtil::Pt;
using std::flush;
using std::string;

namespace {

GoRules::KoRule KoRuleArg(GtpCommand& cmd, size_t number) {
  string arg = cmd.ArgToLower(number);
  if (arg == "simple")
    return GoRules::SIMPLEKO;
  if (arg == "superko")
    return GoRules::SUPERKO;
  if (arg == "pos_superko")
    return GoRules::POS_SUPERKO;
  throw GtpFailure() << "unknown ko rule \"" << arg << '"';
}

string KoRuleToString(GoRules::KoRule rule) {
  switch (rule) {
    case GoRules::SIMPLEKO:return "simple";
    case GoRules::SUPERKO:return "superko";
    case GoRules::POS_SUPERKO:return "pos_superko";
    default:DBG_ASSERT(false);
      return "?";
  }
}

}

GoGtpEngine::GoGtpEngine(int fixedBoardSize, const char* programPath,
                         bool noPlayer, bool noHandicap)
    : m_playerType(PT_UctPlayer),
      m_noPlayer(noPlayer),
      m_acceptIllegal(false),
      m_autoSave(false),
      m_autoShowBoard(false),
      m_debugToComment(false),
      m_useBook(true),
      m_isPonderPosition(true),
      m_fixedBoardSize(fixedBoardSize),
      m_maxClearBoard(-1),
      m_numberClearBoard(0),
      m_timeLastMove(0),
      m_timeLimit(100000000),
      m_overhead(0),
      m_game(fixedBoardSize > 0 ? fixedBoardSize : GO_DEFAULT_SIZE),
      m_sgCommands(*this, programPath),
      m_mpiSynchronizer(NullMpiSynchronizer::Create()) {

  for (auto& player : m_playerList)
    player = nullptr;

  Init(Board().Size());

  Register("all_legal", &GoGtpEngine::CmdAllLegal, this);
  Register("boardsize", &GoGtpEngine::CmdSetBoardSize, this);
  Register("set_size", &GoGtpEngine::CmdSetBoardSize, this);
  Register("get_size", &GoGtpEngine::CmdGetBoardSize, this);
  Register("query_boardsize", &GoGtpEngine::CmdGetBoardSize, this);
  Register("clear_board", &GoGtpEngine::CmdClearBoard, this);
  Register("clear", &GoGtpEngine::CmdClearBoard, this);
  Register("cgos-gameover", &GoGtpEngine::CmdGameOver, this);
  Register("get_komi", &GoGtpEngine::CmdGetKomi, this);
  Register("gg-undo", &GoGtpEngine::CmdGGUndo, this);
  Register("go_board", &GoGtpEngine::CmdBoard, this);
  Register("go_distance", &GoGtpEngine::CmdDistance, this);
  Register("dist", &GoGtpEngine::CmdDistance, this);
  Register("go_param", &GoGtpEngine::CmdParam, this);
  Register("go_param_rules", &GoGtpEngine::CmdParamRules, this);
  Register("go_player_board", &GoGtpEngine::CmdPlayerBoard, this);
  Register("go_point_info", &GoGtpEngine::CmdPointInfo, this);
  Register("go_point_numbers", &GoGtpEngine::CmdPointNumbers, this);
  Register("go_rules", &GoGtpEngine::CmdRules, this);
  Register("go_sentinel_file", &GoGtpEngine::CmdSentinelFile, this);
  Register("go_set_info", &GoGtpEngine::CmdSetInfo, this);
  Register("gogui-analyze_commands", &GoGtpEngine::CmdAnalyzeCommands, this);
  Register("gogui-interrupt", &GoGtpEngine::CmdInterrupt, this);
  Register("gogui-play_sequence", &GoGtpEngine::CmdPlaySequence, this);
  Register("gogui-setup", &GoGtpEngine::CmdSetup, this);
  Register("gogui-setup_player", &GoGtpEngine::CmdSetupPlayer, this);
  Register("is_legal", &GoGtpEngine::CmdIsLegal, this);
  Register("kgs-genmove_cleanup", &GoGtpEngine::CmdGenMoveCleanup, this);
  Register("kgs-time_settings", &GoGtpEngine::CmdKgsTimeSettings, this);
  Register("komi", &GoGtpEngine::CmdKomi, this);
  Register("list_stones", &GoGtpEngine::CmdListStones, this);
  Register("ls", &GoGtpEngine::CmdListStones, this);
  Register("loadsgf", &GoGtpEngine::CmdLoadSgf, this);
  Register("play", &GoGtpEngine::CmdPlay, this);
  Register("p", &GoGtpEngine::CmdPlay, this);
  Register("play-against", &GoGtpEngine::CmdPlayAgainst, this);
  Register("against", &GoGtpEngine::CmdPlayAgainst, this);
  Register("a", &GoGtpEngine::CmdPlayAgainst, this);
  Register("savesgf", &GoGtpEngine::CmdSaveSgf, this);
  Register("showboard", &GoGtpEngine::CmdShowBoard, this);
  Register("s", &GoGtpEngine::CmdShowBoard, this);
  Register("show", &GoGtpEngine::CmdShowBoard, this);
  Register("static_score", &GoGtpEngine::CmdStaticScore, this);
  Register("time_left", &GoGtpEngine::CmdTimeLeft, this);
  Register("time_settings", &GoGtpEngine::CmdTimeSettings, this);
  Register("undo", &GoGtpEngine::CmdUndo, this);
  m_sgCommands.Register(*this);
  if (!m_noPlayer) {
    Register("all_move_values", &GoGtpEngine::CmdAllMoveValues, this);
    Register("final_score", &GoGtpEngine::CmdFinalScore, this);
    Register("genmove", &GoGtpEngine::CmdGenMove, this);
    Register("move", &GoGtpEngine::CmdGenMove, this);
    Register("m", &GoGtpEngine::CmdGenMove, this);
    // Register("genuctmove", &GoGtpEngine::CmdGenUctMove, this);
    // Register("uctgenmove", &GoGtpEngine::CmdGenUctMove, this);
    Register("go_clock", &GoGtpEngine::CmdClock, this);
    Register("go_param_timecontrol", &GoGtpEngine::CmdParamTimecontrol, this);
    Register("reg_genmove", &GoGtpEngine::CmdRegGenMove, this);
    Register("reg_genmove_toplay", &GoGtpEngine::CmdRegGenMoveToPlay, this);
    Register("time_lastmove", &GoGtpEngine::CmdTimeLastMove, this);
  }
  if (!noHandicap) {
    Register("fixed_handicap", &GoGtpEngine::CmdFixedHandicap, this);
    Register("place_free_handicap", &GoGtpEngine::CmdPlaceFreeHandicap, this);
    Register("set_free_handicap", &GoGtpEngine::CmdSetFreeHandicap, this);
  }
}

GoGtpEngine::~GoGtpEngine() {
  for (auto& player : m_playerList)
    delete player;
}

void GoGtpEngine::AddPlayStatistics() {
}

void GoGtpEngine::AddStatistics(const std::string& key,
                                const std::string& value) {
  DBG_ASSERT(m_statisticsValues.size() == m_statisticsSlots.size());
  if (value.find('\t') != string::npos)
    throw SgException("GoGtpEngine::AddStatistics: value contains tab: '"
                          + value + "'");
  for (size_t i = 0; i < m_statisticsSlots.size(); ++i)
    if (m_statisticsSlots[i] == key) {
      m_statisticsValues[i] = value;
      return;
    }
  throw SgException("GoGtpEngine::AddStatistics: invalid key '" + key + "'");
}

void GoGtpEngine::ApplyTimeSettings() {
  DBG_ASSERT(Board().MoveNumber() == 0);
  m_game.SetTimeSettingsGlobal(m_timeSettings, m_overhead);
}

void GoGtpEngine::AutoSave() const {
  if (!m_autoSave)
    return;
  try {
    SaveGame(m_autoSaveFileName);
  }
  catch (const GtpFailure& failure) {
    SgWarning() << failure.Response() << '\n';
  }
}

void GoGtpEngine::BoardChanged() {
  const GoBoard& bd = Board();
  if (m_autoShowBoard)
    SgDebug() << bd;
  if (m_playerList[m_playerType] != 0)
    m_playerList[m_playerType]->UpdateSubscriber();
  AutoSave();
  m_isPonderPosition = (!GoBoardUtil::IsBoardEmpty(bd)
      && !GoBoardUtil::EndOfGame(bd));
#ifdef DEBUG_BOARD
  DebugBoard();
#endif
}

void GoGtpEngine::BeforeHandleCommand() {
  SetForceAbort(false);
  SgDebug() << flush;
}

void GoGtpEngine::BeforeWritingResponse() {
  SgDebug() << flush;
}

void GoGtpEngine::CheckBoardEmpty() const {
  if (!GoBoardUtil::IsBoardEmpty(Board()))
    throw GtpFailure("board is not empty");
}

void GoGtpEngine::CheckLegal(string message, SgBlackWhite color, GoPoint move,
                             bool checkOnlyOccupied) {
  GoModBoard modBoard(Board());
  GoBoard& bd = modBoard.Board();
  bool illegal = false;
  string reason = "";
  if (move != GO_PASS) {
    if (bd.Occupied(move)) {
      illegal = true;
      reason = " (occupied)";
    } else if (!checkOnlyOccupied) {
      bd.Play(move, color);
      GoMoveInfo moveInfo = bd.GetLastMoveInfo();
      bd.Undo();
      if (moveInfo.test(GO_MOVEFLAG_ILLEGAL)) {
        illegal = true;
        if (moveInfo.test(GO_MOVEFLAG_SUICIDE))
          reason = " (suicide)";
        else if (moveInfo.test(GO_MOVEFLAG_REPETITION)) {
          reason =
              " (" + KoRuleToString(bd.Rules().GetKoRule()) + ")";
        }
      }
    }
  }
  if (illegal) {
    int moveNumber = m_game.CurrentMoveNumber() + 1;
    throw GtpFailure() << message << moveNumber << ' ' << SgBW(color)
                       << ' ' << GoWritePoint(move) << reason;
  }
}

void GoGtpEngine::CheckMaxClearBoard() {
  if (m_maxClearBoard >= 0 && m_numberClearBoard > m_maxClearBoard - 1)
    throw GtpFailure() << "maximum number of " << m_maxClearBoard
                       << " reached";
  ++m_numberClearBoard;
}

void GoGtpEngine::CmdAllLegal(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  SgVector<GoPoint> allLegal;
  for (GoBoard::Iterator p(Board()); p; ++p)
    if (Board().IsLegal(*p, color))
      allLegal.PushBack(*p);
  cmd << SgWritePointList(allLegal, "", false);
}

void GoGtpEngine::CmdAllMoveValues(GtpCommand& cmd) {
  cmd.CheckArgNone();
  const GoBoard& bd = Board();
  GoPlayer& player = Player();
  for (GoBoard::Iterator it(bd); it; ++it)
    if (!bd.Occupied(*it)) {
      int value = player.MoveValue(*it);
      if (value > std::numeric_limits<int>::min())
        cmd << GoWritePoint(*it) << ' ' << value << '\n';
    }
}

void GoGtpEngine::CmdAnalyzeCommands(GtpCommand& cmd) {
  cmd.CheckArgNone();
  cmd <<
      "hpstring/Go Board/go_board\n"
          "param/Go Param/go_param\n"
          "param/Go Param Rules/go_param_rules\n"
          "hpstring/Go Point Info/go_point_info %p\n"
          "sboard/Go Point Numbers/go_point_numbers\n"
          "none/Go Rules/go_rules %s\n"
          "plist/All Legal/all_legal %c\n"
          "sboard/Go Distance/go_distance %c\n"
          "string/ShowBoard/showboard\n"
          "string/CpuTime/cputime\n"
          "string/Get Komi/get_komi\n"
          "string/Get Random Seed/get_random_seed\n"
          "string/Static Score Japanese/static_score japanese_score\n"
          "string/Static Score Tromp-Taylor/static_score tromp_taylor_score\n"
          "plist/List Stones/list_stones %c\n"
          "none/Set Random Seed/set_random_seed %s\n"
          "none/SaveSgf/savesgf %w\n";
  m_sgCommands.AddGoGuiAnalyzeCommands(cmd);
  if (!m_noPlayer) {
    cmd <<
        "pspairs/All Move Values/all_move_values\n"
            "string/Final Score/final_score\n"
            "param/Go Param TimeControl/go_param_timecontrol\n"
            "varc/Reg GenMove/reg_genmove %c\n";
  }
}

void GoGtpEngine::CmdBoard(GtpCommand& cmd) {
  WriteBoardInfo(cmd, Board());
}

void GoGtpEngine::CmdSetBoardSize(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  int size = cmd.ArgMinMax<int>(0, GO_MIN_SIZE, GO_MAX_SIZE);
  if (m_fixedBoardSize > 0 && size != m_fixedBoardSize)
    throw GtpFailure() << "Boardsize " << m_fixedBoardSize << " fixed";
  if (Board().MoveNumber() > 0)
    GameFinished();
  Init(size);
}

void GoGtpEngine::CmdGetBoardSize(GtpCommand& cmd) {
  cmd.CheckArgNone();
  cmd << m_game.Board().Size();
}

void GoGtpEngine::CmdClearBoard(GtpCommand& cmd) {
  cmd.CheckArgNone();
  CheckMaxClearBoard();
  if (!m_sentinelFile.empty() && exists(m_sentinelFile)) {
    throw GtpFailure() << "Detected sentinel file '"
                       << m_sentinelFile.string() << "'";
  }
  if (Board().MoveNumber() > 0)
    GameFinished();
  Init(Board().Size());
  if (m_playerList[m_playerType] != 0)
    m_playerList[m_playerType]->OnNewGame();
  BoardChanged();
}

void GoGtpEngine::CmdClock(GtpCommand& cmd) {
  cmd.CheckArgNone();
  m_game.TimeRecord().UpdateTimeLeft();
  cmd << '\n' << m_game.TimeRecord();
}

void GoGtpEngine::CmdDistance(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SgBlackWhite color = GoGtpCommandUtil::BlackWhiteArg(cmd, 0);
  GoPointArray<int> distance;
  GoInfluence::FindDistanceToStones(Board(), color, distance);
  cmd << '\n' << SgWritePointArray<int>(distance, Board().Size());
}

void GoGtpEngine::CmdFinalScore(GtpCommand& cmd) {
  cmd.CheckArgNone();
  const GoBoard& bd = Board();
  if (!bd.Rules().CaptureDead())
    throw GtpFailure("can only score after capturing dead");
  float komi = bd.Rules().Komi().ToFloat();
  float score = GoBoardUtil::Score(bd, komi);
  cmd << GoUtil::ScoreToString(score);
}

void GoGtpEngine::CmdFixedHandicap(GtpCommand& cmd) {
  int n = cmd.ArgMin<int>(0, 2);
  int size = Board().Size();
  SgVector<GoPoint> stones = GoGtpCommandUtil::GetHandicapStones(size, n);
  PlaceHandicap(stones);
}

void GoGtpEngine::CmdGameOver(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  const string& result = cmd.Arg(0);
  m_game.UpdateResult(result);
  m_isPonderPosition = false;
  AutoSave();
}

void GoGtpEngine::Play(SgBlackWhite color, GoPoint move) {
  if (move == UCT_RESIGN) {
    m_isPonderPosition = false;
    return;
  }
  CheckMoveStackOverflow();
  CheckLegal("illegal move: ", color, move, m_acceptIllegal);
  m_game.AddMove(move, color);
  auto& player = (UctDeepPlayer&)Player();
  player.OnOppMove(move, color);
}

void GoGtpEngine::CmdPlay(GtpCommand& cmd) {
  cmd.CheckNuArg(2);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  GoPoint move = MoveArg(cmd, 1);
  Play(color, move);
  BoardChanged();
}

void GoGtpEngine::OnGenMove(GtpCommand& cmd, SgBlackWhite color) {
  std::unique_ptr<SgDebugToString> debugStrToString;
  if (m_debugToComment)
    debugStrToString.reset(new SgDebugToString(true));

  //GoPoint move = GenDeepUctMove(color, true);
  CheckMoveStackOverflow();
  StartStatistics();

  auto& player = (UctDeepPlayer&)Player();
  player.TryInitNeuralNetwork();
  double startTime = SgTime::Get();
  SgTimeRecord timeRecord;
  if (true || m_timeSettings.IsUnknown())
    timeRecord = SgTimeRecord(true, m_timeLimit);
  else {
    timeRecord = m_game.TimeRecord();
    timeRecord.UpdateTimeLeft();
  }
  AddStatistics("GAME", m_autoSaveFileName);
  AddStatistics("MOVE", m_game.CurrentMoveNumber() + 1);

  GoPoint move = GO_NULLMOVE;
  m_mpiSynchronizer->SynchronizeMove(move);
  player.ClearSearchTraces();
  move = player.GenMove(timeRecord, color);
  SgNode* searchTraces = player.TransferSearchTraces();
  if (searchTraces != 0)
    m_game.AppendChild(searchTraces);
  m_mpiSynchronizer->SynchronizeMove(move);

  m_timeLastMove = SgTime::Get() - startTime;
  AddStatistics("TIME", m_timeLastMove);
  if (move == GO_NULLMOVE)
    throw GtpFailure() << player.Name() << " generated NULLMOVE";
  if (move == UCT_RESIGN)
    m_isPonderPosition = false;
  else
    CheckLegal(player.Name() + " generated illegal move: ", color, move, false);
  AddPlayStatistics();
  SaveStatistics();


  if (move == UCT_RESIGN) {
    cmd << "resign";
    const SgNode& node = m_game.AddResignNode(color);
    if (debugStrToString != nullptr) {
      m_game.AddComment(node, "\n\n");
      m_game.AddComment(node, debugStrToString->GetString());
    }
    AutoSave();
  } else {
    m_game.AddMove(move, color);
    if (debugStrToString != nullptr)
      m_game.AddComment(debugStrToString->GetString());
    BoardChanged();
    cmd << GoWritePoint(move);
  }
  if (m_game.GetPlayerName(color).empty())
    m_game.UpdatePlayerName(color, Player().Name());
}

void GoGtpEngine::CmdGenMove(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(1);
  SgBlackWhite color;
  if (cmd.NuArg() == 1)
    color = BlackWhiteArg(cmd, 0);
  else
    color = m_game.Board().ToPlay();
  OnGenMove(cmd, color);
}

/*void GoGtpEngine::CmdGenUctMove(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  std::unique_ptr<SgDebugToString> debugStrToString;
  if (m_debugToComment)
    debugStrToString.reset(new SgDebugToString(true));
  GoPoint move = GenUctMove(color, false);
  if (move == UCT_RESIGN) {
    cmd << "resign";
    const SgNode& node = m_game.AddResignNode(color);
    if (debugStrToString != 0) {
      m_game.AddComment(node, "\n\n");
      m_game.AddComment(node, debugStrToString->GetString());
    }
    AutoSave();
  } else {
    m_game.AddMove(move, color);
    if (debugStrToString.get() != 0)
      m_game.AddComment(debugStrToString->GetString());
    BoardChanged();
    cmd << GoWritePoint(move);
  }
  if (m_game.GetPlayerName(color).empty())
    m_game.UpdatePlayerName(color, Player().Name());
}*/

void GoGtpEngine::CmdGenMoveCleanup(GtpCommand& cmd) {
  GoRules rules = Board().Rules();
  bool oldCaptureDead = rules.CaptureDead();
  rules.SetCaptureDead(true);
  m_game.SetRulesGlobal(rules);
  RulesChanged();
  try {
    CmdGenMove(cmd);
  }
  catch (const GtpFailure& failure) {
    rules.SetCaptureDead(oldCaptureDead);
    m_game.SetRulesGlobal(rules);
    RulesChanged();
    throw failure;
  }
  rules.SetCaptureDead(oldCaptureDead);
  m_game.SetRulesGlobal(rules);
  RulesChanged();
}

void GoGtpEngine::CmdGetKomi(GtpCommand& cmd) {
  cmd.CheckArgNone();
  cmd << Board().Rules().Komi();
}

void GoGtpEngine::CmdGGUndo(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(1);
  Undo(cmd.NuArg() == 0 ? 0 : cmd.ArgMin<int>(0, 0));
  BoardChanged();
}

void GoGtpEngine::CmdKgsTimeSettings(GtpCommand& cmd) {
  if (cmd.NuArg() < 1)
    throw GtpFailure("Need at least one argument!");
  if (Board().MoveNumber() > 0)
    throw GtpFailure("cannot change time settings during game");
  const std::string& type = cmd.Arg(0);
  if (type == "none") {
    cmd.CheckNuArg(1);
    m_timeSettings = SgTimeSettings();
    ApplyTimeSettings();
  } else if (type == "absolute") {
    cmd.CheckNuArg(2);
    int mainTime = cmd.ArgMin<int>(1, 0);
    SgTimeSettings timeSettings(mainTime);
    if (m_timeSettings == timeSettings)
      return;
    m_timeSettings = timeSettings;
    ApplyTimeSettings();
  } else if (type == "byoyomi") {
    cmd.CheckNuArg(4);
    int mainTime = cmd.ArgMin<int>(1, 0);
    int overtime = cmd.ArgMin<int>(2, 0);
    SgTimeSettings timeSettings(mainTime, overtime, 1);
    if (m_timeSettings == timeSettings)
      return;
    m_timeSettings = timeSettings;
    ApplyTimeSettings();
  } else if (type == "canadian") {
    cmd.CheckNuArg(4);
    int mainTime = cmd.ArgMin<int>(1, 0);
    int overtime = cmd.ArgMin<int>(2, 0);
    int overtimeMoves = cmd.ArgMin<int>(3, 0);
    SgTimeSettings timeSettings(mainTime, overtime, overtimeMoves);
    if (m_timeSettings == timeSettings)
      return;
    m_timeSettings = timeSettings;
    ApplyTimeSettings();
  } else
    throw GtpFailure("Unknown type of time control");
}

void GoGtpEngine::CmdInterrupt(GtpCommand& cmd) {
  cmd.CheckArgNone();
}

void GoGtpEngine::CmdIsLegal(GtpCommand& cmd) {
  cmd.CheckNuArg(2);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  GoPoint move = MoveArg(cmd, 1);
  cmd << SgWriteBoolAsInt(Board().IsLegal(move, color));
}

void GoGtpEngine::CmdKomi(GtpCommand& cmd) {
  try {
    GoKomi komi(cmd.Arg());
    m_game.SetKomiGlobal(komi);
    m_defaultRules.SetKomi(komi);
    RulesChanged();
  }
  catch (const GoKomi::InvalidKomi& e) {
    throw GtpFailure(e.what());
  }
}

void GoGtpEngine::CmdListStones(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  const GoBoard& bd = Board();
  const GoPointSet& points = bd.All(color);
  bool isFirst = true;
  for (int row = bd.Size(); row >= 1; --row)
    for (int col = 1; col <= bd.Size(); ++col) {
      GoPoint p = Pt(col, row);
      if (points.Contains(p)) {
        if (!isFirst)
          cmd << ' ';
        cmd << GoWritePoint(p);
        isFirst = false;
      }
    }
}

void GoGtpEngine::CmdLoadSgf(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(2);
  string fileName = cmd.Arg(0);
  int moveNumber = -1;
  if (cmd.NuArg() == 2)
    moveNumber = cmd.ArgMin<int>(1, 1);
  std::ifstream in(fileName.c_str());
  if (!in)
    throw GtpFailure("could not open file");
  SgGameReader reader(in);
  SgNode* root = reader.ReadGame();
  if (root == 0)
    throw GtpFailure("no games in file");
  int boardSize = GoNodeUtil::GetBoardSize(root);
  if (boardSize < GO_MIN_SIZE || boardSize > GO_MAX_SIZE) {
    throw GtpFailure()
        << "file contains invalid board size, must be between "
        << GO_MIN_SIZE << " and " << GO_MAX_SIZE;
  }
  if (reader.GetWarnings().any()) {
    SgWarning() << fileName << ":\n";
    reader.PrintWarnings(SgDebug());
  }
  if (Board().MoveNumber() > 0)
    GameFinished();
  m_game.Init(root);
  if (!GoGameUtil::GotoBeforeMove(&m_game, moveNumber))
    throw GtpFailure("invalid move number");
  GoRules rules = m_defaultRules;
  rules.SetKomi(GoNodeUtil::GetKomi(m_game.CurrentNode()));
  rules.SetHandicap(GoNodeUtil::GetHandicap(m_game.CurrentNode()));
  m_game.SetRulesGlobal(rules);
  RulesChanged();
  if (m_playerList[m_playerType] != 0)
    m_playerList[m_playerType]->OnNewGame();
  BoardChanged();
}

void GoGtpEngine::CmdName(GtpCommand& cmd) {
  cmd.CheckArgNone();
  if (m_playerList[m_playerType] == 0)
    GtpEngine::CmdName(cmd);
  else
    cmd << m_playerList[m_playerType]->Name();
}

void GoGtpEngine::CmdParam(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(2);
  if (cmd.NuArg() == 0) {
    cmd << "[bool] accept_illegal " << m_acceptIllegal << '\n'
        << "[bool] debug_to_comment " << m_debugToComment << '\n'
        << "[bool] use_book " << m_useBook << '\n'
        << "[string] auto_save " << (m_autoSave ? m_autoSavePrefix : "")
        << '\n'
        << "[string] overhead " << m_overhead << '\n'
        << "[string] statistics_file " << m_statisticsFile << '\n'
        << "[string] timelimit " << m_timeLimit << '\n';
  } else if (cmd.NuArg() >= 1 && cmd.NuArg() <= 2) {
    string name = cmd.Arg(0);
    if (name == "accept_illegal")
      m_acceptIllegal = cmd.ArgT<bool>(1);
    else if (name == "debug_to_comment")
      m_debugToComment = cmd.ArgT<bool>(1);
    else if (name == "use_book")
      m_useBook = cmd.ArgT<bool>(1);
    else if (name == "auto_save") {
      string prefix = cmd.RemainingLine(0);
      if (prefix == "")
        m_autoSave = false;
      else
        SetAutoSave(prefix);
    } else if (name == "overhead") {
      m_overhead = cmd.ArgT<double>(1);
      m_game.TimeRecord().SetOverhead(m_overhead);
    } else if (name == "statistics_file")
      SetStatisticsFile(cmd.RemainingLine(0));
    else if (name == "timelimit")
      m_timeLimit = cmd.ArgT<double>(1);
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoGtpEngine::CmdParamRules(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(2);
  if (cmd.NuArg() == 0) {
    const GoRules& r = Board().Rules();
    cmd << "[bool] allow_suicide "
        << SgWriteBoolAsInt(r.AllowSuicide()) << '\n'
        << "[bool] capture_dead "
        << SgWriteBoolAsInt(r.CaptureDead()) << '\n'
        << "[bool] extra_handicap_komi "
        << SgWriteBoolAsInt(r.ExtraHandicapKomi()) << '\n'
        << "[bool] japanese_scoring "
        << SgWriteBoolAsInt(r.JapaneseScoring()) << '\n'
        << "[bool] two_passes_end_game "
        << SgWriteBoolAsInt(r.TwoPassesEndGame()) << '\n'
        << "[list/simple/superko/pos_superko] ko_rule "
        << KoRuleToString(r.GetKoRule()) << '\n';
  } else if (cmd.NuArg() == 2) {
    GoRules r = Board().Rules();
    string name = cmd.Arg(0);
    if (name == "allow_suicide") {
      r.SetAllowSuicide(cmd.ArgT<bool>(1));
      m_defaultRules.SetAllowSuicide(cmd.ArgT<bool>(1));
    } else if (name == "capture_dead") {
      r.SetCaptureDead(cmd.ArgT<bool>(1));
      m_defaultRules.SetCaptureDead(cmd.ArgT<bool>(1));
    } else if (name == "extra_handicap_komi") {
      r.SetExtraHandicapKomi(cmd.ArgT<bool>(1));
      m_defaultRules.SetExtraHandicapKomi(cmd.ArgT<bool>(1));
    } else if (name == "japanese_scoring") {
      r.SetJapaneseScoring(cmd.ArgT<bool>(1));
      m_defaultRules.SetJapaneseScoring(cmd.ArgT<bool>(1));
    } else if (name == "two_passes_end_game") {
      r.SetTwoPassesEndGame(cmd.ArgT<bool>(1));
      m_defaultRules.SetTwoPassesEndGame(cmd.ArgT<bool>(1));
    } else if (name == "ko_rule") {
      r.SetKoRule(KoRuleArg(cmd, 1));
      m_defaultRules.SetKoRule(KoRuleArg(cmd, 1));
    } else
      throw GtpFailure() << "unknown parameter: " << name;
    m_game.SetRulesGlobal(r);
    RulesChanged();
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoGtpEngine::CmdParamTimecontrol(GtpCommand& cmd) {
  auto* object =
      dynamic_cast<SgObjectWithDefaultTimeControl*>(&Player());
  if (object == 0)
    throw GtpFailure("current player is not a "
                         "SgObjectWithDefaultTimeControl");
  GoTimeControl* c = dynamic_cast<GoTimeControl*>(&object->TimeControl());
  if (c == 0)
    throw GtpFailure("current player does not have a GoTimeControl");
  cmd.CheckNuArgLessEqual(2);
  if (cmd.NuArg() == 0) {
    cmd << "fast_open_factor " << c->FastOpenFactor() << '\n'
        << "fast_open_moves " << c->FastOpenMoves() << '\n'
        << "final_space " << c->FinalSpace() << '\n'
        << "remaining_constant " << c->RemainingConstant() << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (name == "fast_open_factor")
      c->SetFastOpenFactor(cmd.ArgT<double>(1));
    else if (name == "fast_open_moves")
      c->SetFastOpenMoves(cmd.ArgMin<int>(1, 0));
    else if (name == "final_space")
      c->SetFinalSpace(std::max(cmd.ArgT<float>(1), 0.f));
    else if (name == "remaining_constant")
      c->SetRemainingConstant(std::max(cmd.ArgT<double>(1), 0.));
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoGtpEngine::CmdPlaceFreeHandicap(GtpCommand& cmd) {
  CheckBoardEmpty();
  int n = cmd.ArgMin<int>(0, 2);
  int size = Board().Size();
  SgVector<GoPoint> stones;
  try {
    stones = GoGtpCommandUtil::GetHandicapStones(size, n);
  }
  catch (const GtpFailure&) {}
  if (stones.Length() < n && m_playerList[m_playerType] != 0) {
    if (n >= 9 && size % 2 != 0 && size >= 9 && size <= 25)
      stones = GoGtpCommandUtil::GetHandicapStones(size, 9);
    else if (n >= 4 && size % 2 == 0 && (size >= 8 || size == 7)
        && size <= 25)
      stones = GoGtpCommandUtil::GetHandicapStones(size, 4);
    SgDebug() << "GoGtpEngine: Generating missing handicap\n";
    GoSetup setup;
    for (SgVectorIterator<GoPoint> it(stones); it; ++it)
      setup.AddBlack(*it);
    GoBoard& playerBd = m_playerList[m_playerType]->Board();
    playerBd.Init(playerBd.Size(), setup);
    for (int i = stones.Length(); i < n; ++i) {
      GoPoint p = GenMove(SG_BLACK, true);
      SgDebug() << "GoGtpEngine: " << i << ' ' << GoWritePoint(p)
                << '\n';
      if (p == GO_PASS)
        break;
      playerBd.Play(p, SG_BLACK);
      stones.PushBack(p);
    }
  }
  DBG_ASSERT(stones.Length() <= n);
  PlaceHandicap(stones);
  cmd << SgWritePointList(stones, "", false);
}

void GoGtpEngine::CmdPlayAgainst(GtpCommand& cmd) {
  cmd.CheckNuArg(2);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  GoPoint move = MoveArg(cmd, 1);
  Play(color, move);
  BoardChanged();

  OnGenMove(cmd, SgOppBW(color));
}

void GoGtpEngine::CmdPlaySequence(GtpCommand& cmd) {
  const SgNode* oldCurrentNode = m_game.CurrentNode();
  try {
    for (size_t i = 0; i < cmd.NuArg(); i += 2)
      Play(BlackWhiteArg(cmd, i), MoveArg(cmd, i + 1));
  }
  catch (GtpFailure fail) {
    if (oldCurrentNode != m_game.CurrentNode()) {
      m_game.GoToNode(oldCurrentNode);
      BoardChanged();
    }
    throw fail;
  }
  BoardChanged();
}

void GoGtpEngine::CmdPointInfo(GtpCommand& cmd) {
  GoPoint p = PointArg(cmd);
  const GoBoard& bd = Board();
  cmd << "Point:\n"
      << SgWriteLabel("Color") << SgEBW(bd.GetColor(p)) << '\n'
      << SgWriteLabel("InCenter") << bd.InCenter(p) << '\n'
      << SgWriteLabel("InCorner") << bd.InCorner(p) << '\n'
      << SgWriteLabel("Line") << bd.Line(p) << '\n'
      << SgWriteLabel("OnEdge") << bd.OnEdge(p) << '\n'
      << SgWriteLabel("EmptyNb") << bd.NumEmptyNeighbors(p) << '\n'
      << SgWriteLabel("EmptyNb8") << bd.Num8EmptyNeighbors(p) << '\n'
      << SgWriteLabel("Pos") << bd.Pos(p) << '\n';
  if (bd.Occupied(p)) {
    SgVector<GoPoint> adjBlocks;
    GoBoardUtil::AdjacentBlocks(bd, p, GO_MAXPOINT, &adjBlocks);
    cmd << "Block:\n"
        << SgWritePointList(adjBlocks, "AdjBlocks", true)
        << SgWriteLabel("Anchor") << GoWritePoint(bd.Anchor(p)) << '\n'
        << SgWriteLabel("InAtari") << bd.InAtari(p) << '\n'
        << SgWriteLabel("IsSingleStone") << bd.IsSingleStone(p) << '\n'
        << SgWriteLabel("Liberties") << bd.NumLiberties(p) << '\n'
        << SgWriteLabel("Stones") << bd.NumStones(p) << '\n';
  } else
    cmd << "EmptyPoint:\n"
        << SgWriteLabel("IsFirst") << bd.IsFirst(p) << '\n'
        << SgWriteLabel("IsLegal/B") << bd.IsLegal(p, SG_BLACK) << '\n'
        << SgWriteLabel("IsLegal/W") << bd.IsLegal(p, SG_WHITE) << '\n'
        << SgWriteLabel("IsSuicide") << bd.IsSuicide(p) << '\n'
        << SgWriteLabel("MakesNakadeShape/B")
        << GoEyeUtil::MakesNakadeShape(bd, p, SG_BLACK) << '\n'
        << SgWriteLabel("MakesNakadeShape/W")
        << GoEyeUtil::MakesNakadeShape(bd, p, SG_WHITE) << '\n'
        << SgWriteLabel("IsSimpleEye/B")
        << GoEyeUtil::IsSimpleEye(bd, p, SG_BLACK) << '\n'
        << SgWriteLabel("IsSimpleEye/W")
        << GoEyeUtil::IsSimpleEye(bd, p, SG_WHITE) << '\n'
        << SgWriteLabel("IsSinglePointEye/B")
        << GoEyeUtil::IsSinglePointEye(bd, p, SG_BLACK) << '\n'
        << SgWriteLabel("IsSinglePointEye/W")
        << GoEyeUtil::IsSinglePointEye(bd, p, SG_WHITE) << '\n'
        << SgWriteLabel("IsPossibleEye/B")
        << GoEyeUtil::IsPossibleEye(bd, SG_BLACK, p) << '\n'
        << SgWriteLabel("IsPossibleEye/W")
        << GoEyeUtil::IsPossibleEye(bd, SG_WHITE, p) << '\n';
}

void GoGtpEngine::CmdPlayerBoard(GtpCommand& cmd) {
  WriteBoardInfo(cmd, Player().Board());
}

void GoGtpEngine::CmdPointNumbers(GtpCommand& cmd) {
  cmd.CheckArgNone();
  GoPointArray<int> array(0);
  for (GoBoard::Iterator p(Board()); p; ++p)
    array[*p] = *p;
  cmd << SgWritePointArray<int>(array, Board().Size());
}

void GoGtpEngine::CmdQuit(GtpCommand& cmd) {
  cmd.CheckArgNone();
  if (Board().MoveNumber() > 0)
    GameFinished();
  GtpEngine::CmdQuit(cmd);
}

void GoGtpEngine::CmdRegGenMove(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SgRandom::SetSeed(SgRandom::Seed());
  GoPoint move = GenMove(BlackWhiteArg(cmd, 0), true);
  if (move == UCT_RESIGN)
    cmd << "resign";
  else
    cmd << GoWritePoint(move);
}

void GoGtpEngine::CmdRegGenMoveToPlay(GtpCommand& cmd) {
  cmd.CheckArgNone();
  SgRandom::SetSeed(SgRandom::Seed());
  GoPoint move = GenMove(Board().ToPlay(), true);
  cmd << GoWritePoint(move);
}

void GoGtpEngine::CmdRules(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  string arg = cmd.Arg(0);
  try {
    SetNamedRules(arg);
  }
  catch (const SgException&) {
    throw GtpFailure() << "unknown rules: " << arg;
  }
}

void GoGtpEngine::CmdSaveSgf(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  SaveGame(cmd.Arg(0));
}

void GoGtpEngine::CmdSentinelFile(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  path sentinelFile = path(cmd.Arg(0));
  if (!sentinelFile.empty())
    try {
      remove(sentinelFile);
    }
    catch (const std::exception& e) {
      throw GtpFailure() << "could not remove sentinel file: "
                         << e.what();
    }
  m_sentinelFile = sentinelFile;
}

void GoGtpEngine::CmdSetFreeHandicap(GtpCommand& cmd) {
  SgVector<GoPoint> stones = PointListArg(cmd);
  if (stones.RemoveDuplicates())
    throw GtpFailure("duplicate handicap stones not allowed");
  PlaceHandicap(stones);
}

void GoGtpEngine::CmdSetInfo(GtpCommand& cmd) {
  const string& key = cmd.Arg(0);
  string value = cmd.RemainingLine(0);
  if (key == "game_name")
    m_game.UpdateGameName(value);
  else if (key == "player_black")
    m_game.UpdatePlayerName(SG_BLACK, value);
  else if (key == "player_white")
    m_game.UpdatePlayerName(SG_WHITE, value);
  else if (key == "result") {
    m_game.UpdateResult(value);
    m_isPonderPosition = false;
  }
  AutoSave();
}

void GoGtpEngine::CmdSetup(GtpCommand& cmd) {
  const GoBoard& bd = Board();
  if (bd.MoveNumber() > 0)
    throw GtpFailure("setup only allowed on empty board");
  if (cmd.NuArg() % 2 != 0)
    throw GtpFailure("need even number of arguments");
  SgBWArray<GoPointSet> points;
  for (size_t i = 0; i < cmd.NuArg(); i += 2) {
    SgBlackWhite c = BlackWhiteArg(cmd, i);
    GoPoint p = PointArg(cmd, i + 1);
    for (SgBWIterator it; it; ++it)
      points[*it].Exclude(p);
    points[c].Include(p);
  }
  m_game.SetupPosition(points);
  BoardChanged();
}

void GoGtpEngine::CmdSetupPlayer(GtpCommand& cmd) {
  cmd.CheckNuArg(1);
  m_game.SetToPlay(BlackWhiteArg(cmd, 0));
  BoardChanged();
}

void GoGtpEngine::ShowBoard(GtpCommand& cmd) const {
#ifdef SHOW_BOARD
  cmd << '\n' << Board();
#endif
}

void GoGtpEngine::DebugBoard() const {
  SgDebug() << Board();
}

void GoGtpEngine::CmdShowBoard(GtpCommand& cmd) {
  cmd.CheckArgNone();
  ShowBoard(cmd);
}

void GoGtpEngine::CmdStaticScore(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(1);
  std::string type = "tromp_taylor_score";
  bool japaneseScoring = false;
  if (cmd.NuArg() > 0) {
    type = cmd.Arg(0);
    if (type == "japanese_score")
      japaneseScoring = true;
    else if (type != "tromp_taylor_score")
      cmd << ("Warning: Unknown type of static scoring, using tromp_taylor_score!\n");
  }
  GoModBoard modBoard(Board());
  GoBoard& bd = modBoard.Board();
  GoBoardRestorer r(bd);
  bd.Rules().SetJapaneseScoring(japaneseScoring);
  float komi = bd.Rules().Komi().ToFloat();
  float score = GoBoardUtil::Score(bd, komi);
  cmd << GoUtil::ScoreToString(score);
}

void GoGtpEngine::CmdTimeLastMove(GtpCommand& cmd) {
  cmd.CheckArgNone();
  cmd << std::setprecision(2) << m_timeLastMove;
}

void GoGtpEngine::CmdTimeLeft(GtpCommand& cmd) {
  cmd.CheckNuArg(3);
  SgBlackWhite color = BlackWhiteArg(cmd, 0);
  int timeLeft = std::max(0, cmd.ArgT<int>(1));
  int movesLeft = cmd.ArgMin<int>(2, 0);
  SgTimeRecord& time = m_game.TimeRecord();
  time.SetTimeLeft(color, timeLeft);
  time.SetMovesLeft(color, movesLeft);
}

void GoGtpEngine::CmdTimeSettings(GtpCommand& cmd) {
  cmd.CheckNuArg(3);
  int mainTime = cmd.ArgMin<int>(0, 0);
  int overtime = cmd.ArgMin<int>(1, 0);
  int overtimeMoves = cmd.ArgMin<int>(2, 0);
  SgTimeSettings timeSettings(mainTime, overtime, overtimeMoves);
  if (m_timeSettings == timeSettings)
    return;
  if (Board().MoveNumber() > 0)
    throw GtpFailure("cannot change time settings during game");
  m_timeSettings = timeSettings;
  ApplyTimeSettings();
}

void GoGtpEngine::CmdUndo(GtpCommand& cmd) {
  cmd.CheckArgNone();
  Undo(1);
  BoardChanged();
}

void GoGtpEngine::CheckMoveStackOverflow() const {
  const int RESERVE = 50;
  if (Board().MoveNumber() >= GO_MAX_NUM_MOVES - RESERVE)
    throw GtpFailure("too many moves");
  if (Board().StackOverflowLikely())
    throw GtpFailure("move stack overflow");
}

std::vector<std::string> GoGtpEngine::CreateStatisticsSlots() {
  return std::vector<string>();
}

SgBlackWhite GoGtpEngine::BlackWhiteArg(const GtpCommand& cmd,
                                        std::size_t number) const {
  return GoGtpCommandUtil::BlackWhiteArg(cmd, number);
}

void GoGtpEngine::CreateAutoSaveFileName() {
  time_t timeValue = time(0);
  struct tm* timeStruct = localtime(&timeValue);
  char timeBuffer[128];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d%H%M%S", timeStruct);
  std::ostringstream fileName;
  fileName << m_autoSavePrefix << timeBuffer << ".sgf";
  m_autoSaveFileName = fileName.str();
}

void GoGtpEngine::DumpState(std::ostream& out) const {
  out << "GoGtpEngine board:\n";
  GoBoardUtil::DumpBoard(Board(), out);
  if (m_playerList[m_playerType] != 0) {
    out << "GoPlayer board:\n";
    GoBoardUtil::DumpBoard(m_playerList[m_playerType]->Board(), out);
  }
}

SgEmptyBlackWhite GoGtpEngine::EmptyBlackWhiteArg(const GtpCommand& cmd,
                                                  std::size_t number) const {
  return GoGtpCommandUtil::EmptyBlackWhiteArg(cmd, number);
}

GoPoint GoGtpEngine::EmptyPointArg(const GtpCommand& cmd,
                                   std::size_t number) const {
  return GoGtpCommandUtil::EmptyPointArg(cmd, number, Board());
}

void GoGtpEngine::GameFinished() {
  if (m_playerList[m_playerType] != 0)
    m_playerList[m_playerType]->OnGameFinished();
}

GoPoint GoGtpEngine::GenMove(SgBlackWhite color, bool ignoreClock) {
  DBG_ASSERT_BW(color);
  CheckMoveStackOverflow();
  StartStatistics();
  GoPlayer& player = Player();
  double startTime = SgTime::Get();
  SgTimeRecord time;
  if (ignoreClock || m_timeSettings.IsUnknown())
    time = SgTimeRecord(true, m_timeLimit);
  else {
    time = m_game.TimeRecord();
    time.UpdateTimeLeft();
  }
  AddStatistics("GAME", m_autoSaveFileName);
  AddStatistics("MOVE", m_game.CurrentMoveNumber() + 1);
  GoPoint move = GenDeepUctMove(color, false);
  m_mpiSynchronizer->SynchronizeMove(move);
  if (move != GO_NULLMOVE) {
    SgDebug() << "GoGtpEngine: Using move from opening book\n";
    AddStatistics("BOOK", 1);
  } else
    AddStatistics("BOOK", 0);
  if (move == GO_NULLMOVE) {
    player.ClearSearchTraces();
    move = player.GenMove(time, color);
    SgNode* searchTraces = player.TransferSearchTraces();
    if (searchTraces != 0)
      m_game.AppendChild(searchTraces);
  }
  m_mpiSynchronizer->SynchronizeMove(move);
  m_timeLastMove = SgTime::Get() - startTime;
  AddStatistics("TIME", m_timeLastMove);
  if (move == GO_NULLMOVE)
    throw GtpFailure() << player.Name() << " generated NULLMOVE";
  if (move == UCT_RESIGN)
    m_isPonderPosition = false;
  else
    CheckLegal(player.Name() + " generated illegal move: ", color, move,
               false);
  AddPlayStatistics();
  SaveStatistics();
  return move;
}

GoPoint GoGtpEngine::GenUctMove(SgBlackWhite color, bool ignoreClock) {
  DBG_ASSERT_BW(color);
  CheckMoveStackOverflow();
  StartStatistics();
  GoPlayer& player = Player();
  double startTime = SgTime::Get();
  SgTimeRecord time;
  if (ignoreClock || m_timeSettings.IsUnknown())
    time = SgTimeRecord(true, m_timeLimit);
  else {
    time = m_game.TimeRecord();
    time.UpdateTimeLeft();
  }
  AddStatistics("GAME", m_autoSaveFileName);
  AddStatistics("MOVE", m_game.CurrentMoveNumber() + 1);

  GoPoint move = GO_NULLMOVE;
  m_mpiSynchronizer->SynchronizeMove(move);
  player.ClearSearchTraces();
  move = player.GenMove(time, color);
  SgNode* searchTraces = player.TransferSearchTraces();
  if (searchTraces != 0)
    m_game.AppendChild(searchTraces);
  m_mpiSynchronizer->SynchronizeMove(move);

  m_timeLastMove = SgTime::Get() - startTime;
  AddStatistics("TIME", m_timeLastMove);
  if (move == GO_NULLMOVE)
    throw GtpFailure() << player.Name() << " generated NULLMOVE";
  if (move == UCT_RESIGN)
    m_isPonderPosition = false;
  else
    CheckLegal(player.Name() + " generated illegal move: ", color, move, false);
  AddPlayStatistics();
  SaveStatistics();
  return move;
}

GoPoint GoGtpEngine::GenDeepUctMove(SgBlackWhite color, bool ignoreClock) {
  DBG_ASSERT(m_playerType == PT_DeepUctPlayer);
  DBG_ASSERT_BW(color);
  CheckMoveStackOverflow();
  StartStatistics();

  auto& player = (UctDeepPlayer&)Player();
  player.TryInitNeuralNetwork();
  double startTime = SgTime::Get();
  SgTimeRecord timeRecord;
  if (ignoreClock || m_timeSettings.IsUnknown())
    timeRecord = SgTimeRecord(true, m_timeLimit);
  else {
    timeRecord = m_game.TimeRecord();
    timeRecord.UpdateTimeLeft();
  }
  AddStatistics("GAME", m_autoSaveFileName);
  AddStatistics("MOVE", m_game.CurrentMoveNumber() + 1);

  GoPoint move = GO_NULLMOVE;
  m_mpiSynchronizer->SynchronizeMove(move);
  player.ClearSearchTraces();
  move = player.GenMove(timeRecord, color);
  SgNode* searchTraces = player.TransferSearchTraces();
  if (searchTraces != 0)
    m_game.AppendChild(searchTraces);
  m_mpiSynchronizer->SynchronizeMove(move);

  m_timeLastMove = SgTime::Get() - startTime;
  AddStatistics("TIME", m_timeLastMove);
  if (move == GO_NULLMOVE)
    throw GtpFailure() << player.Name() << " generated NULLMOVE";
  if (move == UCT_RESIGN)
    m_isPonderPosition = false;
  else
    CheckLegal(player.Name() + " generated illegal move: ", color, move, false);
  AddPlayStatistics();
  SaveStatistics();
  return move;
}

void GoGtpEngine::Init(int size) {
  m_game.Init(size, m_defaultRules);
  m_game.UpdateDate(SgTime::TodaysDate());
  ApplyTimeSettings();
  CreateAutoSaveFileName();
  BoardChanged();
}

void GoGtpEngine::InitStatistics() {
  m_statisticsSlots.clear();
  m_statisticsSlots.emplace_back("GAME");
  m_statisticsSlots.emplace_back("MOVE");
  m_statisticsSlots.emplace_back("TIME");
  m_statisticsSlots.emplace_back("BOOK");
  std::vector<string> slots = CreateStatisticsSlots();
  for (std::vector<string>::const_iterator i = slots.begin();
       i != slots.end();
       ++i) {
    if (i->find('\t') != string::npos)
      throw SgException("GoGtpEngine::InitStatistics: statistics slot"
                            " contains tab: '" + (*i) + "'");
    if (find(m_statisticsSlots.begin(), m_statisticsSlots.end(), *i)
        != m_statisticsSlots.end())
      throw SgException("GoGtpEngine::InitStatistics: duplicate"
                            " statistics slot '" + (*i) + "'");
    m_statisticsSlots.push_back(*i);
  }
  if (MpiSynchronizer()->IsRootProcess()) {
    std::ofstream out(m_statisticsFile.c_str(), std::ios::app);
    out << '#';
    for (size_t i = 0; i < m_statisticsSlots.size(); ++i) {
      out << m_statisticsSlots[i];
      if (i < m_statisticsSlots.size() - 1)
        out << '\t';
      else
        out << '\n';
    }
  }
}

GoPoint GoGtpEngine::MoveArg(const GtpCommand& cmd, std::size_t number) const {
  return GoGtpCommandUtil::MoveArg(cmd, number, Board());
}

void GoGtpEngine::PlaceHandicap(const SgVector<GoPoint>& stones) {
  CheckBoardEmpty();
  m_game.PlaceHandicap(stones);
  RulesChanged();
  BoardChanged();
}

GoPlayer& GoGtpEngine::Player() const {
  if (m_playerList[m_playerType] == 0)
    throw GtpFailure("no player set");
  return *m_playerList[m_playerType];
}

GoPoint GoGtpEngine::PointArg(const GtpCommand& cmd) const {
  return GoGtpCommandUtil::PointArg(cmd, Board());
}

GoPoint GoGtpEngine::PointArg(const GtpCommand& cmd, std::size_t number) const {
  return GoGtpCommandUtil::PointArg(cmd, number, Board());
}

SgVector<GoPoint> GoGtpEngine::PointListArg(const GtpCommand& cmd,
                                            std::size_t number) const {
  return GoGtpCommandUtil::PointListArg(cmd, number, Board());
}

SgVector<GoPoint> GoGtpEngine::PointListArg(const GtpCommand& cmd) const {
  return GoGtpCommandUtil::PointListArg(cmd, Board());
}

void GoGtpEngine::RespondNumberArray(GtpCommand& cmd,
                                     const GoPointArray<int>& array,
                                     int scale) {
  GoGtpCommandUtil::RespondNumberArray(cmd, array, scale, Board());
}

void GoGtpEngine::RulesChanged() {
  if (m_playerList[m_playerType] != 0)
    m_playerList[m_playerType]->UpdateSubscriber();
}

GoRules& GoGtpEngine::GetDefaultRules() {
  return m_defaultRules;
}

void GoGtpEngine::SaveGame(const std::string& fileName) const {
  if (MpiSynchronizer()->IsRootProcess()) {
    try {
      std::ofstream out(fileName.c_str());
      SgGameWriter writer(out);
      writer.WriteGame(m_game.Root(), true, 0, 1, 19);
    }
    catch (const SgException& e) {
      throw GtpFailure(e.what());
    }
  }
}

void GoGtpEngine::SaveStatistics() {
  if (!MpiSynchronizer()->IsRootProcess()
      || m_statisticsFile == "")
    return;
  DBG_ASSERT(m_statisticsValues.size() == m_statisticsSlots.size());
  std::ofstream out(m_statisticsFile.c_str(), std::ios::app);
  for (size_t i = 0; i < m_statisticsSlots.size(); ++i) {
    out << m_statisticsValues[i];
    if (i < m_statisticsSlots.size() - 1)
      out << '\t';
    else
      out << '\n';
  }
}

void GoGtpEngine::SetAutoSave(const std::string& prefix) {
  m_autoSave = true;
  m_autoSavePrefix = prefix;
  CreateAutoSaveFileName();
}

void GoGtpEngine::SetAutoShowBoard(bool showBoard) {
  m_autoShowBoard = showBoard;
  if (m_autoShowBoard)
    SgDebug() << Board();
}

inline void GoGtpEngine::SetStatisticsFile(const std::string& fileName) {
  m_statisticsFile = fileName;
  InitStatistics();
}

void GoGtpEngine::SetPlayer(GoPlayer* player, PlayerType type) {
  if (m_playerList[type] != player) {
    delete m_playerList[type];
    m_playerList[type] = player;
  }
  if (m_playerList[type] != 0) {
    m_playerList[type]->UpdateSubscriber();
    m_playerList[type]->OnNewGame();
  }
}

void GoGtpEngine::SetPlayerType(PlayerType type) {
  m_playerType = type;
}

void GoGtpEngine::SetNamedRules(const string& namedRules) {
  m_defaultRules.SetNamedRules(namedRules);
  m_game.SetRulesGlobal(m_defaultRules);
  RulesChanged();
}

void GoGtpEngine::StartStatistics() {
  m_statisticsValues.clear();
  m_statisticsValues.resize(m_statisticsSlots.size(), "-");
}

GoPoint GoGtpEngine::StoneArg(const GtpCommand& cmd, std::size_t number) const {
  return GoGtpCommandUtil::StoneArg(cmd, number, Board());
}

void GoGtpEngine::Undo(int n) {
  DBG_ASSERT(n >= 0);
  const SgNode* node = m_game.CurrentNode();
  for (int i = 0; i < n; ++i) {
    if (!node->HasNodeMove() || !node->HasFather())
      throw GtpFailure() << "cannot undo " << n << " move(s)";
    node = node->Father();
  }
  m_game.GoToNode(node);
}

void GoGtpEngine::WriteBoardInfo(GtpCommand& cmd, const GoBoard& bd) {
  cmd.CheckNuArgLessEqual(1);
  if (cmd.NuArg() == 1) {
    string arg = cmd.Arg(0);
    if (arg == "countplay")
      cmd << bd.CountPlay();
    else
      throw GtpFailure() << "unknown argument " << arg;
    return;
  }
  cmd << "Board:\n"
      << SgWriteLabel("Hash") << bd.GetHashCode() << '\n'
      << SgWriteLabel("HashToPlay") << bd.GetHashCodeInclToPlay() << '\n'
      << SgWriteLabel("KoColor") << SgEBW(bd.KoColor()) << '\n'
      << SgWriteLabel("MoveNumber") << bd.MoveNumber() << '\n'
      << SgWriteLabel("NumStones[B]") << bd.TotalNumStones(SG_BLACK) << '\n'
      << SgWriteLabel("NumStones[W]") << bd.TotalNumStones(SG_WHITE) << '\n'
      << SgWriteLabel("NumEmpty") << bd.TotalNumEmpty() << '\n'
      << SgWriteLabel("ToPlay") << SgBW(bd.ToPlay()) << '\n'
      << SgWriteLabel("CountPlay") << bd.CountPlay() << '\n'
      << "Sets:\n"
      << PointSetWriter(bd.AllPoints(), "AllPoints")
      << PointSetWriter(bd.All(SG_BLACK), "AllBlack")
      << PointSetWriter(bd.All(SG_WHITE), "AllWhite")
      << PointSetWriter(bd.AllEmpty(), "AllEmpty")
      << PointSetWriter(bd.Corners(), "Corners")
      << PointSetWriter(bd.Edges(), "Edges")
      << PointSetWriter(bd.Centers(), "Centers")
      << PointSetWriter(bd.SideExtensions(), "SideExtensions")
      << PointSetWriter(bd.Occupied(), "Occupied");
}

#if GTPENGINE_PONDER

void GoGtpEngine::Ponder() {
  if (m_playerList[m_playerType] == 0 || !m_isPonderPosition)
    return;
  boost::xtime time;
  boost::xtime_get(&time, boost::TIME_UTC_);
  bool aborted = false;
  for (int i = 0; i < 200; ++i) {
    if (ForceAbort()) {
      aborted = true;
      break;
    }
    time.nsec += 1000000;
    boost::thread::sleep(time);
  }
  m_mpiSynchronizer->SynchronizeUserAbort(aborted);
  if (!aborted) {
    m_mpiSynchronizer->OnStartPonder();
    m_playerList[m_playerType]->Ponder();
    m_mpiSynchronizer->OnEndPonder();
  }
}

void GoGtpEngine::StopPonder() {
  SetForceAbort(true);
}

void GoGtpEngine::InitPonder() {
  SetForceAbort(false);
}

#endif

#if GTPENGINE_INTERRUPT

void GoGtpEngine::Interrupt() {
  SetForceAbort(true);
}

#endif

void GoGtpEngine::SetMpiSynchronizer(const MpiSynchronizerHandle& handle) {
  m_mpiSynchronizer = MpiSynchronizerHandle(handle);
}

MpiSynchronizerHandle GoGtpEngine::MpiSynchronizer() {
  return MpiSynchronizerHandle(m_mpiSynchronizer);
}

const MpiSynchronizerHandle GoGtpEngine::MpiSynchronizer() const {
  return MpiSynchronizerHandle(m_mpiSynchronizer);
}

GoGtpAssertionHandler::GoGtpAssertionHandler(const GoGtpEngine& engine)
    : m_engine(engine) {}

void GoGtpAssertionHandler::Run() {
  m_engine.DumpState(SgDebug());
  SgDebug() << flush;
}
