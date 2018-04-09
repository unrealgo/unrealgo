

#ifndef GO_STATICSAFETYSOLVER_H
#define GO_STATICSAFETYSOLVER_H

#include "GoBoard.h"
#include "GoRegion.h"
#include "GoRegionBoard.h"

class GoStaticSafetySolver {
 public:

  GoStaticSafetySolver(const GoBoard& board, GoRegionBoard* regions = 0);
  virtual ~GoStaticSafetySolver();
  const GoBoard& Board() const;
  virtual bool UpToDate() const;
  const GoRegionBoard* Regions() const;

 protected:

  GoRegionBoard* Regions();
  virtual void FindTestSets(SgVectorOf<SgVectorOf<GoBlock> >* sets,
                            SgBlackWhite color) const;
  virtual void FindClosure(SgVectorOf<GoBlock>* blocks) const;
  virtual void GenBlocksRegions();
  virtual bool RegionHealthyForBlock(const GoRegion& r,
                                     const GoBlock& b) const;
  virtual void FindSafePoints(GoBWSet* safe);
  virtual void FindHealthy();
  void TestAlive(SgVectorOf<GoBlock>* blocks, GoBWSet* safe,
                 SgBlackWhite color);
  void TestAdjacent(SgVectorOf<GoRegion>* regions,
                    const SgVectorOf<GoBlock>& blocks) const;

 private:

  const GoBoard& m_board;
  GoRegionBoard* m_regions;
  bool m_allocRegion;
  GoStaticSafetySolver(const GoStaticSafetySolver&);
  GoStaticSafetySolver& operator=(const GoStaticSafetySolver&);
};

inline const GoBoard& GoStaticSafetySolver::Board() const {
  return m_board;
}

inline GoRegionBoard* GoStaticSafetySolver::Regions() {
  DBG_ASSERT(m_regions);
  return m_regions;
}

inline const GoRegionBoard* GoStaticSafetySolver::Regions() const {
  DBG_ASSERT(m_regions);
  return m_regions;
}

#endif
