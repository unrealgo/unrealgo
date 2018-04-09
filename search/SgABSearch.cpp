
#include "platform/SgSystem.h"
#include "SgABSearch.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <math.h>
#include "platform/SgDebug.h"
#include "SgHashTable.h"
#include "SgMath.h"
#include "SgNode.h"
#include "SgProbCut.h"
#include "SgSearchControl.h"
#include "SgSearchValue.h"
#include "platform/SgTime.h"
#include "lib/SgVector.h"
#include "board/SgWrite.h"

using namespace std;

namespace {
const bool DEBUG_SEARCH = false;
const bool DEBUG_SEARCH_ITERATIONS = false;
}

void SgKiller::MarkKiller(GoMove killer) {
  if (killer == m_killer1)
    ++m_count1;
  else if (killer == m_killer2) {
    ++m_count2;
    if (m_count1 < m_count2) {
      swap(m_killer1, m_killer2);
      swap(m_count1, m_count2);
    }
  } else if (m_killer1 == GO_NULLMOVE) {
    m_killer1 = killer;
    m_count1 = 1;
  } else {
    m_killer2 = killer;
    m_count2 = 1;
  }
}

void SgKiller::Clear() {
  m_killer1 = GO_NULLMOVE;
  m_count1 = 0;
  m_killer2 = GO_NULLMOVE;
  m_count2 = 0;
}

bool SgSearchHashData::IsBetterThan(const SgSearchHashData &data) const {
  if (m_depth > data.m_depth)
    return true;
  if (m_depth < data.m_depth)
    return false;
  return (!m_isUpperBound && !m_isLowerBound)
      || (m_isLowerBound && data.m_isLowerBound && m_value > data.m_value)
      || (m_isUpperBound && data.m_isUpperBound && m_value < data.m_value);
}

namespace {


void ReverseCopyStack(const SgSearchStack &moveStack, SgVector<GoMove> &sequence) {
  sequence.Clear();
  for (int i = moveStack.Size() - 1; i >= 0; --i)
    sequence.PushBack(moveStack[i]);
}

void WriteSgSearchHashData(std::ostream &str, const SgABSearch &search,
                           const SgSearchHashData &data) {
  str << "move = " << search.MoveString(data.BestMove())
      << " exact = " << data.IsExactValue()
      << " value = " << data.Value()
      << '\n';
}

void WriteMoves(const SgABSearch &search, const SgVector<GoMove> &sequence) {
  for (SgVectorIterator<GoMove> it(sequence); it; ++it)
    SgDebug() << ' ' << search.MoveString(*it);
}

void PrintPV(const SgABSearch &search, int depth, int value,
             const SgVector<GoMove> &sequence,
             bool isExactValue) {
  SgDebug() << "Iteration d = " << depth
            << ", value = " << value
            << ", exact = " << isExactValue
            << ", sequence = ";
  WriteMoves(search, sequence);
  SgDebug() << '\n';
}

}

const int SgABSearch::SG_INFINITY = numeric_limits<int>::max();

SgABSearch::SgABSearch(SgSearchHashTable *hash)
    : m_hash(hash),
      m_tracer(0),
      m_currentDepth(0),
      m_useScout(false),
      m_useKillers(false),
      m_useOpponentBest(0),
      m_useNullMove(0),
      m_nullMoveDepth(2),
      m_aborted(false),
      m_foundNewBest(false),
      m_reachedDepthLimit(false),
      m_stat(),
      m_timerLevel(0),
      m_control(0),
      m_probcut(0),
      m_abortFrequency(1) {
  InitSearch();
}

SgABSearch::~SgABSearch() {}

void SgABSearch::CallGenerate(SgVector<GoMove> *moves, int depth) {
  Generate(moves, depth);
  if (DEBUG_SEARCH) {
    SgDebug() << "SgABSearch::CallGenerate: d=" << depth;
    WriteMoves(*this, *moves);
    SgDebug() << '\n';
  }
}

