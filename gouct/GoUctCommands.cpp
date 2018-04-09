
#include "platform/SgSystem.h"
#include "GoUctCommands.h"

#include "GoGame.h"
#include "GoGtpCommandUtil.h"
#include "GoBoardUtil.h"
#include "GoSafetySolver.h"
#include "GoUctDefaultMoveFilter.h"
#include "GoUctEstimatorStat.h"
#include "GoUctGlobalSearch.h"
#include "GoUctPlayer.h"
#include "GoUctPlayoutPolicy.h"
#include "GoUctUtil.h"
#include "GoUtil.h"
#include "platform/SgException.h"
#include "SgGtpUtil.h"
#include "board/GoPointSetUtil.h"
#include "platform/SgRestorer.h"
#include "UctTreeUtil.h"
#include "board/SgWrite.h"

using boost::format;
using GoGtpCommandUtil::BlackWhiteArg;
using GoGtpCommandUtil::EmptyPointArg;
using GoGtpCommandUtil::PointArg;
using std::string;

namespace {

bool IsBlockedInDeterministicMode(const string &name) {
  return name == "ignore_clock"
      || name == "reuse_subtree"
      || name == "number_threads"
      || name == "max_nodes";
}

bool IsCorrected(GoUctPlayoutPolicy<GoBoard> &policy,
                 GoPoint move) {
  GoPoint corrected = move;
  policy.CorrectMove(GoUctUtil::DoFalseEyeToCaptureCorrection, corrected, GOUCT_REPLACE_CAPTURE);
  if (corrected != move)
    return true;
  else {
    policy.CorrectMove(GoUctUtil::DoSelfAtariCorrection, corrected, GOUCT_SELFATARI_CORRECTION);
    return move != corrected;
  }
}

bool IsPolicyMove(GoUctPlayoutPolicy<GoBoard> &policy, GoPoint move) {
  return move == policy.GenerateMove();
}

GoUctLiveGfx LiveGfxArg(const GtpCommand &cmd, size_t number) {
  string arg = cmd.ArgToLower(number);
  if (arg == "none")
    return GOUCT_LIVEGFX_NONE;
  if (arg == "counts")
    return GOUCT_LIVEGFX_COUNTS;
  if (arg == "sequence")
    return GOUCT_LIVEGFX_SEQUENCE;
  throw GtpFailure() << "unknown live-gfx argument \"" << arg << '"';
}

string LiveGfxToString(GoUctLiveGfx mode) {
  switch (mode) {
    case GOUCT_LIVEGFX_NONE:return "none";
    case GOUCT_LIVEGFX_COUNTS:return "counts";
    case GOUCT_LIVEGFX_SEQUENCE:return "sequence";
    default:DBG_ASSERT(false);
      return "?";
  }
}

GoUctGlobalSearchMode SearchModeArg(const GtpCommand &cmd, size_t number) {
  string arg = cmd.ArgToLower(number);
  if (arg == "playout_policy")
    return GOUCT_SEARCHMODE_PLAYOUTPOLICY;
  if (arg == "deepuct")
    return GOUCT_SEARCHMODE_DEEPUCT;
  if (arg == "uct")
    return GOUCT_SEARCHMODE_UCT;
  if (arg == "one_ply")
    return GOUCT_SEARCHMODE_ONEPLY;
  throw GtpFailure() << "unknown search mode argument \"" << arg << '"';
}

string SearchModeToString(GoUctGlobalSearchMode mode) {
  switch (mode) {
    case GOUCT_SEARCHMODE_PLAYOUTPOLICY:return "playout_policy";
    case GOUCT_SEARCHMODE_DEEPUCT:return "deepuct";
    case GOUCT_SEARCHMODE_UCT:return "uct";
    case GOUCT_SEARCHMODE_ONEPLY:return "one_ply";
    default:DBG_ASSERT(false);
      return "?";
  }
}
}

GoUctCommands::GoUctCommands(GoPlayer *&player, const GoGame &game, GoRules &rules)
    : m_bd(game.Board()),
      m_player(player),
      m_trainer(game, rules),
      m_game(game) {}

void GoUctCommands::AddGoGuiAnalyzeCommands(GtpCommand &cmd) {
  cmd <<
      "dboard/Approximate Territory/approximate_territory\n"
          "none/Deterministic Mode/deterministic_mode\n"
          "gfx/Uct Additive Knowledge/uct_additive_knowledge\n"
          "gfx/Uct Bounds/uct_bounds\n"
          "plist/Uct Default Policy/uct_default_policy\n"
          "gfx/Uct Gfx/uct_gfx\n"
          "none/IsPolicyCorrectedMove/is_policy_corrected_move\n"
          "none/IsPolicyMove/is_policy_move\n"
          "gfx/Uct Ladder Knowledge/uct_ladder_knowledge\n"
          "none/Uct Max Memory/uct_max_memory %s\n"
          "plist/Uct Moves/uct_moves\n"
          "none/Uct Node Info/uct_node_info\n"
          "param/Uct Param GlobalSearch/uct_param_globalsearch\n"
          "param/Uct Param Feature Knowledge/uct_param_feature_knowledge\n"
          "param/Uct Param Policy/uct_param_policy\n"
          "param/Uct Param Player/uct_param_player\n"
          "param/Uct Param RootFilter/uct_param_rootfilter\n"
          "param/Uct Param TreeFilter/uct_param_treefilter\n"
          "param/Uct Param Search/uct_param_search\n"
          "plist/Uct Patterns/uct_patterns\n"
          "pstring/Uct Policy Corrected Moves/uct_policy_corrected_moves\n"
          "pstring/Uct Policy Moves/uct_policy_moves\n"
          "pstring/Uct Policy Moves Simple List/uct_policy_moves_simple\n"
          "gfx/Uct Prior Knowledge/uct_prior_knowledge\n"
          "sboard/Uct Rave Values/uct_rave_values\n"
          "plist/Uct Root Filter/uct_root_filter\n"
          "none/Uct SaveGames/uct_savegames %w\n"
          "none/Uct SaveTree/uct_savetree %w\n"
          "gfx/Uct Sequence/uct_sequence\n"
          "hstring/Uct Stat Player/uct_stat_player\n"
          "none/Uct Stat Player Clear/uct_stat_player_clear\n"
          "hstring/Uct Stat Policy/uct_stat_policy\n"
          "none/Uct Stat Policy Clear/uct_stat_policy_clear\n"
          "hstring/Uct Stat Search/uct_stat_search\n"
          "dboard/Uct Stat Territory/uct_stat_territory\n";
}

void GoUctCommands::CmdAdditiveKnowledge(GtpCommand &cmd) {
  DisplayKnowledge(cmd, true);
}

