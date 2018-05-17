
#include <platform/SgSystem.h>
#include "GoUctPlayer.h"

template
class GoUctPlayer<GoUctGlobalSearch<GoUctPlayoutPolicy<GoUctBoard>, GoUctPlayoutPolicyFactory<GoUctBoard> >,
                  GoUctGlobalSearchState<GoUctPlayoutPolicy<GoUctBoard> > >;

template<class SEARCH, class THREADSTATE>
GoUctPlayer<SEARCH, THREADSTATE>::GoUctPlayer(const GoBoard &bd)
    : GoPlayer(bd),
      m_searchMode(GOUCT_SEARCHMODE_UCT),
      m_autoParam(true),
      m_forcedOpeningMoves(true),
      m_ignoreClock(false),
      m_enablePonder(false),
      m_useRootFilter(true),
      m_reuseSubtree(true),
      m_earlyPass(true),
      m_resignThreshold(0),
      m_sureWinThreshold(0.80f),
      m_lastBoardSize(-1),
      m_maxGames(std::numeric_limits<UctValueType>::max()),
      m_resignMinGames(5000),
      m_maxPonderTime(300),
      m_search(
          Board(),
          new GoUctPlayoutPolicyFactory<GoUctBoard>(m_playoutPolicyParam),
          m_playoutPolicyParam,
          m_treeFilterParam),
      m_timeControl(Board()),
      m_rootFilter(new GoUctDefaultMoveFilter(Board(), m_rootFilterParam)),
      m_mpiSynchronizer(NullMpiSynchronizer::Create()),
      m_writeDebugOutput(true) {
  SetDefaultParameters(Board().Size());
  m_search.SetMpiSynchronizer(m_mpiSynchronizer);
  m_treeFilterParam.SetCheckSafety(false);
}

template<class SEARCH, class THREADSTATE>
GoUctPlayer<SEARCH, THREADSTATE>::~GoUctPlayer() {}

inline UctValueType ValueForPlayer(UctValueType v, SgBlackWhite player) {
  if (player == SG_WHITE)
    return 1 - v;
  else
    return v;
}

inline bool HasNonControlledLib(const GoBoard &bd,
                                GoPoint block,
                                SgBlackWhite toPlay,
                                const TerrArray &territory,
                                UctValueType threshold) {
  DBG_ASSERT(bd.IsColor(block, toPlay));

  const UctValueType blockMean = ValueForPlayer(territory[block].Mean(),
                                                toPlay);
  if (blockMean < threshold)

    return false;

  for (GoBoard::LibertyIterator it(bd, block); it; ++it) {
    const UctValueType mean = ValueForPlayer(territory[*it].Mean(), toPlay);
    if (mean < threshold) {
      SgDebug() << "non-controlled liberty " << GoWritePoint(*it)
                << " of block " << GoWritePoint(block)
                << " mean " << territory[*it].Mean()
                << "\n";

      return true;
    }
  }
  return false;
}

