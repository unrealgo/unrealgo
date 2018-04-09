
#include "platform/SgSystem.h"
#include "SgMiaiStrategy.h"

#include "board/SgWrite.h"

GoPoint SgMiaiPairUtil::Other(const SgMiaiPair &pair, GoPoint p) {
  DBG_ASSERT(pair.first == p || pair.second == p);
  return pair.first == p ? pair.second : pair.first;
}

void SgMiaiStrategy::Write(std::ostream &stream) const {
  SgStrategy::Write(stream);
  stream << "Miai Strategies: ";
  for (SgVectorIterator<SgMiaiPair> it(m_miaiStrategies); it; ++it)
    stream << '(' << GoWritePoint((*it).first)
           << "-" << GoWritePoint((*it).second)
           << ") ";
  stream << "\n" << SgWritePointList(m_openThreats, "open threats", false)
         << "Broken: " << m_failed
         << '\n';
}

void SgMiaiStrategy::AddPair(const SgMiaiPair &m) {
#ifndef NDEBUG
  GoPointSet dependency = Dependency();
  DBG_ASSERT(!dependency.Contains(m.first));
  DBG_ASSERT(!dependency.Contains(m.second));
  DBG_ASSERT(m.first != m.second);
  DBG_ASSERT(GoPointUtil::InBoardRange(m.first));
  DBG_ASSERT(GoPointUtil::InBoardRange(m.second));
#endif
  m_miaiStrategies.PushBack(m);
}

void SgMiaiStrategy::Clear() {
  SgStrategy::Clear();
  m_miaiStrategies.Clear();
  m_openThreats.Clear();
  m_failed = false;
  DBG_ASSERT(Dependency().IsEmpty());
}

GoPointSet SgMiaiStrategy::Dependency() const {
  GoPointSet dependency;
  for (SgVectorIterator<SgMiaiPair> it(m_miaiStrategies); it; ++it) {
    dependency.Include((*it).first);
    dependency.Include((*it).second);
  }
  return dependency;
}

SgStrategyStatus SgMiaiStrategy::Status() const {
  if (m_failed)
    return SGSTRATEGY_FAILED;
  else if (m_openThreats.IsEmpty())
    return SGSTRATEGY_ACHIEVED;
  else if (m_openThreats.IsLength(1))
    return SGSTRATEGY_THREATENED;
  else
    return SGSTRATEGY_FAILED;
}

void SgMiaiStrategy::StrategyFailed() {
  m_failed = true;
  m_openThreats.Clear();
  m_miaiStrategies.Clear();
}

void SgMiaiStrategy::ExecuteMove(const GoPoint p, SgBlackWhite player) {
  if (m_failed)
     return;

  SgVector<GoPoint> fixedThreats;
  for (SgVectorIterator<GoPoint> it(m_openThreats); it; ++it)
    if (p == *it) {
      if (player == Player())
        fixedThreats.PushBack(*it);
      else {
        StrategyFailed();
        break;
      }
    }

  if (m_failed) {
     return;
  }

  SgVector<SgMiaiPair> toChange;
  for (SgVectorIterator<SgMiaiPair> it(m_miaiStrategies); it; ++it)
    if (p == (*it).first || p == (*it).second)
      toChange.PushBack(*it);

  m_miaiStrategies.Exclude(toChange);
  if (player == Player()) {
    m_openThreats.Exclude(fixedThreats);
  } else
    for (SgVectorIterator<SgMiaiPair> it(toChange); it; ++it) {
      m_openThreats.PushBack(SgMiaiPairUtil::Other(*it, p));
    }
}

bool SgMiaiStrategy::HasOverlappingMiaiPairs() const {
  GoPointSet used;
  for (SgVectorIterator<SgMiaiPair> it(m_miaiStrategies); it; ++it) {
    const GoPoint p1 = (*it).first;
    const GoPoint p2 = (*it).second;
    if (used[p1] || used[p2])
       return true;

    used.Include(p1);
    used.Include(p2);
  }
  return false;
}

const SgVector<GoPoint> &SgMiaiStrategy::OpenThreats() const {
  return m_openThreats;
}

GoPoint SgMiaiStrategy::OpenThreatMove() const {
  DBG_ASSERT(m_openThreats.MaxLength(1));
  return m_openThreats.IsEmpty() ? GO_NULLPOINT :
         m_openThreats.Back();
}

void SgMiaiStrategy::UndoMove() {
  DBG_ASSERT(false);
}