void GoUctCommands::CmdBounds(GtpCommand &cmd) {
  cmd.CheckArgNone();
  const GoUctSearch &search = Search();
  const UctSearchTree &tree = search.Tree();
  const UctNode &root = tree.Root();
  bool hasPass = false;
  UctValueType passBound = 0;
  cmd << "LABEL";
  for (UctChildNodeIterator it(tree, root); it; ++it) {
    const UctNode &child = *it;
    GoPoint move = child.Move();
    UctValueType bound = search.GetBound(search.Rave(), root, child);
    if (move == GO_PASS) {
      hasPass = true;
      passBound = bound;
    } else
      cmd << ' ' << GoWritePoint(move) << ' '
          << std::fixed << std::setprecision(2) << bound;
  }
  cmd << '\n';
  if (hasPass)
    cmd << "TEXT PASS="
        << std::fixed << std::setprecision(2) << passBound << '\n';
}

void GoUctCommands::CmdDeterministicMode(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GoUctSearch &s = Search();
  GoUctPlayerType &p = Player();
  SgDeterministic::SetDeterministicMode(true);
  SgDebug() <<
            "This command irreversibly puts the player into deterministic mode.\n"
                "It also enforces and fixes the following parameters:\n"
                "uct_param_player ignore_clock true\n"
                "uct_param_player reuse_subtree false\n"
                "uct_param_search number_threads 1\n"
                "uct_param_search max_nodes 5000000\n";
  if (p.MaxGames() == std::numeric_limits<UctValueType>::max())
    SgWarning() << "Set Uct Param Player-> Max Games to finite "
        "value in deterministic mode\n";
  p.SetIgnoreClock(true);
  p.SetReuseSubtree(false);
  s.SetNumberThreads(1);
  SgRandom::SetSeed(1);
  s.SetCheckTimeInterval(UctValueType(1500));
  s.SetMaxNodes(std::size_t(5000000));

  cmd << "ignore_clock " << p.IgnoreClock() << '\n'
      << "reuse_subtree " << p.ReuseSubtree() << '\n'
      << "number_threads " << s.NumberThreads() << '\n'
      << "set_random_seed " << SgRandom::Seed() << '\n'
      << "m_check_time_interval " << s.CheckTimeInterval() << '\n'
      << "max_nodes " << s.MaxNodes() << '\n';
}

void GoUctCommands::CmdEstimatorStat(GtpCommand &cmd) {
  cmd.CheckNuArg(4);
  size_t trueValueMaxGames = cmd.ArgT<size_t>(0);
  size_t maxGames = cmd.ArgT<size_t>(1);
  size_t stepSize = cmd.ArgT<size_t>(2);
  string fileName = cmd.Arg(3);
  GoUctEstimatorStat::Compute(Search(), trueValueMaxGames, maxGames,
                              stepSize, fileName);
}

void GoUctCommands::CmdFinalScore(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GoPointSet deadStones = DoFinalStatusSearch();
  float score;
  if (!GoBoardUtil::ScorePosition(m_bd, deadStones, score))
    throw GtpFailure("cannot score");
  cmd << GoUtil::ScoreToString(score);
}

void GoUctCommands::CmdFinalStatusList(GtpCommand &cmd) {
  string arg = cmd.Arg();
  if (arg == "seki")
    return;
  bool getDead;
  if (arg == "alive")
    getDead = false;
  else if (arg == "dead")
    getDead = true;
  else
    throw GtpFailure("invalid final status argument");
  GoPointSet deadPoints = DoFinalStatusSearch();
  for (GoBlockIterator it(m_bd); it; ++it) {
    if ((getDead && deadPoints.Contains(*it))
        || (!getDead && !deadPoints.Contains(*it))) {
      for (GoBoard::StoneIterator it2(m_bd, *it); it2; ++it2)
        cmd << GoWritePoint(*it2) << ' ';
      cmd << '\n';
    }
  }
}

void GoUctCommands::CmdGfx(GtpCommand &cmd) {
  cmd.CheckArgNone();
  const GoUctSearch &s = Search();
  SgBlackWhite toPlay = s.ToPlay();
  GoUctUtil::GfxBestMove(s, toPlay, cmd.ResponseStream());
  GoUctUtil::GfxMoveValues(s, toPlay, cmd.ResponseStream());
  GoUctUtil::GfxCounts(s.Tree(), cmd.ResponseStream());
  GoUctUtil::GfxStatus(s, cmd.ResponseStream());
}

void GoUctCommands::CmdIsPolicyCorrectedMove(GtpCommand &cmd) {
  CompareMove(cmd, GOUCT_COMPAREMOVE_CORRECTED);
}

void GoUctCommands::CmdIsPolicyMove(GtpCommand &cmd) {
  CompareMove(cmd, GOUCT_COMPAREMOVE_POLICY);
}

void GoUctCommands::CmdMaxMemory(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(1);
  if (cmd.NuArg() == 0)
    cmd << Search().MaxNodes() * 2 * sizeof(UctNode);
  else {
    if (SgDeterministic::IsDeterministicMode())
      throw GtpFailure() << "Command is blocked in deterministic mode.";

    std::size_t memory = cmd.ArgMin<size_t>(0, 2 * sizeof(UctNode));
    Search().SetMaxNodes(memory / 2 / sizeof(UctNode));
  }
}

void GoUctCommands::CmdMoves(GtpCommand &cmd) {
  cmd.CheckArgNone();
  std::vector<UctMoveInfo> moves;
  Search().GenerateAllMoves(moves);
  for (std::size_t i = 0; i < moves.size(); ++i)
    cmd << GoWritePoint(moves[i].uct_move) << ' ';
  cmd << '\n';
}

