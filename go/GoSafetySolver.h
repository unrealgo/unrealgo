

#ifndef GO_SAFETYSOLVER_H
#define GO_SAFETYSOLVER_H

#include "GoStaticSafetySolver.h"

class GoSafetySolver : public GoStaticSafetySolver {
 public:

  explicit GoSafetySolver(const GoBoard& board, GoRegionBoard* regions = 0)
      : GoStaticSafetySolver(board, regions),
        m_code() {}

  void FindSafePoints(GoBWSet* safe);
  virtual void FindSurroundedSafeAreas(GoBWSet* safe, SgBlackWhite color);
  virtual void FindHealthy();
  bool PotentialCaptureMove(GoPoint p,
                            SgBlackWhite regionColor) const;

  virtual bool UpToDate() const {
    return GoStaticSafetySolver::UpToDate()
        && m_code == Board().GetHashCode();
  }

 protected:

  virtual bool RegionHealthyForBlock(const GoRegion& r,
                                     const GoBlock& b) const;
  virtual void GenBlocksRegions();
  virtual void FindClosure(SgVectorOf<GoBlock>* blocks) const;
  virtual void FindTestSets(SgVectorOf<SgVectorOf<GoBlock> >* sets,
                            SgBlackWhite color) const;
  virtual void Test2Vital(GoRegion* r, GoBWSet* safe);
  void Find2VitalAreas(GoBWSet* safe);
  bool FindSafePair(GoBWSet* safe,
                    SgBlackWhite color,
                    const GoPointSet& anySafe,
                    const GoRegion* r1);
  void Merge(GoChain* c1, GoChain* c2, GoRegion* r, bool bySearch);

 private:

  bool FindSurroundedRegionPair(GoBWSet* safe, SgBlackWhite color);
  bool FindSurroundedSingleRegion(GoBWSet* safe, SgBlackWhite color);
  SgHashCode m_code;
};

#endif
