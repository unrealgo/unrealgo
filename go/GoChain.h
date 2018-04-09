

#ifndef GO_CHAIN_H
#define GO_CHAIN_H

#include <iosfwd>
#include "GoBlock.h"
#include "GoBoard.h"
#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class GoRegionBoard;
enum GoChainType {
  GO_CHAIN_TWO_LIBERTIES_IN_REGION,
  GO_CHAIN_TWO_SEPARATE_LIBERTIES,
  GO_CHAIN_BY_SEARCH,
  _GO_CHAIN_COUNT
};
std::ostream& operator<<(std::ostream& stream, GoChainType f);
class GoChainCondition {
 public:

  GoChainCondition(GoChainType type)
      : m_type(type),
        m_lib1(GO_NULLPOINT),
        m_lib2(GO_NULLPOINT) {
    DBG_ASSERT(type == GO_CHAIN_BY_SEARCH);
  }

  GoChainCondition(GoChainType type, GoPoint lib1, GoPoint lib2)
      : m_type(type),
        m_lib1(lib1),
        m_lib2(lib2) {
    DBG_ASSERT(type == GO_CHAIN_TWO_LIBERTIES_IN_REGION);
  }

  bool Overlaps(const GoChainCondition& condition) const;
  bool Overlaps(const SgVectorOf<GoChainCondition>& conditions) const;

  bool UsesLibs() const { return m_type != GO_CHAIN_BY_SEARCH; }

  GoChainType Type() const { return m_type; }

  GoPoint Lib1() const {
    DBG_ASSERT(m_type != GO_CHAIN_BY_SEARCH);
    return m_lib1;
  }

  GoPoint Lib2() const {
    DBG_ASSERT(m_type != GO_CHAIN_BY_SEARCH);
    return m_lib2;
  }

 private:

  GoChainType m_type;
  GoPoint m_lib1, m_lib2;
};
std::ostream& operator<<(std::ostream& stream, const GoChainCondition& c);
class GoChain : public GoBlock {
 public:

  GoChain(const GoBlock* b, const GoBoard& board)
      : GoBlock(b->Color(), b->Anchor(), board),
        m_isSingleBlock(true),
        m_freeLiberties(b->Liberties()) {
    ++s_alloc;
    if (b->IsSafe()) SetToSafe();
  }

  GoChain(const GoChain* c1, const GoChain* c2,
          GoChainCondition* cond);

  virtual ~GoChain() {
    FreeChainConditions();
    ++s_free;
  }

  void CheckConsistency(const GoBoard& bd) const;
  void Write(std::ostream& out) const;
  void WriteID(std::ostream& out) const;
  virtual bool AllEmptyAreLiberties(const GoPointSet& area) const;

  bool IsSingleBlock() const {
    return m_isSingleBlock;
  }

  void TestFor1Eye(const GoRegionBoard* ra);

  const GoPointSet& FreeLiberties() const { return m_freeLiberties; }

  const SgVectorOf<GoChainCondition>& ChainConditions() const {
    return m_chainConditions;
  }

  void GetBlocks(const GoRegionBoard* ra,
                 SgVectorOf<GoBlock>* blocks) const;
  static void Finish();

 private:

  void FreeChainConditions();
  bool m_isSingleBlock;
  GoPointSet m_freeLiberties;
  SgVectorOf<GoChainCondition> m_chainConditions;
  static int s_alloc, s_free;
};
std::ostream& operator<<(std::ostream& stream, const GoChain& c);

#endif
