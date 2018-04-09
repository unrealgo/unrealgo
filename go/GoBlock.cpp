

#include "platform/SgSystem.h"
#include "GoBlock.h"

#include <iostream>
#include "GoRegion.h"
#include "board/GoPointSetUtil.h"
#include "board/SgWrite.h"

static const bool CHECK = SG_CHECK && true;
static const bool HEAVYCHECK = SG_HEAVYCHECK && CHECK && false;
int GoBlock::s_allocated = 0;
int GoBlock::s_freed = 0;

void GoBlock::AddStone(GoPoint stone) {
  if (HEAVYCHECK)
    DBG_ASSERT(!m_stones.Contains(stone));
  m_stones.Include(stone);
  if (stone < m_anchor) {
    if (HEAVYCHECK)
      DBG_ASSERT(stone == m_bd.Anchor(m_anchor));
    m_anchor = stone;
  }
}

void GoBlock::RemoveStone(GoPoint stone) {
  if (HEAVYCHECK)
    DBG_ASSERT(m_stones.Contains(stone));
  m_stones.Exclude(stone);
  if (stone == m_anchor)
    m_anchor = m_bd.Anchor(m_stones.PointOf());
}

bool GoBlock::AllEmptyAreLiberties(const GoPointSet& area) const {
  const GoPoint anchor = Anchor();
  for (SgSetIterator it(area); it; ++it)
    if (m_bd.IsEmpty(*it)
        && !m_bd.IsLibertyOfBlock(*it, anchor)
        )
      return false;
  return true;
}

void GoBlock::TestFor1Eye(const GoRegion* r) {
  if (r->GetFlag(GO_REGION_SMALL) && r->Blocks().IsLength(1)) {
    m_has1Eye = true;
  }
}

void GoBlock::CheckConsistency() const {
  DBG_ASSERT(Stones().SubsetOf(m_bd.All(Color())));
  GoPoint anchor = Anchor();
  DBG_ASSERT(m_bd.Anchor(anchor) == Anchor());
  GoPointSet stones;
  for (GoBoard::StoneIterator it(m_bd, anchor); it; ++it)
    stones.Include(*it);
  DBG_ASSERT(Stones() == stones);
}

void GoBlock::Finish() {
  DBG_ASSERT(s_allocated == s_freed);
}

void GoBlock::Write(std::ostream& stream) const {
  WriteID(stream);
  stream << '\n'
         << PointSetWriter(Stones(), "Stones: ")
         << "\nhealthy: " << Healthy().Length()
         << "\nisSafe: " << SgWriteBoolean(IsSafe())
         << "\nhas1Eye: " << SgWriteBoolean(Has1Eye())
         << "\n";
}

void GoBlock::WriteID(std::ostream& stream) const {
  stream << ' ' << SgBW(Color())
         << " Block " << GoWritePoint(Anchor());
}

std::ostream& operator<<(std::ostream& stream, const GoBlock& b) {
  b.Write(stream);
  return stream;
}

