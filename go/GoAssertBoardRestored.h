

#ifndef GO_ASSERTBOARDRESTORED_H
#define GO_ASSERTBOARDRESTORED_H

#include "GoBoard.h"

class GoAssertBoardRestored {
 public:

  GoAssertBoardRestored();
  GoAssertBoardRestored(const GoBoard& bd);
  ~GoAssertBoardRestored();
  void AssertRestored();
  void Init(const GoBoard& bd);
  void Clear();

 private:
#ifndef NDEBUG
  const GoBoard *m_bd;

  int m_size;

  SgBlackWhite m_toPlay;


  SgHashCode m_hash;


  int m_moveNumber;

  SgBWArray<int> m_numStones;

  GoRules m_rules;

  bool m_allowKoRepetition;

  bool m_allowAnyRepetition;

  bool m_koModifiesHash;

  SgEmptyBlackWhite m_koColor;

  int m_koLevel;

  SgEmptyBlackWhite m_koLoser;
#endif

  GoAssertBoardRestored(const GoAssertBoardRestored&);
  GoAssertBoardRestored& operator=(const GoAssertBoardRestored&);
};

inline GoAssertBoardRestored::GoAssertBoardRestored() {
#ifndef NDEBUG
  m_bd = 0;
#endif
}

inline GoAssertBoardRestored::GoAssertBoardRestored(const GoBoard& bd) {
  SG_DEBUG_ONLY(bd);
#ifndef NDEBUG
  Init(bd);
#endif
}

inline GoAssertBoardRestored::~GoAssertBoardRestored() {
#ifndef NDEBUG
  AssertRestored();
#endif
}

inline void GoAssertBoardRestored::AssertRestored() {
#ifndef NDEBUG
  if (m_bd == 0)
    return;
  DBG_ASSERT(m_bd->Size() == m_size);
  DBG_ASSERT(m_bd->ToPlay() == m_toPlay);
  DBG_ASSERT(m_bd->GetHashCode() == m_hash);
  DBG_ASSERT(m_bd->MoveNumber() == m_moveNumber);
  DBG_ASSERT(m_bd->TotalNumStones(SG_BLACK) == m_numStones[SG_BLACK]);
  DBG_ASSERT(m_bd->TotalNumStones(SG_WHITE) == m_numStones[SG_WHITE]);
  DBG_ASSERT(m_bd->Rules() == m_rules);
  DBG_ASSERT(m_bd->KoRepetitionAllowed() == m_allowKoRepetition);
  DBG_ASSERT(m_bd->AnyRepetitionAllowed() == m_allowAnyRepetition);
  DBG_ASSERT(m_bd->KoModifiesHash() == m_koModifiesHash);
  DBG_ASSERT(m_bd->KoColor() == m_koColor);
  DBG_ASSERT(m_bd->KoLevel() == m_koLevel);
  DBG_ASSERT(m_bd->KoLoser() == m_koLoser);
#endif
}

inline void GoAssertBoardRestored::Clear() {
#ifndef NDEBUG
  m_bd = 0;
#endif
}

inline void GoAssertBoardRestored::Init(const GoBoard& bd) {
  SG_DEBUG_ONLY(bd);
#ifndef NDEBUG
  m_bd = &bd;
  m_size = bd.Size();
  m_toPlay = bd.ToPlay();
  m_hash = bd.GetHashCode();
  m_moveNumber = bd.MoveNumber();
  m_numStones = bd.TotalNumStones();
  m_rules = bd.Rules();
  m_allowKoRepetition = bd.KoRepetitionAllowed();
  m_allowAnyRepetition = bd.AnyRepetitionAllowed();
  m_koModifiesHash = bd.KoModifiesHash();
  m_koColor = bd.KoColor();
  m_koLevel = bd.KoLevel();
  m_koLoser = bd.KoLoser();
#endif
}

#endif
