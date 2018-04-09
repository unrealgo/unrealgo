
#include "platform/SgSystem.h"
#include "SgDfpnSearch.h"
#include "SgSearchTracer.h"

#include <cmath>
#include "platform/SgDebug.h"
#include "board/SgWrite.h"

namespace {


const bool USE_WIDENING = true;

inline SgEmptyBlackWhite Winner(bool isWinning, SgEmptyBlackWhite toPlay) {
  return isWinning ? toPlay : SgOppBW(toPlay);
}

}

void DfpnBounds::CheckConsistency() const {
#ifndef NDEBUG
  DBG_ASSERT(phi <= INFTY);
  DBG_ASSERT(delta <= INFTY);
  DBG_ASSERT(0 != phi || INFTY == delta);
  DBG_ASSERT(INFTY != phi || 0 == delta);
  DBG_ASSERT(0 != delta || INFTY == phi);
  DBG_ASSERT(INFTY != delta || 0 == phi);
#endif
}

DfpnChildren::DfpnChildren() {}

DfpnSolver::DfpnSolver()
    : m_hashTable(0),
      m_timelimit(0.0),
      m_wideningBase(1),
      m_wideningFactor(0.25f),
      m_epsilon(0.0f) {}

DfpnSolver::~DfpnSolver() {}

bool DfpnSolver::CheckAbort() {
  if (!m_aborted) {
    if (ForceAbort()) {
      m_aborted = true;
      SgDebug() << "DfpnSolver::CheckAbort(): Abort flag!\n";
    } else if (m_timelimit > 0) {
      if (m_checkTimerAbortCalls == 0) {
        double elapsed = m_timer.GetTime();
        if (elapsed > m_timelimit) {
          m_aborted = true;
          SgDebug() << "DfpnSolver::CheckAbort(): Timelimit!\n";
        } else if (m_numMIDcalls < 100)
          m_checkTimerAbortCalls = 10;
        else {
          size_t midsPerSec = static_cast<size_t>
          (double(m_numMIDcalls) / elapsed);
          m_checkTimerAbortCalls = midsPerSec / 2;
        }
      } else
        --m_checkTimerAbortCalls;
    }
  }
  return m_aborted;
}

void DfpnSolver::GetPVFromHash(PointSequence &pv) {
  int nuMoves = 0;
  for (;; ++nuMoves) {
    DfpnData data;
    if (!TTRead(data)
        || data.m_bestMove == GO_NULLMOVE
        )
      break;
    pv.push_back(data.m_bestMove);
    PlayMove(data.m_bestMove);
  }
  while (--nuMoves >= 0)
    UndoMove();
}

void DfpnSolver::LookupChildDataNonConst(GoMove move, DfpnData &data) {
  PlayMove(move);
  if (!TTRead(data)) {
    data.m_bounds.phi = 1;
    data.m_bounds.delta = 1;
    data.m_work = 0;
  }
  UndoMove();
}

void DfpnSolver::LookupChildData(GoMove move, DfpnData &data) const {
  DfpnSolver *solver = const_cast<DfpnSolver *>(this);
  solver->LookupChildDataNonConst(move, data);
}

void DfpnSolver::LookupData(DfpnData &data, const DfpnChildren &children,
                            std::size_t childIndex) const {
  const GoMove move = children.MoveAt(childIndex);
  LookupChildData(move, data);
}