void SgABSearch::InitSearch(int startDepth) {
  m_currentDepth = startDepth;
  m_moveStack.Clear();
  m_moveStack.PushBack(GO_NULLMOVE);
  m_moveStack.PushBack(GO_NULLMOVE);
  if (m_useKillers) {
    for (int i = 0; i <= MAX_KILLER_DEPTH; ++i)
      m_killers[i].Clear();
  }
}

bool SgABSearch::LookupHash(SgSearchHashData &data) const {
  DBG_ASSERT(!data.IsValid());
  if (m_hash == 0 || !m_hash->Lookup(GetHashCode(), &data))
    return false;
  if (DEBUG_SEARCH) {
    SgDebug() << "SgABSearch::LookupHash: " << GetHashCode() << ' ';
    WriteSgSearchHashData(SgDebug(), *this, data);
  }
  return true;
}

void SgABSearch::OnStartSearch() {
}

void SgABSearch::SetSearchControl(SgSearchControl *control) {
  m_control = control;
}

void SgABSearch::SetProbCut(SgProbCut *probcut) {
  m_probcut = probcut;
}

void SgABSearch::StoreHash(int depth, int value, GoMove move,
                           bool isUpperBound, bool isLowerBound, bool isExact) {
  DBG_ASSERT(m_hash);
  SgSearchHashData data(depth, value, move, isUpperBound, isLowerBound,
                        isExact);
  if (DEBUG_SEARCH) {
    SgDebug() << "SgABSearch::StoreHash: " << GetHashCode()
              << ": ";
    WriteSgSearchHashData(SgDebug(), *this, data);
  }
  m_hash->Store(GetHashCode(), data);
}

bool SgABSearch::TraceIsOn() const {
  return m_tracer && m_tracer->TraceIsOn();
}

bool SgABSearch::AbortSearch() {
  if (!m_aborted) {
    if (m_stat.NumNodes() % m_abortFrequency != 0)
      return false;
    m_aborted =
        m_control
            && m_control->Abort(m_timer.GetTime(), m_stat.NumNodes());
    if (!m_aborted && ForceAbort())
      m_aborted = true;
    if (m_aborted && TraceIsOn())
      m_tracer->TraceComment("aborted");
  }
  return m_aborted;
}

bool SgABSearch::NullMovePrune(int depth, int delta, int beta) {
  SgSearchStack ignoreStack;
  bool childIsExact = true;
  if (beta >= SG_INFINITY - 1)
    return false;
  if (CallExecute(GO_PASS, &delta, depth)) {
    float nullvalue =
        float(-SearchEngine(depth - delta, -beta, -beta + 1, ignoreStack,
                            &childIsExact, true));
    CallTakeBack();
    if (nullvalue >= beta) {
      if (TraceIsOn())
        m_tracer->TraceComment("null-move-cut");
      return true;
    }
  }
  return false;
}

void SgABSearch::GetStatistics(SgSearchStatistics *stat) {
  m_stat.SetTimeUsed(m_timer.GetTime());
  *stat = m_stat;
}

void SgABSearch::StartTime() {
  if (++m_timerLevel == 1) {
    m_stat.Clear();
    m_timer.Start();
  }
}

void SgABSearch::StopTime() {
  if (--m_timerLevel == 0 && !m_timer.IsStopped())
    m_timer.Stop();
}

int SgABSearch::CallEvaluate(int depth, bool *isExact) {
  int v = Evaluate(isExact, depth);
  if (DEBUG_SEARCH)
    SgDebug() << "SgABSearch::CallEvaluate d = " << depth
              << ", v = " << v
              << '\n';
  return v;
}

