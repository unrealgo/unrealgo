
#include "platform/SgSystem.h"
#include "board/GoConnCompIterator.h"

using namespace std;


GoConnCompIterator::GoConnCompIterator(const GoPointSet &set, int boardSize)
    : m_set(set),
      m_nextPoint(GoPointUtil::Pt(1, 1) - 1),
      m_lastBoardPoint(GoPointUtil::Pt(boardSize, boardSize)) {
  DBG_ASSERTRANGE(boardSize, 1, GO_MAX_SIZE);
  operator++();
}

void GoConnCompIterator::operator++() {
  ++m_nextPoint;
  while ((m_nextPoint <= m_lastBoardPoint) && !(m_set[m_nextPoint]))
    ++m_nextPoint;
  if (m_nextPoint <= m_lastBoardPoint) {
    m_nextSet = m_set.ConnComp(m_nextPoint);
    m_set -= m_nextSet;
  }
}

const GoPointSet &GoConnCompIterator::operator*() const {
  DBG_ASSERT(m_nextPoint <= m_lastBoardPoint);
  return m_nextSet;
}


GoConnComp8Iterator::GoConnComp8Iterator(const GoPointSet &set, int boardSize)
    : m_set(set),
      m_nextPoint(GoPointUtil::Pt(1, 1) - 1),
      m_lastBoardPoint(GoPointUtil::Pt(boardSize, boardSize)) {
  DBG_ASSERTRANGE(boardSize, 1, GO_MAX_SIZE);
  operator++();
}

void GoConnComp8Iterator::operator++() {
  ++m_nextPoint;
  while ((m_nextPoint <= m_lastBoardPoint) && !(m_set[m_nextPoint]))
    ++m_nextPoint;
  if (m_nextPoint <= m_lastBoardPoint) {
    m_nextSet = m_set.ConnComp8(m_nextPoint);
    m_set -= m_nextSet;
  }
}

const GoPointSet &GoConnComp8Iterator::operator*() const {
  DBG_ASSERT(m_nextPoint <= m_lastBoardPoint);
  return m_nextSet;
}