void GoUctCommands::CmdParamGlobalSearch(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>,
                    GoUctPlayoutPolicyFactory<GoUctBoard> > &search = GlobalSearch();
  GoUctGlobalSearchStateParam &p = search.m_param;
  if (cmd.NuArg() == 0) {
    cmd << "[bool] live_gfx " << search.GlobalSearchLiveGfx() << '\n'
        << "[bool] mercy_rule " << p.m_mercyRule << '\n'
        << "[bool] territory_statistics " << p.m_territoryStatistics
        << '\n'
        << "[bool] use_default_prior_knowledge "
        << p.m_useDefaultPriorKnowledge << '\n'
        << "[bool] use_tree_filter " << p.m_useTreeFilter << '\n'
        << "[float] default_prior_weight " << p.m_defaultPriorWeight
        << '\n'
        << "[float] additive_knowledge_scale "
        << p.m_additiveKnowledgeScale << '\n'
        << "[string] length_modification " << p.m_lengthModification
        << '\n'
        << "[string] score_modification " << p.m_scoreModification
        << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (name == "live_gfx")
      search.SetGlobalSearchLiveGfx(cmd.ArgT<bool>(1));
    else if (name == "mercy_rule")
      p.m_mercyRule = cmd.ArgT<bool>(1);
    else if (name == "territory_statistics")
      p.m_territoryStatistics = cmd.ArgT<bool>(1);
    else if (name == "use_default_prior_knowledge")
      p.m_useDefaultPriorKnowledge = cmd.ArgT<bool>(1);
    else if (name == "use_tree_filter")
      p.m_useTreeFilter = cmd.ArgT<bool>(1);
    else if (name == "default_prior_weight")
      p.m_defaultPriorWeight = cmd.ArgT<float>(1);
    else if (name == "additive_knowledge_scale")
      p.m_additiveKnowledgeScale = cmd.ArgT<float>(1);
    else if (name == "length_modification")
      p.m_lengthModification = cmd.ArgT<UctValueType>(1);
    else if (name == "score_modification")
      p.m_scoreModification = cmd.ArgT<UctValueType>(1);
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamPlayer(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctPlayerType &p = Player();
  if (cmd.NuArg() == 0) {
    cmd << "[bool] auto_param " << p.AutoParam() << '\n'
        << "[bool] early_pass " << p.EarlyPass() << '\n'
        << "[bool] forced_opening_moves " << p.ForcedOpeningMoves() << '\n'
        << "[bool] ignore_clock " << p.IgnoreClock() << '\n'
        << "[bool] ponder " << p.EnablePonder() << '\n'
        << "[bool] reuse_subtree " << p.ReuseSubtree() << '\n'
        << "[bool] use_root_filter " << p.UseRootFilter() << '\n'
        << "[string] max_games " << p.MaxGames() << '\n'
        << "[string] max_ponder_time " << p.MaxPonderTime() << '\n'
        << "[string] resign_min_games " << p.ResignMinGames() << '\n'
        << "[string] resign_threshold " << p.ResignThreshold() << '\n'
        << "[list/playout_policy/uct/one_ply] search_mode "
        << SearchModeToString(p.SearchMode()) << '\n';
  } else if (cmd.NuArg() >= 1 && cmd.NuArg() <= 2) {
    string name = cmd.Arg(0);
    if (SgDeterministic::IsDeterministicMode()
        && IsBlockedInDeterministicMode(name)
        )
      throw GtpFailure() << "Command "
                         << name << " is blocked in deterministic mode.";

    if (name == "auto_param")
      p.SetAutoParam(cmd.ArgT<bool>(1));
    else if (name == "early_pass")
      p.SetEarlyPass(cmd.ArgT<bool>(1));
    else if (name == "forced_opening_moves")
      p.SetForcedOpeningMoves(cmd.ArgT<bool>(1));
    else if (name == "ignore_clock")
      p.SetIgnoreClock(cmd.ArgT<bool>(1));
    else if (name == "ponder")
      p.SetEnablePonder(cmd.ArgT<bool>(1));
    else if (name == "reuse_subtree")
      p.SetReuseSubtree(cmd.ArgT<bool>(1));
    else if (name == "use_root_filter")
      p.SetUseRootFilter(cmd.ArgT<bool>(1));
    else if (name == "max_games")
      p.SetMaxGames(cmd.ArgMin<UctValueType>(1, UctValueType(0)));
    else if (name == "max_ponder_time")
      p.SetMaxPonderTime(cmd.ArgMin<UctValueType>(1, 0));
    else if (name == "resign_min_games")
      p.SetResignMinGames(cmd.ArgMin<UctValueType>(1, UctValueType(0)));
    else if (name == "resign_threshold")
      p.SetResignThreshold(cmd.ArgMinMax<UctValueType>(1, 0, 1));
    else if (name == "search_mode")
      p.SetSearchMode(SearchModeArg(cmd, 1));
    else
      throw GtpFailure() << "unknown parameter: " << name;

    if (SgDeterministic::IsDeterministicMode())
      SgWarning() << "changing " << name
                  << " in deterministic mode can affect results\n";
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamDeepPlayer(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctPlayerType &p = Player();
  if (cmd.NuArg() == 0) {
    cmd << "[bool] auto_param " << p.AutoParam() << '\n'
        << "[bool] early_pass " << p.EarlyPass() << '\n'
        << "[bool] forced_opening_moves " << p.ForcedOpeningMoves() << '\n'
        << "[bool] ignore_clock " << p.IgnoreClock() << '\n'
        << "[bool] ponder " << p.EnablePonder() << '\n'
        << "[bool] reuse_subtree " << p.ReuseSubtree() << '\n'
        << "[bool] use_root_filter " << p.UseRootFilter() << '\n'
        << "[string] max_games " << p.MaxGames() << '\n'
        << "[string] max_ponder_time " << p.MaxPonderTime() << '\n'
        << "[string] resign_min_games " << p.ResignMinGames() << '\n'
        << "[string] resign_threshold " << p.ResignThreshold() << '\n'
        << "[list/playout_policy/uct/one_ply] search_mode "
        << SearchModeToString(p.SearchMode()) << '\n';
  } else if (cmd.NuArg() >= 1 && cmd.NuArg() <= 2) {
    string name = cmd.Arg(0);
    if (SgDeterministic::IsDeterministicMode()
        && IsBlockedInDeterministicMode(name)
        )
      throw GtpFailure() << "Command "
                         << name << " is blocked in deterministic mode.";

    if (name == "auto_param")
      p.SetAutoParam(cmd.ArgT<bool>(1));
    else if (name == "early_pass")
      p.SetEarlyPass(cmd.ArgT<bool>(1));
    else if (name == "forced_opening_moves")
      p.SetForcedOpeningMoves(cmd.ArgT<bool>(1));
    else if (name == "ignore_clock")
      p.SetIgnoreClock(cmd.ArgT<bool>(1));
    else if (name == "ponder")
      p.SetEnablePonder(cmd.ArgT<bool>(1));
    else if (name == "reuse_subtree")
      p.SetReuseSubtree(cmd.ArgT<bool>(1));
    else if (name == "use_root_filter")
      p.SetUseRootFilter(cmd.ArgT<bool>(1));
    else if (name == "max_games")
      p.SetMaxGames(cmd.ArgMin<UctValueType>(1, UctValueType(0)));
    else if (name == "max_ponder_time")
      p.SetMaxPonderTime(cmd.ArgMin<UctValueType>(1, 0));
    else if (name == "resign_min_games")
      p.SetResignMinGames(cmd.ArgMin<UctValueType>(1, UctValueType(0)));
    else if (name == "resign_threshold")
      p.SetResignThreshold(cmd.ArgMinMax<UctValueType>(1, 0, 1));
    else if (name == "search_mode")
      p.SetSearchMode(SearchModeArg(cmd, 1));
    else
      throw GtpFailure() << "unknown parameter: " << name;

    if (SgDeterministic::IsDeterministicMode())
      SgWarning() << "changing " << name
                  << " in deterministic mode can affect results\n";
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamPolicy(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctPlayoutPolicyParam &p = Player().m_playoutPolicyParam;
  if (cmd.NuArg() == 0) {
    cmd << "[bool] nakade_heuristic " << p.m_useNakadeHeuristic << '\n'
        << "[bool] statistics_enabled " << p.m_statisticsEnabled << '\n'
        << "[bool] use_patterns_in_playout "
        << p.m_usePatternsInPlayout << '\n'
        << "[bool] use_patterns_in_prior_knowledge "
        << p.m_usePatternsInPriorKnowledge << '\n'
        << "[int] fillboard_tries " << p.m_fillboardTries << '\n'
        << "[list/none/greenpeep/rulebased/features/both] knowledge_type "
        << "[list/multiply/geometric_mean/add/average/max] "
            "combination_type "
        << "[float] pattern_gamma_threshold "
        << p.m_patternGammaThreshold << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (name == "nakade_heuristic")
      p.m_useNakadeHeuristic = cmd.ArgT<bool>(1);
    else if (name == "statistics_enabled")
      p.m_statisticsEnabled = cmd.ArgT<bool>(1);
    else if (name == "use_patterns_in_playout")
      p.m_usePatternsInPlayout = cmd.ArgT<bool>(1);
    else if (name == "use_patterns_in_prior_knowledge")
      p.m_usePatternsInPriorKnowledge = cmd.ArgT<bool>(1);
    else if (name == "fillboard_tries") {
      p.m_fillboardTries = cmd.ArgT<int>(1);
    } else if (name == "pattern_gamma_threshold")
      p.m_patternGammaThreshold = cmd.ArgT<float>(1);
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamRootFilter(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctDefaultMoveFilterParam &p = Player().m_rootFilterParam;
  if (cmd.NuArg() == 0) {
    cmd << "[bool] check_ladders " << p.CheckLadders() << '\n';
    cmd << "[bool] check_offensive_ladders "
        << p.CheckOffensiveLadders() << '\n';
    cmd << "[bool] check_safety " << p.CheckSafety() << '\n';
    cmd << "[bool] filter_first_line " << p.FilterFirstLine() << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (name == "check_ladders")
      p.SetCheckLadders(cmd.ArgT<bool>(1));
    else if (name == "check_offensive_ladders")
      p.SetCheckOffensiveLadders(cmd.ArgT<bool>(1));
    else if (name == "check_safety")
      p.SetCheckSafety(cmd.ArgT<bool>(1));
    else if (name == "filter_first_line")
      p.SetFilterFirstLine(cmd.ArgT<bool>(1));
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamTreeFilter(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctDefaultMoveFilterParam &p = Player().m_treeFilterParam;
  if (cmd.NuArg() == 0) {
    cmd << "[bool] check_ladders " << p.CheckLadders() << '\n';
    cmd << "[bool] check_offensive_ladders "
        << p.CheckOffensiveLadders() << '\n';
    cmd << "[bool] check_safety " << p.CheckSafety() << '\n';
    cmd << "[bool] filter_first_line " << p.FilterFirstLine() << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (name == "check_ladders")
      p.SetCheckLadders(cmd.ArgT<bool>(1));
    else if (name == "check_offensive_ladders")
      p.SetCheckOffensiveLadders(cmd.ArgT<bool>(1));
    else if (name == "check_safety")
      p.SetCheckSafety(cmd.ArgT<bool>(1));
    else if (name == "filter_first_line")
      p.SetFilterFirstLine(cmd.ArgT<bool>(1));
    else
      throw GtpFailure() << "unknown parameter: " << name;
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdParamSearch(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(2);
  GoUctSearch &s = Search();
  if (cmd.NuArg() == 0) {
    cmd << "[bool] check_float_precision " << s.CheckFloatPrecision()
        << '\n'
        << "[bool] keep_games " << s.KeepGames() << '\n'
        << "[bool] lock_free " << s.LockFree() << '\n'
        << "[bool] log_games " << s.LogGames() << '\n'
        << "[bool] prune_full_tree " << s.PruneFullTree() << '\n'
        << "[bool] rave " << s.Rave() << '\n'
        << "[bool] update_multiple_playouts_as_single " << s.UpdateMultiplePlayoutsAsSingle() << '\n'
        << "[bool] use_virtual_loss " << s.VirtualLoss() << '\n'
        << "[bool] weight_rave_updates " << s.WeightRaveUpdates() << '\n'
        << "[string] bias_term_constant " << s.BiasTermConstant() << '\n'
        << "[string] bias_term_frequency " << s.BiasTermFrequency() << '\n'
        << "[string] bias_term_depth " << s.BiasTermDepth() << '\n'
        << "[string] expand_threshold " << s.ExpandThreshold() << '\n'
        << "[string] first_play_urgency " << s.FirstPlayUrgency() << '\n'
        << "[list/none/counts/sequence] live_gfx " << LiveGfxToString(s.LiveGfx()) << '\n'
        << "[string] live_gfx_interval " << s.LiveGfxInterval() << '\n'
        << "[string] max_nodes " << s.MaxNodes() << '\n'
        << "[list/value/count/bound/estimate] move_select " << SgGtpUtil::MoveSelectToString(s.MoveSelect()) << '\n'
        << "[string] number_threads " << s.NumberThreads() << '\n'
        << "[string] number_playouts " << s.NumberPlayouts() << '\n'
        << "[string] prune_min_count " << s.PruneMinCount() << '\n'
        << "[string] randomize_rave_frequency " << s.RandomizeRaveFrequency() << '\n'
        << "[string] rave_weight_final " << s.RaveWeightFinal() << '\n'
        << "[string] rave_weight_initial " << s.RaveWeightInitial() << '\n';
  } else if (cmd.NuArg() == 2) {
    string name = cmd.Arg(0);
    if (SgDeterministic::IsDeterministicMode()
        && IsBlockedInDeterministicMode(name)
        )
      throw GtpFailure() << "Command "
                         << name << " is blocked in deterministic mode.";

    if (name == "bias_term_constant")
      s.SetBiasTermConstant(cmd.ArgT<float>(1));
    else if (name == "bias_term_frequency")
      s.SetBiasTermFrequency(cmd.ArgT<int>(1));
    else if (name == "bias_term_depth")
      s.SetBiasTermDepth(cmd.ArgT<size_t>(1));
    else if (name == "check_float_precision")
      s.SetCheckFloatPrecision(cmd.ArgT<bool>(1));
    else if (name == "expand_threshold")
      s.SetExpandThreshold(cmd.ArgMin<UctValueType>(1, 0));
    else if (name == "first_play_urgency")
      s.SetFirstPlayUrgency(cmd.ArgT<UctValueType>(1));
    else if (name == "keep_games")
      s.SetKeepGames(cmd.ArgT<bool>(1));
    else if (name == "live_gfx")
      s.SetLiveGfx(LiveGfxArg(cmd, 1));
    else if (name == "live_gfx_interval")
      s.SetLiveGfxInterval(cmd.ArgMin<UctValueType>(1, 1));
    else if (name == "lock_free")
      s.SetLockFree(cmd.ArgT<bool>(1));
    else if (name == "log_games")
      s.SetLogGames(cmd.ArgT<bool>(1));
    else if (name == "max_nodes")
      s.SetMaxNodes(cmd.ArgMin<size_t>(1, 1));
    else if (name == "move_select")
      s.SetMoveSelect(SgGtpUtil::MoveSelectArg(cmd, 1));
    else if (name == "number_threads")
      s.SetNumberThreads(cmd.ArgMin<size_t>(1, 1));
    else if (name == "number_playouts")
      s.SetNumberPlayouts(cmd.ArgMin<int>(1, 1));
    else if (name == "prune_full_tree")
      s.SetPruneFullTree(cmd.ArgT<bool>(1));
    else if (name == "prune_min_count")
      s.SetPruneMinCount(cmd.ArgMin<UctValueType>(1, UctValueType(1)));
    else if (name == "randomize_rave_frequency")
      s.SetRandomizeRaveFrequency(cmd.ArgMin<int>(1, 0));
    else if (name == "rave")
      s.SetRave(cmd.ArgT<bool>(1));
    else if (name == "rave_weight_final")
      s.SetRaveWeightFinal(cmd.ArgT<float>(1));
    else if (name == "rave_weight_initial")
      s.SetRaveWeightInitial(cmd.ArgT<float>(1));
    else if (name == "update_multiple_playouts_as_single")
      s.SetUpdateMultiplePlayoutsAsSingle(cmd.ArgT<bool>(1));
    else if (name == "use_virtual_loss")
      s.SetVirtualLoss(cmd.ArgT<bool>(1));
    else if (name == "weight_rave_updates")
      s.SetWeightRaveUpdates(cmd.ArgT<bool>(1));
    else
      throw GtpFailure() << "unknown parameter: " << name;

    if (SgDeterministic::IsDeterministicMode())
      SgWarning() << "changing " << name
                  << " in deterministic mode can affect results\n";
  } else
    throw GtpFailure() << "need 0 or 2 arguments";
}

void GoUctCommands::CmdPolicyCorrectedMoves(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GoUctPlayoutPolicy<GoBoard> policy(m_bd, Player().m_playoutPolicyParam);
  for (GoBoard::Iterator it(m_bd); it; ++it)
    if (m_bd.IsLegal(*it)) {
      GoPoint move = *it;
      policy.CorrectMove(GoUctUtil::DoFalseEyeToCaptureCorrection, move,
                         GOUCT_REPLACE_CAPTURE);
      if (move != *it)
        cmd << "FalseEyeToCaptureCorrection "
            << GoWritePoint(*it) << ' '
            << GoWritePoint(move) << ' ';
      move = *it;
      policy.CorrectMove(GoUctUtil::DoSelfAtariCorrection, move,
                         GOUCT_SELFATARI_CORRECTION);
      if (move != *it)
        cmd << "SelfAtariCorrection "
            << GoWritePoint(*it) << ' '
            << GoWritePoint(move) << ' ';
    }
}

void GoUctCommands::WritePolicyMoves(GtpCommand &cmd, bool writeGammas) {
  cmd.CheckArgNone();
  GoUctPlayoutPolicy<GoBoard> policy(m_bd, Player().m_playoutPolicyParam);
  policy.StartPlayout();
  policy.GenerateMove();
  cmd << GoUctPlayoutPolicyTypeStr(policy.MoveType());
  GoPointList moves = policy.GetEquivalentBestMoves();
  moves.Sort();
  for (int i = 0; i < moves.Length(); ++i)
    cmd << ' ' << GoWritePoint(moves[i]);
  if (policy.MoveType() == GOUCT_GAMMA_PATTERN) {
    cmd << ' ';
    policy.GammaGenerator().WriteMovesAndGammas(cmd.ResponseStream(), writeGammas);
  }
}

void GoUctCommands::CmdPolicyMoves(GtpCommand &cmd) {
  WritePolicyMoves(cmd, true);
}

void GoUctCommands::CmdPolicyMovesSimple(GtpCommand &cmd) {
  WritePolicyMoves(cmd, false);
}

void GoUctCommands::CmdPriorKnowledge(GtpCommand &cmd) {
  DisplayKnowledge(cmd, false);
}

namespace {

bool CompareValue(const UctNode *p1, const UctNode *p2) {
  return p1->PosCount() > p2->PosCount();
}

void OsWriteTopNMoves(std::ostringstream &stream, const UctSearchTree &tree,
                      const UctNode &node) {
  if (!node.HasChildren())
    return;
  std::vector<const UctNode *> sorted;
  for (UctChildNodeIterator it(tree, node); it; ++it)
    sorted.push_back(&(*it));
  std::sort(sorted.begin(), sorted.end(), CompareValue);
  const int n = std::min(6, static_cast<int>(sorted.size()));
  stream << "Top " << n << " moves:\n";
  for (int i = 0; i < n; ++i) {
    stream << i + 1 << ". " << *sorted[i] << '\n';
  }
}

void WriteTopNMoves(GtpCommand &command, const UctSearchTree &tree,
                    const UctNode &node) {
  OsWriteTopNMoves(command.ResponseStream(), tree, node);
}
}

void GoUctCommands::CmdNodeInfo(GtpCommand &cmd) {
  cmd.CheckArgNone();
  const GoUctSearch &search = Search();
  const UctSearchTree &tree = search.Tree();
  std::vector<GoPoint> sequence;
  if (search.BoardHistory().SequenceToCurrent(m_bd, sequence)) {
    const UctNode *node =
        UctTreeUtil::FindMatchingNode(tree, sequence);
    cmd << *node;
    WriteTopNMoves(cmd, tree, *node);
  } else
    cmd << "Current position not in tree";
}

void GoUctCommands::CmdRaveValues(GtpCommand &cmd) {
  cmd.CheckArgNone();
  const GoUctSearch &search = Search();
  if (!search.Rave())
    throw GtpFailure("RAVE not enabled");
  GoPointArray<string> array("\"\"");
  const UctSearchTree &tree = search.Tree();
  for (UctChildNodeIterator it(tree, tree.Root()); it; ++it) {
    const UctNode &child = *it;
    GoPoint p = child.Move();
    if (p == GO_PASS || !child.HasRaveValue())
      continue;
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << child.RaveValue();
    array[p] = out.str();
  }
  cmd << '\n'
      << SgWritePointArray<string>(array, m_bd.Size());
}

void GoUctCommands::CmdRootFilter(GtpCommand &cmd) {
  cmd.CheckArgNone();
  cmd << SgWritePointList(Player().RootFilter().Get(), "", false);
}

void GoUctCommands::CmdSaveTree(GtpCommand &cmd) {
  if (Search().GetMpiSynchronizer()->IsRootProcess()) {
    cmd.CheckNuArgLessEqual(2);
    string fileName = cmd.Arg(0);
    int maxDepth = -1;
    if (cmd.NuArg() == 2)
      maxDepth = cmd.ArgMin<int>(1, 0);
    std::ofstream out(fileName.c_str());
    if (!out)
      throw GtpFailure() << "Could not open " << fileName;
    Search().SaveTree(out, maxDepth);
  }
}

void GoUctCommands::CmdSaveGames(GtpCommand &cmd) {
  string fileName = cmd.Arg();
  try {
    Search().SaveGames(fileName);
  }
  catch (const SgException &e) {
    throw GtpFailure(e.what());
  }
}

void GoUctCommands::CmdScore(GtpCommand &cmd) {
  cmd.CheckArgNone();
  try {
    float komi = m_bd.Rules().Komi().ToFloat();
    cmd << GoBoardUtil::ScoreSimpleEndPosition(m_bd, komi);
  }
  catch (const SgException &e) {
    throw GtpFailure(e.what());
  }
}

void GoUctCommands::CmdSequence(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GoUctUtil::GfxSequence(Search(), Search().ToPlay(), cmd.ResponseStream());
}

void GoUctCommands::CmdStatPlayer(GtpCommand &cmd) {
  cmd.CheckArgNone();
  Player().GetStatistics().Write(cmd.ResponseStream());
}

void GoUctCommands::CmdStatPlayerClear(GtpCommand &cmd) {
  cmd.CheckArgNone();
  Player().ClearStatistics();
}

void GoUctCommands::CmdStatPolicy(GtpCommand &cmd) {
  cmd.CheckArgNone();
  if (!Player().m_playoutPolicyParam.m_statisticsEnabled)
    SgWarning() << "statistics not enabled in policy parameters\n";
  cmd << "Black Statistics:\n";
  Policy(0).Statistics(SG_BLACK).Write(cmd.ResponseStream());
  cmd << "\nWhite Statistics:\n";
  Policy(0).Statistics(SG_WHITE).Write(cmd.ResponseStream());
}

void GoUctCommands::CmdStatPolicyClear(GtpCommand &cmd) {
  cmd.CheckArgNone();
  Policy(0).ClearStatistics();
}

void GoUctCommands::CmdStatSearch(GtpCommand &cmd) {
  cmd.CheckNuArgLessEqual(1);
  const GoUctSearch &search = Search();
  UctTreeStatistics treeStatistics;
  treeStatistics.Compute(search.Tree());
  if (cmd.NuArg() == 0) {
    cmd << "SearchStatistics:\n";
    search.WriteStatistics(cmd.ResponseStream());
    cmd << "TreeStatistics:\n"
        << treeStatistics;
  } else {
    string name = cmd.Arg(0);
    if (name == "count")
      cmd << search.Tree().Root().MoveCount() << '\n';
    else if (name == "games_played")
      cmd << search.GamesPlayed() << '\n';
    else if (name == "nodes")
      cmd << search.Tree().NuNodes() << '\n';
    else
      throw GtpFailure() << "unknown parameter: " << name;
  }
}

namespace {

UctValueType MapMeanToTerritoryEstimate(UctValueType mean) {
  return mean * 2 - 1;
}

UctValueType MapMeanToLikelyTerritory(UctValueType mean) {
  const UctValueType threshold = 0.25f;
  return mean < threshold ? -1
                          : mean > 1 - threshold ? 1
                                                 : 0;
}

}

UctValueType GoUctCommands::DisplayTerritory(GtpCommand &cmd,
                                             MeanMapperFunction f) {
  cmd.CheckArgNone();
  GoPointArray<UctStatistics> territoryStatistics
      = ThreadState(0).m_territoryStatistics;
  GoPointArray<UctValueType> array(0);
  UctValueType sum = UctValueType(0);
  for (GoBoard::Iterator it(m_bd); it; ++it) {
    if (territoryStatistics[*it].Count() == 0)
      throw GtpFailure("no statistics available: "
                           "enable them and run search first");
    array[*it] = f(territoryStatistics[*it].Mean());
    sum += array[*it];
  }
  cmd << '\n'
      << SgWritePointArrayFloat<UctValueType>(array, m_bd.Size(), true, 3);
  return sum;
}

void GoUctCommands::CmdStatTerritory(GtpCommand &cmd) {
  DisplayTerritory(cmd, MapMeanToTerritoryEstimate);
}

void GoUctCommands::CmdApproximateTerritory(GtpCommand &cmd) {
  UctValueType score =
      DisplayTerritory(cmd, MapMeanToLikelyTerritory);
  SgDebug() << "Score = " << score << '\n';
}

void GoUctCommands::CmdValue(GtpCommand &cmd) {
  cmd.CheckArgNone();
  cmd << Search().Tree().Root().Mean();
}

void GoUctCommands::CmdValueBlack(GtpCommand &cmd) {
  cmd.CheckArgNone();
  if (Search().Tree().Root().HasMean()) {
    UctValueType value = Search().Tree().Root().Mean();
    if (Search().ToPlay() == SG_WHITE)
      value = UctSearch::InverseEval(value);
    cmd << value;
  } else
    cmd << "There is no search history :(:\n";
}

void GoUctCommands::CmdStartTrainPipeline(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartTrainEvalPipeLine(true, EVAL_FROM_SERVER_MSG);
}

void GoUctCommands::CmdStartEvalCheckPoint(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartTrainEvalPipeLine(false);
}

void GoUctCommands::CmdStartEvalNoIPC(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartTrainEvalPipeLine(false, EVAL_FROM_SERVER);
}

void GoUctCommands::CmdStartEvalFromClient(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartTrainEvalPipeLine(false, EVAL_FROM_CLIENT);
}

void GoUctCommands::CmdStartSelfPlayAsClient(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartSelfPlay(true);
}

void GoUctCommands::CmdStartSelfPlayAsServer(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartSelfPlay(false);
}

void GoUctCommands::CmdStartStatServer(GtpCommand &cmd) {
  cmd.CheckArgNone();
   GetEvalStatSvr()->Start();
}

void GoUctCommands::CmdInitCheckPoint(GtpCommand &cmd) {
  cmd.CheckArgNone();
  GetDeepTrainer()->StartInitCK();
}

void GoUctCommands::CompareMove(GtpCommand &cmd, GoUctCompareMoveType type) {
  cmd.CheckArgNone();
  bool isCorrected = false;
  const GoPoint move = Game().CurrentMove();

  if (!SgIsSpecialMove(move)) {
    GoModBoard modBoard(m_bd);
    GoBoard &bd = modBoard.Board();
    DBG_ASSERT(bd.CanUndo());
    bd.Undo();
    GoUctPlayoutPolicy<GoBoard> policy(bd, Player().m_playoutPolicyParam);
    switch (type) {
      case GOUCT_COMPAREMOVE_CORRECTED:isCorrected = IsCorrected(policy, move);
        break;
      case GOUCT_COMPAREMOVE_POLICY:isCorrected = IsPolicyMove(policy, move);
        break;
      default:DBG_ASSERT(false);
        break;
    }
    bd.Play(move);
  }
  cmd << isCorrected;
}

void GoUctCommands::DisplayKnowledge(GtpCommand &cmd,
                                     bool additiveKnowledge) {
  cmd.CheckNuArgLessEqual(1);
  UctValueType count = 0;
  if (cmd.NuArg() == 1)
    count = UctValueType(cmd.ArgMin<size_t>(0, 0));
  GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > &state
      = ThreadState(0);
  state.StartSearch();
  std::vector<UctMoveInfo> moves;
  UctProvenType ignoreProvenType;
  state.GenerateAllMoves(count, moves, ignoreProvenType);
  DisplayMoveInfo(cmd, moves, additiveKnowledge);
}

void GoUctCommands::DisplayMoveInfo(GtpCommand &cmd,
                                    const std::vector<UctMoveInfo> &moves,
                                    bool additiveKnowledge) {
  cmd << "INFLUENCE ";
  for (size_t i = 0; i < moves.size(); ++i) {
    GoMove move = moves[i].uct_move;
    UctValueType value = additiveKnowledge ? moves[i].predictor_val
                                           : moves[i].uct_value;
    value = UctSearch::InverseEval(value);
    const bool show = additiveKnowledge || (moves[i].visit_count > 0);
    if (show) {
      UctValueType scaledValue = value * 2 - 1;
      if (m_bd.ToPlay() != SG_BLACK)
        scaledValue *= -1;
      cmd << ' ' << GoWritePoint(move) << ' ' << scaledValue;
    }
  }
  if (!additiveKnowledge) {
    cmd << "\nLABEL ";
    for (size_t i = 0; i < moves.size(); ++i) {
      GoMove move = moves[i].uct_move;
      const UctValueType count = moves[i].visit_count;
      if (count > 0)
        cmd << ' ' << GoWritePoint(move) << ' ' << count;
    }
  }
  cmd << '\n';
}

GoPointSet GoUctCommands::DoFinalStatusSearch() {
  GoPointSet deadStones;
  if (GoBoardUtil::TwoPasses(m_bd) && m_bd.Rules().CaptureDead())
    return deadStones;

  const UctValueType MAX_GAMES = 10000;
  SgDebug() << "GoUctCommands::DoFinalStatusSearch: doing a search with "
            << MAX_GAMES << " games to determine final status\n";
  GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>,
                    GoUctPlayoutPolicyFactory<GoUctBoard> > &
      search = GlobalSearch();
  SgRestorer<bool> restorer(&search.m_param.m_territoryStatistics);
  search.m_param.m_territoryStatistics = true;
  int nuUndoPass = 0;
  SgBlackWhite toPlay = m_bd.ToPlay();
  GoModBoard modBoard(m_bd);
  GoBoard &bd = modBoard.Board();
  while (bd.GetLastMove() == GO_PASS) {
    bd.Undo();
    toPlay = SgOppBW(toPlay);
    ++nuUndoPass;
  }
  m_player->UpdateSubscriber();
  if (nuUndoPass > 0)
    SgDebug() << "Undoing " << nuUndoPass << " passes\n";
  std::vector<GoMove> sequence;
  search.StartSearchThread(MAX_GAMES, std::numeric_limits<double>::max(), sequence);
  SgDebug() << SgWriteLabel("Sequence")
            << SgWritePointList(sequence, "", false);
  for (int i = 0; i < nuUndoPass; ++i) {
    bd.Play(GO_PASS, toPlay);
    toPlay = SgOppBW(toPlay);
  }
  m_player->UpdateSubscriber();

  GoPointArray<UctStatistics> territoryStatistics =
      ThreadState(0).m_territoryStatistics;
  GoSafetySolver safetySolver(bd);
  GoBWSet safe;
  safetySolver.FindSafePoints(&safe);
  for (GoBlockIterator it(bd); it; ++it) {
    SgBlackWhite c = bd.GetStone(*it);
    bool isDead = safe[SgOppBW(c)].Contains(*it);
    if (!isDead && !safe[c].Contains(*it)) {
      SgStatistics<UctValueType, int> averageStatus;
      for (GoBoard::StoneIterator it2(bd, *it); it2; ++it2) {
        if (territoryStatistics[*it2].Count() == 0)
          return deadStones;
        averageStatus.Add(territoryStatistics[*it2].Mean());
      }
      const float threshold = 0.3f;
      isDead =
          (c == SG_BLACK && averageStatus.Mean() < threshold)
              || (c == SG_WHITE && averageStatus.Mean() > 1 - threshold);
    }
    if (isDead)
      for (GoBoard::StoneIterator it2(bd, *it); it2; ++it2)
        deadStones.Include(*it2);
  }
  return deadStones;
}

GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>,
                  GoUctPlayoutPolicyFactory<GoUctBoard> > &
GoUctCommands::GlobalSearch() {
  return Player().GlobalSearch();
}

GoUctPlayerType &GoUctCommands::Player() {
  if (m_player == nullptr)
    throw GtpFailure("player not GoUctPlayer");
  try {
    return dynamic_cast<GoUctPlayerType &>(*m_player);
  }
  catch (const std::bad_cast &) {
    throw GtpFailure("player not GoUctPlayer");
  }
}

GoUctPlayoutPolicy<GoUctBoard> &
GoUctCommands::Policy(unsigned int threadId) {
  auto *policy = ThreadState(threadId).Policy();
  if (policy == nullptr)
    throw GtpFailure("player has no GoUctPlayoutPolicy");
  return *policy;
}

void GoUctCommands::Register(GtpEngine &e) {
  Register(e, "approximate_territory",
           &GoUctCommands::CmdApproximateTerritory);
  Register(e, "deterministic_mode", &GoUctCommands::CmdDeterministicMode);
  Register(e, "final_score", &GoUctCommands::CmdFinalScore);
  Register(e, "final_status_list", &GoUctCommands::CmdFinalStatusList);
  Register(e, "is_policy_corrected_move",
           &GoUctCommands::CmdIsPolicyCorrectedMove);
  Register(e, "is_policy_move",
           &GoUctCommands::CmdIsPolicyMove);
  Register(e, "uct_additive_knowledge",
           &GoUctCommands::CmdAdditiveKnowledge);
  Register(e, "uct_bounds", &GoUctCommands::CmdBounds);
  Register(e, "uct_estimator_stat", &GoUctCommands::CmdEstimatorStat);
  Register(e, "uct_gfx", &GoUctCommands::CmdGfx);
  Register(e, "uct_max_memory", &GoUctCommands::CmdMaxMemory);
  Register(e, "uct_moves", &GoUctCommands::CmdMoves);
  Register(e, "uct_node_info", &GoUctCommands::CmdNodeInfo);
  Register(e, "uct_param_globalsearch",
           &GoUctCommands::CmdParamGlobalSearch);
  Register(e, "uct_param_policy", &GoUctCommands::CmdParamPolicy);
  Register(e, "uct_param_player", &GoUctCommands::CmdParamPlayer);
  Register(e, "uct_param_rootfilter", &GoUctCommands::CmdParamRootFilter);
  Register(e, "uct_param_treefilter", &GoUctCommands::CmdParamTreeFilter);
  Register(e, "uct_param_search", &GoUctCommands::CmdParamSearch);
  Register(e, "uct_policy_corrected_moves",
           &GoUctCommands::CmdPolicyCorrectedMoves);
  Register(e, "uct_policy_moves", &GoUctCommands::CmdPolicyMoves);
  Register(e, "uct_policy_moves_simple",
           &GoUctCommands::CmdPolicyMovesSimple);
  Register(e, "uct_prior_knowledge", &GoUctCommands::CmdPriorKnowledge);
  Register(e, "uct_rave_values", &GoUctCommands::CmdRaveValues);
  Register(e, "uct_root_filter", &GoUctCommands::CmdRootFilter);
  Register(e, "uct_savegames", &GoUctCommands::CmdSaveGames);
  Register(e, "uct_savetree", &GoUctCommands::CmdSaveTree);
  Register(e, "uct_sequence", &GoUctCommands::CmdSequence);
  Register(e, "uct_score", &GoUctCommands::CmdScore);
  Register(e, "uct_stat_player", &GoUctCommands::CmdStatPlayer);
  Register(e, "uct_stat_player_clear", &GoUctCommands::CmdStatPlayerClear);
  Register(e, "uct_stat_policy", &GoUctCommands::CmdStatPolicy);
  Register(e, "uct_stat_policy_clear", &GoUctCommands::CmdStatPolicyClear);
  Register(e, "uct_stat_search", &GoUctCommands::CmdStatSearch);
  Register(e, "uct_stat_territory", &GoUctCommands::CmdStatTerritory);
  Register(e, "uct_value", &GoUctCommands::CmdValue);
  Register(e, "uct_value_black", &GoUctCommands::CmdValueBlack);
  Register(e, "uct_start_train", &GoUctCommands::CmdStartTrainPipeline);
  Register(e, "uct_self_train", &GoUctCommands::CmdStartTrainPipeline);
  Register(e, "deep-train", &GoUctCommands::CmdStartTrainPipeline);
  Register(e, "deeptrain", &GoUctCommands::CmdStartTrainPipeline);
  Register(e, "deep-evaluate", &GoUctCommands::CmdStartEvalCheckPoint);
  Register(e, "deep-eval", &GoUctCommands::CmdStartEvalCheckPoint);
  Register(e, "deep-eval-noipc", &GoUctCommands::CmdStartEvalNoIPC);
  Register(e, "deep-evalasclient", &GoUctCommands::CmdStartEvalFromClient);
  Register(e, "clienteval", &GoUctCommands::CmdStartEvalFromClient);
  Register(e, "init-checkpoint", &GoUctCommands::CmdInitCheckPoint);
  Register(e, "initck", &GoUctCommands::CmdInitCheckPoint);
  Register(e, "init-ck", &GoUctCommands::CmdInitCheckPoint);
  Register(e, "self-play", &GoUctCommands::CmdStartSelfPlayAsClient);
  Register(e, "selfplay", &GoUctCommands::CmdStartSelfPlayAsClient);
  Register(e, "selfplayAsServer", &GoUctCommands::CmdStartSelfPlayAsServer);
  Register(e, "selfplayserver", &GoUctCommands::CmdStartSelfPlayAsServer);
  Register(e, "statsevalresult", &GoUctCommands::CmdStartStatServer);
}

void GoUctCommands::Register(GtpEngine &engine, const std::string &command,
                             GtpCallback<GoUctCommands>::Method method) {
  engine.Register(command, new GtpCallback<GoUctCommands>(this, method));
}

UctDeepTrainer *GoUctCommands::GetDeepTrainer() {
  return &m_trainer;
}

UctEvalStatServer *GoUctCommands::GetEvalStatSvr() {
  return &m_statSvr;
}

GoUctSearch &GoUctCommands::Search() {
  try {
    auto &object =
        dynamic_cast<GoUctSearchObject &>(*m_player);
    return object.Search();
  }
  catch (const std::bad_cast &) {
    throw GtpFailure("player is not a GoUctSearchObject");
  }
}

GoUctSearch &GoUctCommands::Search(GoPlayer &player) {
  try {
    auto &object =
        dynamic_cast<GoUctSearchObject &>(player);
    return object.Search();
  }
  catch (const std::bad_cast &) {
    throw GtpFailure("player is not a GoUctSearchObject");
  }
}

GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > &
GoUctCommands::ThreadState(unsigned int threadId) {
  GoUctSearch &search = Search();
  if (!search.ThreadsCreated())
    search.CreateThreads();
  try {
    return dynamic_cast<
        GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > &>
    (search.ThreadState(threadId));
  }
  catch (const std::bad_cast &) {
    throw GtpFailure("player has no GoUctGlobalSearchState");
  }
}