bool SgABSearch::CallExecute(GoMove move, int *delta, int depth) {
  const SgBlackWhite toPlay = GetToPlay();
  if (DEBUG_SEARCH)
    SgDebug() << "SgABSearch::CallExecute: d = " << depth << ' '
              << SgBW(toPlay) << ' ' << MoveString(move) << '\n';
  if (Execute(move, delta, depth)) {
    m_stat.IncNumMoves();
    if (move == GO_PASS)
      m_stat.IncNumPassMoves();
    m_moveStack.PushBack(move);
    ++m_currentDepth;
    if (TraceIsOn())
      m_tracer->AddTraceNode(move, toPlay);
    return true;
  }
  return false;
}

void SgABSearch::CallTakeBack() {
  if (DEBUG_SEARCH)
    SgDebug() << "SgABSearch::CallTakeBack\n";
  TakeBack();
  if (TraceIsOn())
    m_tracer->TakeBackTraceNode();
  m_moveStack.PopBack();
  --m_currentDepth;
}

void SgABSearch::CreateTracer() {
  m_tracer = new SgSearchTracer(0);
}

void SgABSearch::AddSequenceToHash(const SgVector<GoMove> &sequence, int depth) {
  if (!m_hash)
    return;
  int numMovesToUndo = 0;
  for (SgVectorIterator<GoMove> iter(sequence); iter; ++iter) {
    GoMove move = *iter;
    int delta = DEPTH_UNIT;
    if (CallExecute(move, &delta, depth)) {
      CallTakeBack();
      SgSearchHashData data(0, 0, move);
      DBG_ASSERT(move != GO_NULLMOVE);
      m_hash->Store(GetHashCode(), data);
      if (DEBUG_SEARCH)
        SgDebug() << "SgABSearch::AddSequenceToHash: "
                  << MoveString(move) << '\n';
      delta = DEPTH_UNIT;
      if (CallExecute(move, &delta, depth))
        ++numMovesToUndo;
      else
        DBG_ASSERT(false);
    } else
      break;
  }
  while (--numMovesToUndo >= 0)
    CallTakeBack();
}

int SgABSearch::DFS(int startDepth, int depthLimit,
                    int boundLo, int boundHi,
                    SgVector<GoMove> *sequence, bool *isExactValue) {
  InitSearch(startDepth);
  DBG_ASSERT(m_currentDepth == startDepth);
  DBG_ASSERT(sequence);
  m_aborted = false;
  m_foundNewBest = false;
  SgSearchStack moveStack;
  int value = SearchEngine(depthLimit * DEPTH_UNIT,
                           boundLo, boundHi, moveStack,
                           isExactValue);
  ReverseCopyStack(moveStack, *sequence);
  return value;
}

int SgABSearch::DepthFirstSearch(int depthLimit, int boundLo, int boundHi,
                                 SgVector<GoMove> *sequence, bool clearHash,
                                 SgNode *traceNode) {
  DBG_ASSERT(sequence);
  OnStartSearch();
  if (m_tracer && traceNode) {
    DBG_ASSERT(m_tracer->TraceNode() == 0);
    DBG_ASSERT(m_tracer->TraceIsOn());
    m_tracer->InitTracing("DepthFirstSearch");
  }
  StartTime();
  if (clearHash && m_hash) {
    m_hash->Clear();
    AddSequenceToHash(*sequence, 0);
  }
  m_depthLimit = 0;
  bool isExactValue = true;
  int value = DFS(0, depthLimit, boundLo, boundHi, sequence, &isExactValue);
  StopTime();
  if (m_tracer && traceNode)
    m_tracer->AppendTrace(traceNode);
  return value;
}