size_t DfpnSolver::MID(const DfpnBounds &maxBounds, DfpnHistory &history) {
  maxBounds.CheckConsistency();
  DBG_ASSERT(maxBounds.phi > 1);
  DBG_ASSERT(maxBounds.delta > 1);

  ++m_numMIDcalls;
  size_t prevWork = 0;
  SgEmptyBlackWhite colorToMove = GetColorToMove();
  DfpnData data;
  if (TTRead(data)) {
    prevWork = data.m_work;
    if (!maxBounds.GreaterThan(data.m_bounds))
      return 0;
  } else {
    SgEmptyBlackWhite winner = SG_EMPTY;
    if (TerminalState(colorToMove, winner)) {
      ++m_numTerminal;
      DfpnBounds terminal;
      if (colorToMove == winner)
        DfpnBounds::SetToWinning(terminal);
      else {
        DBG_ASSERT(SgOppBW(colorToMove) == winner);
        DfpnBounds::SetToLosing(terminal);
      }
      TTWrite(DfpnData(terminal, GO_NULLMOVE, 1));
      return 1;
    }
  }

  ++m_generateMoves;
  DfpnChildren children;
  GenerateChildren(children.Children());
  std::vector<DfpnData> childrenData(children.Size());
  for (size_t i = 0; i < children.Size(); ++i)
    LookupData(childrenData[i], children, i);
  size_t maxChildIndex = ComputeMaxChildIndex(childrenData);
  SgHashCode currentHash = Hash();
  GoMove bestMove = GO_NULLMOVE;
  DfpnBounds currentBounds;
  size_t localWork = 1;
  do {
    UpdateBounds(currentBounds, childrenData, maxChildIndex);
    if (!maxBounds.GreaterThan(currentBounds))
      break;
    std::size_t bestIndex = 999999;
    DfpnBoundType delta2 = DfpnBounds::INFTY;
    SelectChild(bestIndex, delta2, childrenData, maxChildIndex);
    bestMove = children.MoveAt(bestIndex);
    const DfpnBounds childBounds(childrenData[bestIndex].m_bounds);
    DfpnBounds childMaxBounds;
    childMaxBounds.phi = maxBounds.delta
        - (currentBounds.delta - childBounds.phi);
    childMaxBounds.delta = delta2 == DfpnBounds::INFTY ? maxBounds.phi :
                           std::min(maxBounds.phi,
                                    std::max(delta2 + 1, DfpnBoundType(delta2 * (1.0 + m_epsilon))));
    DBG_ASSERT(childMaxBounds.GreaterThan(childBounds));
    if (delta2 != DfpnBounds::INFTY)
      m_deltaIncrease.Add(float(childMaxBounds.delta - childBounds.delta));
    PlayMove(bestMove);
    history.Push(bestMove, currentHash);
    localWork += MID(childMaxBounds, history);
    history.Pop();
    UndoMove();
    LookupData(childrenData[bestIndex], children, bestIndex);
    if (childrenData[bestIndex].m_bounds.IsLosing()) {
      m_moveOrderingIndex.Add(float(bestIndex));
      m_moveOrderingPercent.Add(float(bestIndex)
                                    / (float) childrenData.size());
      m_totalWastedWork += prevWork + localWork
          - childrenData[bestIndex].m_work;
    } else if (childrenData[bestIndex].m_bounds.IsWinning())
      maxChildIndex = ComputeMaxChildIndex(childrenData);

  } while (!CheckAbort());
  if (currentBounds.IsSolved()) {
    if (currentBounds.IsLosing()) {
      std::size_t maxWork = 0;
      for (std::size_t i = 0; i < children.Size(); ++i) {
        if (childrenData[i].m_work > maxWork) {
          maxWork = childrenData[i].m_work;
          bestMove = children.MoveAt(i);
        }
      }
    } else {
      std::size_t minWork = DfpnBounds::INFTY;
      for (std::size_t i = 0; i < children.Size(); ++i) {
        if (childrenData[i].m_bounds.IsLosing()
            && childrenData[i].m_work < minWork) {
          minWork = childrenData[i].m_work;
          bestMove = children.MoveAt(i);
        }
      }
    }
  }
  TTWrite(DfpnData(currentBounds, bestMove, localWork + prevWork));
  return localWork;
}