inline bool AllowFillinMove(const GoBoard &bd, GoPoint move,
                            const TerrArray &territory, UctValueType threshold) {

  DBG_ASSERT(bd.IsEmpty(move));
  const SgBlackWhite toPlay = bd.ToPlay();
  for (GoNeighborBlockIterator it(bd, move, toPlay); it; ++it)
    if (HasNonControlledLib(bd, *it, toPlay, territory, threshold))
      return true;
  return false;
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::ClearStatistics() {
  m_statistics.Clear();
}

template<class SEARCH, class THREADSTATE>
bool GoUctPlayer<SEARCH, THREADSTATE>::DoEarlyPassSearch(UctValueType maxGames,
                                                         double maxTime,
                                                         GoPoint searchMove,
                                                         GoPoint &move) {
  SgDebug() << "GoUctPlayer: doing a search if early pass is possible\n";
  GoBoard &bd = Board();
  bd.Play(GO_PASS);
  bool winAfterPass = false;
  bool passWins = GoBoardUtil::TrompTaylorPassWins(bd, bd.ToPlay());
  m_mpiSynchronizer->SynchronizePassWins(passWins);
  if (passWins) {
    winAfterPass = false;
  } else {
    SgRestorer<bool> restorer1(&m_search.m_param.m_territoryStatistics);
    SgRestorer<bool> restorer2(&m_search.m_param.m_mercyRule);
    m_search.m_param.m_territoryStatistics = true;

    m_search.m_param.m_mercyRule = false;
    std::vector<GoPoint> sequence;
    UctValueType value = m_search.StartSearchThread(maxGames, maxTime, sequence);
    value = m_search.InverseEstimate(value);
    winAfterPass = (value > m_sureWinThreshold);
  }
  bd.Undo();

  bool earlyPassPossible = true;
  if (!winAfterPass) {
    SgDebug() << "GoUctPlayer: no early pass possible (no win)\n";
    earlyPassPossible = false;
  }
  move = GO_PASS;
  auto &threadState = dynamic_cast<THREADSTATE &>(m_search.ThreadState(0));
  const TerrArray territory = threadState.m_territoryStatistics;
  if (earlyPassPossible && !HasStatsForAllMoves(bd, territory)) {
    earlyPassPossible = false;
  }

  if (earlyPassPossible
      && searchMove != GO_PASS
      && AllowFillinMove(bd, searchMove, territory, m_sureWinThreshold)) {
    move = searchMove;
    SgDebug() << "GoUctPlayer: allow fill-in move "
              << GoWritePoint(move) << "\n";
    return true;
  }

  if (earlyPassPossible) {
    for (GoBoard::Iterator it(bd); it; ++it) {
      const UctValueType mean = territory[*it].Mean();
      if (mean > 1 - m_sureWinThreshold
          && mean < m_sureWinThreshold) {

        bool isSafeToPlayAdj = false;
        bool isSafeOppAdj = false;
        for (GoNbIterator it2(bd, *it); it2; ++it2) {
          const UctValueType nbMean = territory[*it2].Mean();
          if (nbMean > m_sureWinThreshold)
            isSafeToPlayAdj = true;
          if (nbMean < 1 - m_sureWinThreshold)
            isSafeOppAdj = true;
        }
        if (isSafeToPlayAdj && isSafeOppAdj) {
          if (bd.IsLegal(*it) && !GoBoardUtil::SelfAtari(bd, *it))
            move = *it;
          else {
            SgDebug() <<
                      "GoUctPlayer: no early pass possible"
                          " (neutral illegal or self-atari)\n";
            earlyPassPossible = false;
            break;
          }
        } else {
          SgDebug()
              << "GoUctPlayer: no early pass possible (unsafe point "
              << GoWritePoint(*it) << ")\n";
          earlyPassPossible = false;
          break;
        }
      }
    }
  }

  m_mpiSynchronizer->SynchronizeEarlyPassPossible(earlyPassPossible);
  if (!earlyPassPossible)
    return false;
  m_mpiSynchronizer->SynchronizeMove(move);
  if (move == GO_PASS)
    SgDebug() << "GoUctPlayer: early pass is possible\n";
  else if (VerifyNeutralMove(maxGames, maxTime, move))
    SgDebug() << "GoUctPlayer: generate play on neutral point\n";
  else {
    SgDebug() << "GoUctPlayer: neutral move failed to verify\n";
    return false;
  }
  return true;
}

template<class SEARCH, class THREADSTATE>
GoPoint GoUctPlayer<SEARCH, THREADSTATE>::DoSearch(SgBlackWhite toPlay,
                                                   double maxTime,
                                                   bool isDuringPondering) {
  UctSearchTree *initTree = nullptr;
  SgTimer timer;
  double timeInitTree = 0;
  if (m_reuseSubtree) {
    initTree = &m_search.GetTempTree();
    timeInitTree = -timer.GetTime();
    FindInitTree(*initTree, toPlay, maxTime);
    timeInitTree += timer.GetTime();
    if (isDuringPondering) {
      bool aborted = ForceAbort();
      m_mpiSynchronizer->SynchronizeUserAbort(aborted);
      if (aborted)
        return GO_NULLMOVE;
    }
  }
  std::vector<GoMove> rootFilter;
  double timeRootFilter = 0;
  if (m_useRootFilter) {
    timeRootFilter = -timer.GetTime();
    rootFilter = m_rootFilter->Get();
    timeRootFilter += timer.GetTime();
  }
  maxTime -= timer.GetTime();
  m_search.SetToPlay(toPlay);
  std::vector<GoPoint> sequence;
  UctEarlyAbortParam earlyAbort;
  earlyAbort.abort_threshold = m_sureWinThreshold;
  earlyAbort.min_searches_to_abort = m_resignMinGames;
  earlyAbort.reduction_factor = 3;
  UctValueType value = m_search.StartSearchThread(m_maxGames, maxTime, sequence, rootFilter,
                                                  initTree, &earlyAbort);
  bool wasEarlyAbort = m_search.WasEarlyAbort();
  UctValueType rootMoveCount = m_search.Tree().Root().MoveCount();
  m_mpiSynchronizer->SynchronizeSearchStatus(value, wasEarlyAbort, rootMoveCount);

  if (m_writeDebugOutput) {

    std::ostringstream out;
    m_search.WriteStatistics(out);
    out << SgWriteLabel("Value") << std::fixed << std::setprecision(2)
        << value << '\n' << SgWriteLabel("Sequence")
        << SgWritePointList(sequence, "", false);
    if (m_reuseSubtree)
      out << SgWriteLabel("TimeInitTree") << std::fixed
          << std::setprecision(2) << timeInitTree << '\n';
    if (m_useRootFilter)
      out << SgWriteLabel("TimeRootFilter") << std::fixed
          << std::setprecision(2) << timeRootFilter << '\n';
    SgDebug() << out.str();
  }

  if (value < m_resignThreshold
      && rootMoveCount > m_resignMinGames
      )
    return UCT_RESIGN;

  GoPoint move;
  if (sequence.empty())
    move = GO_PASS;
  else {
    move = *(sequence.begin());
    move = GoUctSearchUtil::TrompTaylorPassCheck(move, m_search);
  }

  if (m_earlyPass && (wasEarlyAbort || value > m_sureWinThreshold)) {
    maxTime -= timer.GetTime();
    GoPoint earlyPassMove;
    if (DoEarlyPassSearch(m_maxGames / earlyAbort.reduction_factor,
                          maxTime, move, earlyPassMove))
      move = earlyPassMove;
  }

  m_mpiSynchronizer->SynchronizeMove(move);
  return move;
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::FindInitTree(UctSearchTree &initTree,
                                                    SgBlackWhite toPlay,
                                                    double maxTime) {
  Board().SetToPlay(toPlay);
  std::vector<GoPoint> sequence;
  if (!m_search.BoardHistory().SequenceToCurrent(Board(), sequence)) {
    SgDebug() << "GoUctPlayer: No tree to reuse\n";
    return;
  }
  UctTreeUtil::ExtractSubtree(m_search.Tree(), initTree, sequence, true,
                              maxTime, m_search.PruneMinCount());
  const size_t initTreeNodes = initTree.NuNodes();
  const size_t oldTreeNodes = m_search.Tree().NuNodes();
  if (oldTreeNodes > 1 && initTreeNodes >= 1) {
    const float reuse = float(initTreeNodes) / float(oldTreeNodes);
    const int reusePercent = static_cast<int>(100 * reuse);
    SgDebug() << "GoUctPlayer: Reusing " << initTreeNodes
              << " nodes (" << reusePercent << "%)\n";

    m_statistics.m_reuse.Add(reuse);
  } else {
    SgDebug() << "GoUctPlayer: Subtree to reuse has 0 nodes\n";
    m_statistics.m_reuse.Add(0.f);
  }

  if (initTree.Root().HasChildren()) {
    for (UctChildNodeIterator it(initTree, initTree.Root()); it; ++it)
      if (!Board().IsLegal((*it).Move())) {
        SgWarning() <<
                    "GoUctPlayer: illegal move in root child of init tree\n";
        initTree.Clear();
        DBG_ASSERT(false);
      }
  }
}

template<class SEARCH, class THREADSTATE>
GoPoint GoUctPlayer<SEARCH, THREADSTATE>::GenMove(const SgTimeRecord &time,
                                                  SgBlackWhite toPlay) {
  ++m_statistics.m_nuGenMove;
  if (m_searchMode == GOUCT_SEARCHMODE_PLAYOUTPOLICY)
    return GenMovePlayoutPolicy(toPlay);
  const GoBoard &bd = Board();
  GoMove move = GO_NULLMOVE;
  if (m_forcedOpeningMoves) {
    move = GoUctUtil::GenForcedOpeningMove(bd);
#ifdef DEBUG_FORCE_OPENING_MOVE
    if (move != GO_NULLMOVE)
        SgDebug() << "GoUctPlayer: Forced opening move\n";
#endif
  }
  if (move == GO_NULLMOVE && GoBoardUtil::TrompTaylorPassWins(bd, toPlay)) {
    move = GO_PASS;
    SgDebug() << "GoUctPlayer: Pass wins (Tromp-Taylor rules)\n";
  }
  if (move == GO_NULLMOVE) {
    double maxTime;
    if (m_ignoreClock)
      maxTime = std::numeric_limits<double>::max();
    else
      maxTime = m_timeControl.TimeForCurrentMove(time,
                                                 !m_writeDebugOutput);
    DBG_ASSERT(m_searchMode == GOUCT_SEARCHMODE_UCT);
    move = DoSearch(toPlay, maxTime, false);
    m_statistics.m_gamesPerSecond.Add(m_search.Statistics().searches_per_second);
  }
  return move;
}

template<class SEARCH, class THREADSTATE>
GoMove GoUctPlayer<SEARCH, THREADSTATE>::GenMovePlayoutPolicy(SgBlackWhite toPlay) {
  GoBoard &bd = Board();
  GoBoardRestorer restorer(bd);
  bd.SetToPlay(toPlay);
  if (m_playoutPolicy.get() == 0)
    m_playoutPolicy.reset(
        new GoUctPlayoutPolicy<GoBoard>(bd, m_playoutPolicyParam));
  m_playoutPolicy->StartPlayout();
  GoPoint move = m_playoutPolicy->GenerateMove();
  m_playoutPolicy->EndPlayout();
  if (move == GO_NULLMOVE) {
    SgDebug() <<
              "GoUctPlayer: GoUctPlayoutPolicy generated GO_NULLMOVE\n";
    return GO_PASS;
  }
  return move;
}

template<class SEARCH, class THREADSTATE>
const typename GoUctPlayer<SEARCH, THREADSTATE>::Statistics &
GoUctPlayer<SEARCH, THREADSTATE>::GetStatistics() const {
  return m_statistics;
}

template<class SEARCH, class THREADSTATE>
std::string GoUctPlayer<SEARCH, THREADSTATE>::Name() const {
  return "GoUctPlayer";
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::OnBoardChange() {
  int size = Board().Size();
  if (m_autoParam && size != m_lastBoardSize) {
    SgDebug() << "GoUctPlayer: Setting default parameters for size "
              << size << '\n';
    SetDefaultParameters(size);
    m_search.SetDefaultParameters(size);
    m_lastBoardSize = size;
  }
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::Ponder() {
  const GoBoard &bd = Board();
  if (!m_enablePonder || m_searchMode != GOUCT_SEARCHMODE_UCT)
    return;
  if (!m_reuseSubtree) {

    SgWarning() << "Pondering needs reuse_subtree enabled.\n";
    return;
  }
  SgDebug() << "GoUctPlayer::Ponder: start\n";
  DoSearch(bd.ToPlay(), m_maxPonderTime, true);
  SgDebug() << "GoUctPlayer::Ponder: end\n";
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::SetDefaultParameters(int boardSize) {
  m_timeControl.SetFastOpenMoves(0);
  m_timeControl.SetMinTime(0);
  m_timeControl.SetRemainingConstant(0.5);
  if (boardSize < 15) {
    m_resignThreshold = UctValueType(0.05);
  } else {

    m_resignThreshold = UctValueType(0.08);
  }
}

template<class SEARCH, class THREADSTATE>
void GoUctPlayer<SEARCH, THREADSTATE>::SetReuseSubtree(bool enable) {
  m_reuseSubtree = enable;
}

template<class SEARCH, class THREADSTATE>
SgDefaultTimeControl &GoUctPlayer<SEARCH, THREADSTATE>::TimeControl() {
  return m_timeControl;
}

template<class SEARCH, class THREADSTATE>
const SgDefaultTimeControl &GoUctPlayer<SEARCH, THREADSTATE>::TimeControl() const {
  return m_timeControl;
}

template<class SEARCH, class THREADSTATE>
bool GoUctPlayer<SEARCH, THREADSTATE>::VerifyNeutralMove(UctValueType maxGames,
                                                         double maxTime,
                                                         GoPoint move) {
  GoBoard &bd = Board();
  bd.Play(move);
  std::vector<GoPoint> sequence;
  UctValueType value = m_search.StartSearchThread(maxGames, maxTime, sequence);
  value = m_search.InverseEstimate(value);
  bd.Undo();
  return value >= m_sureWinThreshold;
}