int SgABSearch::IteratedSearch(int depthMin, int depthMax, int boundLo,
                               int boundHi, SgVector<GoMove> *sequence,
                               bool clearHash, SgNode *traceNode) {
  DBG_ASSERT(sequence);
  OnStartSearch();
  if (m_tracer && traceNode) {
    DBG_ASSERT(m_tracer->TraceNode() == 0);
    DBG_ASSERT(m_tracer->TraceIsOn());
    m_tracer->InitTracing("IteratedSearch");
  }
  StartTime();
  if (clearHash && m_hash) {
    m_hash->Clear();
    AddSequenceToHash(*sequence, 0);
  }

  int value = 0;
  m_depthLimit = depthMin;
  m_aborted = false;
  m_prevValue = 0;
  m_prevSequence.Clear();
  bool isExactValue = true;

  do {
    if (m_control != 0
        && !m_control->StartNextIteration(m_depthLimit,
                                          m_timer.GetTime(),
                                          m_stat.NumNodes()))
      SetAbortSearch();
    if (m_aborted)
      break;
    StartOfDepth(m_depthLimit);
    m_stat.SetDepthReached(m_depthLimit);
    m_reachedDepthLimit = false;
    isExactValue = true;
    m_foundNewBest = false;
    value = DFS(0, m_depthLimit, boundLo, boundHi, sequence,
                &isExactValue);
    if (m_aborted) {
      if (m_prevSequence.NonEmpty() && !m_foundNewBest) {
        value = m_prevValue;
        *sequence = m_prevSequence;
      }
      break;
    } else {
      if (DEBUG_SEARCH_ITERATIONS)
        PrintPV(*this, m_depthLimit, value, *sequence, isExactValue);
      m_prevValue = value;
      m_prevSequence = *sequence;
    }
    if (isExactValue || value <= boundLo || boundHi <= value)
      break;

    ++m_depthLimit;

  } while (m_depthLimit <= depthMax
      && !isExactValue
      && !m_aborted
      && (!CheckDepthLimitReached() || m_reachedDepthLimit)
      );

  StopTime();
  if (m_tracer && traceNode)
    m_tracer->AppendTrace(traceNode);
  return value;
}

bool SgABSearch::TryMove(GoMove move, const SgVector<GoMove> &specialMoves,
                         const int depth,
                         const int alpha, const int beta,
                         int &loValue, int &hiValue,
                         SgSearchStack &stack,
                         bool &allExact,
                         bool &isCutoff) {
  if (specialMoves.Contains(move))
    return false;

  int delta = DEPTH_UNIT;
  if (!CallExecute(move, &delta, depth))
    return false;

  bool childIsExact = true;
  SgSearchStack newStack;
  int merit = -SearchEngine(depth - delta, -hiValue,
                            -max(loValue, alpha), newStack,
                            &childIsExact);
  if (loValue < merit && !m_aborted)
  {
    loValue = merit;
    if (m_useScout) {
      if (alpha < merit
          && merit < beta
          && delta < depth
          ) {
        childIsExact = true;
        loValue = -SearchEngine(depth - delta, -beta,
                                -merit, newStack,
                                &childIsExact);
      }
      hiValue = max(loValue, alpha) + 1;
    }
    stack.CopyFrom(newStack);
    stack.Push(move);
    DBG_ASSERT(move != GO_NULLMOVE);
    if (m_currentDepth == 1 && !m_aborted)
      m_foundNewBest = true;
  }
  if (!childIsExact)
    allExact = false;
  CallTakeBack();
  if (loValue >= beta) {
    if (m_useKillers && m_currentDepth <= MAX_KILLER_DEPTH)
      m_killers[m_currentDepth].MarkKiller(move);
    if (TraceIsOn())
      m_tracer->TraceComment("b-cut");
    isCutoff = true;
  }
  return true;
}

bool SgABSearch::TrySpecialMove(GoMove move, SgVector<GoMove> &specialMoves,
                                const int depth,
                                const int alpha, const int beta,
                                int &loValue, int &hiValue,
                                SgSearchStack &stack,
                                bool &allExact,
                                bool &isCutoff) {
  if (specialMoves.Contains(move))
    return false;
  bool executed = TryMove(move, specialMoves,
                          depth, alpha, beta,
                          loValue, hiValue, stack,
                          allExact, isCutoff);
  specialMoves.PushBack(move);
  return executed;
}