size_t DfpnSolver::ComputeMaxChildIndex(const std::vector<DfpnData> &
childrenData) const {
  if (USE_WIDENING) {
    DBG_ASSERT(!childrenData.empty());

    int numNonLosingChildren = 0;
    for (size_t i = 0; i < childrenData.size(); ++i)
      if (!childrenData[i].m_bounds.IsWinning())
        ++numNonLosingChildren;
    if (numNonLosingChildren < 2)
      return childrenData.size();
    int childrenToLookAt = WideningBase()
        + int(ceil(float(numNonLosingChildren) * WideningFactor()));
    DBG_ASSERT(childrenToLookAt >= 2);

    int numNonLosingSeen = 0;
    for (size_t i = 0; i < childrenData.size(); ++i) {
      if (!childrenData[i].m_bounds.IsWinning())
        if (++numNonLosingSeen == childrenToLookAt)
          return i + 1;
    }
  } else

  {
    for (size_t i = childrenData.size() - 1;; --i) {
      if (!childrenData[i].m_bounds.IsWinning())
        return i + 1;
      if (i == 0)
        break;
    }
  }
  return childrenData.size();
}

void DfpnSolver::PrintStatistics(SgEmptyBlackWhite winner,
                                 const PointSequence &pv) const {
  std::ostringstream os;
  os << '\n'
     << SgWriteLabel("MID calls") << m_numMIDcalls << '\n'
     << SgWriteLabel("Generate moves") << m_generateMoves << '\n'
     << SgWriteLabel("Terminal") << m_numTerminal << '\n'
     << SgWriteLabel("Work") << m_numMIDcalls + m_numTerminal << '\n'
     << SgWriteLabel("Wasted Work") << m_totalWastedWork
     << " (" << (double(m_totalWastedWork) * 100.0
      / double(m_numMIDcalls + m_numTerminal)) << "%)\n"
     << SgWriteLabel("Elapsed Time") << m_timer.GetTime() << '\n'
     << SgWriteLabel("MIDs/sec")
     << double(m_numMIDcalls) / m_timer.GetTime() << '\n'
     << SgWriteLabel("generates/sec")
     << double(m_generateMoves) / m_timer.GetTime() << '\n'
     << SgWriteLabel("Cnt prune sib") << m_prunedSiblingStats.Count() << '\n'
     << SgWriteLabel("Avg prune sib");
  m_prunedSiblingStats.Write(os);
  os << '\n' << SgWriteLabel("Move Index");
  m_moveOrderingIndex.Write(os);
  os << '\n' << SgWriteLabel("Move Percent");
  m_moveOrderingPercent.Write(os);
  os << '\n' << SgWriteLabel("Delta Increase");
  m_deltaIncrease.Write(os);
  os << '\n'
     << SgWriteLabel("Winner") << SgEBW(winner) << '\n';
  WriteMoveSequence(os, pv);
  os << '\n';
  if (m_hashTable)
    os << '\n' << *m_hashTable << '\n';
  SgDebug() << os.str();
}

void DfpnSolver::SelectChild(std::size_t &bestIndex, DfpnBoundType &delta2,
                             const std::vector<DfpnData> &childrenData,
                             size_t maxChildIndex) const {
  DfpnBoundType delta1 = DfpnBounds::INFTY;

  DBG_ASSERT(1 <= maxChildIndex && maxChildIndex <= childrenData.size());
  for (std::size_t i = 0; i < maxChildIndex; ++i) {
    const DfpnBounds &child = childrenData[i].m_bounds;
    if (child.delta < delta1) {
      delta2 = delta1;
      delta1 = child.delta;
      bestIndex = i;
    } else if (child.delta < delta2) {
      delta2 = child.delta;
    }
    if (child.IsLosing())
      break;
  }
  DBG_ASSERT(delta1 < DfpnBounds::INFTY);
}

SgEmptyBlackWhite DfpnSolver::StartSearch(DfpnHashTable &hashTable,
                                          PointSequence &pv) {
  return StartSearch(hashTable, pv,
                     DfpnBounds(DfpnBounds::MAX_WORK, DfpnBounds::MAX_WORK));
}

