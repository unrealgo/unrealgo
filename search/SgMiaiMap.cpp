//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"
#include "SgMiaiMap.h"

#include "board/SgWrite.h"

//----------------------------------------------------------------------------

SgMiaiMap::SgMiaiMap()
    : m_forcedMove(GO_NULLPOINT),
      m_failed(false),
      m_map(GoArray<int, GO_MAXPOINT>(GO_NULLPOINT)) {}


void SgMiaiMap::ExecuteMove(GoPoint p, SgBlackWhite player) {
  if (m_forcedMove != GO_NULLPOINT) {
    DBG_ASSERT(m_forcedMove == p);
    m_forcedMove = GO_NULLPOINT;
  } else if (m_map[player][p] != GO_NULLPOINT) {
    m_forcedMove = m_map[player][p];
    m_map[player][p] = GO_NULLPOINT;
    m_map[player][m_forcedMove] = GO_NULLPOINT; // too early???

    SgBlackWhite opp = SgOppBW(player);
    GoPoint temp = m_map[opp][p];
    if (temp != GO_NULLPOINT) {
      DBG_ASSERT(temp != GO_NULLPOINT);
      m_map[opp][p] = GO_NULLPOINT;
      DBG_ASSERT(m_map[opp][temp] == p);
      m_map[opp][temp] = GO_NULLPOINT;
    }
  }
}

SgStrategyStatus SgMiaiMap::Status() const {
  return m_failed ? SGSTRATEGY_FAILED :
         m_forcedMove != GO_NULLPOINT ? SGSTRATEGY_THREATENED :
         SGSTRATEGY_ACHIEVED;
}

void SgMiaiMap::ConvertFromSgMiaiStrategy(const SgMiaiStrategy &s) {
  DBG_ASSERT(!s.HasOverlappingMiaiPairs());

  const SgBlackWhite player = s.Player();
  for (SgVectorIterator<SgMiaiPair> it(s.MiaiStrategies()); it; ++it) {
    const GoPoint p1 = (*it).first;
    const GoPoint p2 = (*it).second;
    DBG_ASSERT(m_map[player][p1] == GO_NULLPOINT);
    DBG_ASSERT(m_map[player][p2] == GO_NULLPOINT);
    m_map[player][p1] = p2;
    m_map[player][p2] = p1;
  }
  const GoPoint m = s.OpenThreatMove();
  if (m != GO_NULLPOINT)
    m_forcedMove = m;
}

void SgMiaiMap::ConvertToSgMiaiStrategy(SgMiaiStrategy *s) const {
  SuppressUnused(s);
  DBG_ASSERT(false);
}

//----------------------------------------------------------------------------