int SgABSearch::SearchEngine(int depth, int alpha, int beta,
                             SgSearchStack &stack, bool *isExactValue,
                             bool lastNullMove) {
  DBG_ASSERT(stack.IsEmpty() || stack.Top() != GO_NULLMOVE);
  DBG_ASSERT(alpha < beta);
  if (AbortSearch()) {
    *isExactValue = false;
    return alpha;
  }
  if (m_useNullMove
      && depth > 0
      && !lastNullMove
      && NullMovePrune(depth, DEPTH_UNIT * (1 + m_nullMoveDepth), beta)
      ) {
    *isExactValue = false;
    return beta;
  }
  if (m_probcut && m_probcut->IsEnabled()) {
    int probCutValue;
    if (m_probcut->ProbCut(*this, depth, alpha, beta, stack,
                           isExactValue, &probCutValue)
        )
      return probCutValue;
  }

  m_stat.IncNumNodes();
  bool hasMove = false;
  int loValue = -(SG_INFINITY - 1);
  m_reachedDepthLimit = m_reachedDepthLimit || (depth <= 0);
  SgSearchHashData data;
  if (LookupHash(data)) {
    if (data.IsExactValue())
    {
      *isExactValue = true;
      stack.Clear();
      if (data.BestMove() != GO_NULLMOVE)
        stack.Push(data.BestMove());
      if (TraceIsOn())
        m_tracer->TraceValue(data.Value(), GetToPlay(),
                             "exact-hash", true);
      return data.Value();
    }
  }

  bool allExact = true;
  if (depth > 0 && !EndOfGame()) {
    GoMove tryFirst = GO_NULLMOVE;
    GoMove opponentBest = GO_NULLMOVE;
    if (data.IsValid()) {
      if (data.Depth() > 0) {
        tryFirst = data.BestMove();
        DBG_ASSERT(tryFirst != GO_NULLMOVE);
      }
      if (depth <= data.Depth()) {
        int delta = DEPTH_UNIT;
        bool canExecute = CallExecute(tryFirst, &delta, depth);
        if (canExecute)
          CallTakeBack();
        else
          tryFirst = GO_NULLMOVE;
        if (tryFirst != GO_NULLMOVE || data.IsExactValue()) {
          m_reachedDepthLimit = true;
          data.AdjustBounds(&alpha, &beta);

          if (alpha >= beta) {
            *isExactValue = data.IsExactValue();
            stack.Clear();
            if (tryFirst != GO_NULLMOVE)
              stack.Push(tryFirst);
            if (TraceIsOn())
              m_tracer->TraceValue(data.Value(), GetToPlay(),
                                   "Hash hit", *isExactValue);
            return data.Value();
          }
        }
      }

      int delta = DEPTH_UNIT;
      if (tryFirst != GO_NULLMOVE
          && CallExecute(tryFirst, &delta, depth)
          ) {
        bool childIsExact = true;
        loValue = -SearchEngine(depth - delta, -beta, -alpha, stack,
                                &childIsExact);
        if (TraceIsOn())
          m_tracer->TraceComment("tryFirst");
        CallTakeBack();
        hasMove = true;
        if (m_aborted) {
          if (TraceIsOn())
            m_tracer->TraceComment("aborted");
          *isExactValue = false;
          return (1 < m_currentDepth) ? alpha : loValue;
        }
        if (stack.NonEmpty()) {
          opponentBest = stack.Top();
          DBG_ASSERT(opponentBest != GO_NULLMOVE);
        }
        stack.Push(tryFirst);
        if (!childIsExact)
          allExact = false;
        if (loValue >= beta) {
          if (TraceIsOn())
            m_tracer->TraceValue(loValue, GetToPlay());
          bool isExact = SgSearchValue::IsSolved(loValue);
          StoreHash(depth, loValue, tryFirst,
                    false ,
                    true , isExact);
          *isExactValue = isExact;
          if (TraceIsOn())
            m_tracer->TraceValue(loValue, GetToPlay(),
                                 "b-cut", isExact);
          return loValue;
        }
      }
    }
    int hiValue =
        (hasMove && m_useScout) ? max(loValue, alpha) + 1 : beta;
    bool foundCutoff = false;
    SgVector<GoMove> specialMoves;
    if (tryFirst != GO_NULLMOVE)
      specialMoves.PushBack(tryFirst);
    if (!foundCutoff
        && m_useOpponentBest
        && opponentBest != GO_NULLMOVE
        && TrySpecialMove(opponentBest, specialMoves,
                          depth, alpha, beta,
                          loValue, hiValue, stack,
                          allExact, foundCutoff)
        )
      hasMove = true;

    if (!foundCutoff
        && m_useKillers
        && m_currentDepth <= MAX_KILLER_DEPTH
        ) {
      GoMove killer1 = m_killers[m_currentDepth].GetKiller1();
      if (killer1 != GO_NULLMOVE
          && TrySpecialMove(killer1, specialMoves,
                            depth, alpha, beta,
                            loValue, hiValue, stack,
                            allExact, foundCutoff)
          )
        hasMove = true;
      GoMove killer2 = m_killers[m_currentDepth].GetKiller2();
      if (!foundCutoff
          && killer2 != GO_NULLMOVE
          && TrySpecialMove(killer2, specialMoves,
                            depth, alpha, beta,
                            loValue, hiValue, stack,
                            allExact, foundCutoff)
          )
        hasMove = true;
    }
    SgVector<GoMove> moves;
    if (!foundCutoff && !m_aborted) {
      CallGenerate(&moves, depth);
      for (SgVectorIterator<GoMove> it(moves); it && !foundCutoff; ++it) {
        if (TryMove(*it, specialMoves,
                    depth, alpha, beta,
                    loValue, hiValue, stack,
                    allExact, foundCutoff)
            )
          hasMove = true;
        if (!foundCutoff && m_aborted) {
          if (TraceIsOn())
            m_tracer->TraceComment("ABORTED");
          *isExactValue = false;
          return (1 < m_currentDepth) ? alpha : loValue;
        }
      }
    }
#ifndef NDEBUG
    if (hasMove && stack.NonEmpty() && !m_aborted) {
      GoMove bestMove = stack.Top();
      DBG_ASSERT(bestMove != GO_NULLMOVE);
      DBG_ASSERT(specialMoves.Contains(bestMove)
                || moves.Contains(bestMove)
      );
    }
#endif
  }

  bool isSolved = !m_aborted;
  if (!m_aborted) {
    bool solvedByEval = false;
    if (!hasMove) {
      m_stat.IncNumEvals();
      stack.Clear();
      loValue = CallEvaluate(depth, &solvedByEval);
    }
    isSolved = solvedByEval
        || SgSearchValue::IsSolved(loValue)
        || (hasMove && allExact);
    if (m_hash
        && !m_aborted
        && (isSolved || stack.NonEmpty())
        ) {
      GoMove bestMove = GO_NULLMOVE;
      if (stack.NonEmpty()) {
        bestMove = stack.Top();
        DBG_ASSERT(bestMove != GO_NULLMOVE);
      }
      DBG_ASSERT(alpha <= beta);
      StoreHash(depth, loValue, bestMove,
                (loValue <= alpha) ,
                (beta <= loValue) , isSolved);
    }
  }
  if (m_aborted && (1 < m_currentDepth || loValue < alpha))
    loValue = alpha;

  *isExactValue = isSolved;
  if (TraceIsOn())
    m_tracer->TraceValue(loValue, GetToPlay(), 0, isSolved);
  DBG_ASSERT(stack.IsEmpty() || stack.Top() != GO_NULLMOVE);
  return loValue;
}

void SgABSearch::StartOfDepth(int depth) {
  if (DEBUG_SEARCH)
    SgDebug() << "SgABSearch::StartOfDepth: " << depth << '\n';
  if (m_tracer && !m_aborted)
    m_tracer->StartOfDepth(depth);
}