SgEmptyBlackWhite DfpnSolver::StartSearch(DfpnHashTable &hashTable,
                                          PointSequence &pv,
                                          const DfpnBounds &maxBounds) {
  m_aborted = false;
  m_hashTable = &hashTable;
  m_numTerminal = 0;
  m_numMIDcalls = 0;
  m_generateMoves = 0;
  m_totalWastedWork = 0;
  m_prunedSiblingStats.Clear();
  m_moveOrderingPercent.Clear();
  m_moveOrderingIndex.Clear();
  m_deltaIncrease.Clear();
  m_checkTimerAbortCalls = 0;
  DfpnData data;
  if (TTRead(data) && data.m_bounds.IsSolved()) {
    SgDebug() << "Already solved!\n";
    const SgEmptyBlackWhite toPlay = GetColorToMove();
    SgEmptyBlackWhite w = Winner(data.m_bounds.IsWinning(), toPlay);
    GetPVFromHash(pv);
    SgDebug() << SgEBW(w) << " wins!\n";
    WriteMoveSequence(SgDebug(), pv);
    return w;
  }

  m_timer.Start();
  DfpnHistory history;
  MID(maxBounds, history);
  m_timer.Stop();

  GetPVFromHash(pv);
  SgEmptyBlackWhite winner = SG_EMPTY;
  if (TTRead(data) && data.m_bounds.IsSolved()) {
    const SgEmptyBlackWhite toPlay = GetColorToMove();
    winner = Winner(data.m_bounds.IsWinning(), toPlay);
  }
  PrintStatistics(winner, pv);

  if (m_aborted)
    SgWarning() << "Search aborted.\n";
  return winner;
}

void DfpnSolver::UpdateBounds(DfpnBounds &bounds,
                              const std::vector<DfpnData> &childData,
                              size_t maxChildIndex) const {
  DfpnBounds boundsAll(DfpnBounds::INFTY, 0);
  DBG_ASSERT(1 <= maxChildIndex && maxChildIndex <= childData.size());
  for (std::size_t i = 0; i < childData.size(); ++i) {
    const DfpnBounds &childBounds = childData[i].m_bounds;
    if (childBounds.IsLosing()) {
      DfpnBounds::SetToWinning(bounds);
      return;
    }
    if (i < maxChildIndex)
      boundsAll.phi = std::min(boundsAll.phi, childBounds.delta);
    DBG_ASSERT(childBounds.phi != DfpnBounds::INFTY);
    boundsAll.delta += childBounds.phi;
  }
  bounds = boundsAll;
}

bool DfpnSolver::Validate(DfpnHashTable &hashTable, const SgBlackWhite winner,
                          SgSearchTracer &tracer) {
  DBG_ASSERT_BW(winner);
  DfpnData data;
  if (!TTRead(data)) {
    PointSequence pv;
    StartSearch(hashTable, pv);
    const bool wasRead = TTRead(data);
    SG_DEBUG_ONLY(wasRead);
    DBG_ASSERT(wasRead);
  }

  const bool orNode = (winner == GetColorToMove());
  if (orNode) {
    if (!data.m_bounds.IsWinning()) {
      SgWarning() << "OR not winning. DfpnData:" << data << std::endl;
      return false;
    }
  } else
  {
    if (!data.m_bounds.IsLosing()) {
      SgWarning() << "AND not losing. DfpnData:" << data << std::endl;
      return false;
    }
  }

  SgEmptyBlackWhite currentWinner;
  if (TerminalState(GetColorToMove(), currentWinner)) {
    if (winner == currentWinner)
      return true;
    else {
      SgWarning() << "winner disagreement: "
                  << SgEBW(winner) << ' ' << SgEBW(currentWinner)
                  << std::endl;
      return false;
    }
  }

  std::vector<GoMove> moves;
  if (orNode)
    moves.push_back(data.m_bestMove);
  else
    GenerateChildren(moves);
  for (std::vector<GoMove>::const_iterator it = moves.begin();
       it != moves.end(); ++it) {
    tracer.AddTraceNode(*it, GetColorToMove());
    PlayMove(*it);
    if (!Validate(hashTable, winner, tracer))
      return false;
    UndoMove();
    tracer.TakeBackTraceNode();
  }
  return true;
}

void DfpnSolver::WriteMove(std::ostream &stream, GoMove move) const {
  stream << "GoMove " << move;
}

