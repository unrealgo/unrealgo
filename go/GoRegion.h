

#ifndef GO_REGION_H
#define GO_REGION_H

#include <bitset>
#include <iosfwd>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoEyeCount.h"
#include "lib/SgVector.h"
#include "SgIncrementalStack.h"
#include "SgMiaiStrategy.h"
#include "board/GoPointArray.h"

class GoBlock;
class GoChain;
class GoRegionBoard;
enum GoRegionFlag {
  GO_REGION_SMALL,
  GO_REGION_CORRIDOR,
  GO_REGION_STATIC_1VC,
  GO_REGION_1VC,
  GO_REGION_STATIC_2V,
  GO_REGION_2V,
  GO_REGION_SINGLE_BLOCK_BOUNDARY,
  GO_REGION_OPP_CAN_LIVE_INSIDE,
  GO_REGION_AT_LEAST_SEKI,
  GO_REGION_SAFE,
  GO_REGION_PROTECTED_CUTS,
  GO_REGION_STATIC_1VITAL,
  GO_REGION_1VITAL,
  GO_REGION_USED_FOR_MERGE,
  GO_REGION_VALID,
  GO_REGION_COMPUTED_BLOCKS,
  GO_REGION_COMPUTED_CHAINS,
  GO_REGION_COMPUTED_NAKADE,
  _GO_REGION_FLAG_COUNT
};
typedef std::bitset<_GO_REGION_FLAG_COUNT> GoRegionFlags;
class GoRegion {
 public:

  GoRegion(const GoBoard& board, const GoPointSet& points,
           SgBlackWhite color);

  ~GoRegion() {
#ifndef NDEBUG
    ++s_freed;
#endif
  }

  void ReInitialize();
  void CheckConsistency() const;

  const GoPointSet& Points() const {
    return m_points;
  }

  GoPointSet Dep() const {
    return m_points.Border(m_bd.Size());
  }

  GoPointSet AllInsideLibs() const;

  const SgVectorOf<GoChain>& Chains() const {
    return m_chains;
  }

  const SgVectorOf<GoBlock>& Blocks() const {
    return m_blocks;
  }

  SgVectorOf<GoBlock>& BlocksNonConst() {
    return m_blocks;
  }

  SgVectorOf<GoBlock> InteriorBlocks() const;
  bool IsInteriorBlock(const GoBlock* block) const;

  bool IsBoundaryBlock(const GoBlock* block) const {
    return !IsInteriorBlock(block);
  }

  GoPointSet BlocksPoints() const;
  GoPointSet PointsPlusInteriorBlocks() const;

  SgBlackWhite Color() const {
    return m_color;
  }

  int MinEyes() const { return m_eyes.MinEyes(); }

  int MaxEyes() const { return m_eyes.MaxEyes(); }

  int MinPotEyes() const { return m_eyes.MinPotEyes(); }

  int MaxPotEyes() const { return m_eyes.MaxPotEyes(); }

  void Write(std::ostream& out) const;
  void WriteID(std::ostream& out) const;
  bool GetFlag(GoRegionFlag flag) const;
  bool ComputeAndGetFlag(GoRegionFlag flag);
  bool ComputedFlag(GoRegionFlag flag) const;
  void ComputeFlag(GoRegionFlag flag);

  void ResetNonBlockFlags() {
    m_computedFlags.reset();
    m_computedFlags.set(GO_REGION_COMPUTED_BLOCKS);
    m_eyes.Clear();
    Invalidate();
  }

  bool IsValid() const { return m_flags.test(GO_REGION_VALID); }

  void Invalidate() { m_flags.reset(GO_REGION_VALID); }

  void ComputeBasicFlags();
  void DoComputeFlag(GoRegionFlag flag);

  void SetComputedFlag(GoRegionFlag flag) { m_computedFlags.set(flag); }

  void SetFlag(GoRegionFlag flag, bool value) {
    m_computedFlags.set(flag);
    m_flags.set(flag, value);
  }

  void Set1VCDepth(int depth) { m_1vcDepth = depth; }

  bool Has2ConnForChains(const GoChain* c1, const GoChain* c2) const;
  bool Has2Conn() const;
  bool HealthyForSomeBlock(const SgVectorOf<GoBlock>& blocks) const;
  bool IsSurrounded(const SgVectorOf<GoBlock>& blocks) const;
  bool ComputedVitalForDepth(int depth) const;
  bool SomeBlockIsSafe() const;
  bool AllBlockIsSafe() const;
  bool AdjacentToBlock(GoPoint anchor) const;
  bool AdjacentToSomeBlock(const SgVector<GoPoint>& anchors) const;
  bool Safe2Cuts(const GoBoard& board) const;
  bool AllEmptyAreLibs() const;
  bool Has2SureLibs(SgMiaiStrategy* miaiStrategy) const;
  void InsideLibs(const GoBlock* b, SgVector<GoPoint>* libs) const;
  bool HasBlockLibs(const GoBlock* b) const;
  bool HasLibsForBlock(const GoBlock* b, int n) const;
  bool HasLibForAllBlocks() const;
  bool HasLibsForAllBlocks(int n) const;
  bool Find2ConnForAll() const;
  bool Find2ConnForAllInterior(SgMiaiStrategy* miaiStrategy,
                               SgVector<GoPoint>& usedLibs) const;
  bool Has2IPs(const SgVector<GoPoint>& interiorEmpty, SgMiaiPair* ips)
  const;
  bool Has2IntersectionPoints(const SgVector<GoPoint>& usedLibs) const;
  void GetIPs(SgVector<GoPoint>* ips) const;
  void GetDivideMiaiPairs(SgVector<SgMiaiPair>& pairs) const;
  void JointLibs(SgVector<GoPoint>* libs) const;
  bool IsCorridor() const;
  bool ReplaceChain(const GoChain* old, const GoChain* newChain);
  bool Find2Mergable(GoChain** c1, GoChain** c2) const;
  void Find2FreeLibs(const GoChain* c1, const GoChain* c2,
                     GoPoint* lib1, GoPoint* lib2) const;
  void FindBlocks(const GoRegionBoard& ra);
  void SetBlocks(const SgVectorOf<GoBlock>& blocks);
  void FindChains(const GoRegionBoard& ra);

  void SetToSafe() { SetFlag(GO_REGION_SAFE, true); }

  void RemoveBlock(const GoBlock* b);
  void OnAddStone(GoPoint p);
  void OnRemoveStone(GoPoint p);
  static void Finish();

 private:

  const GoBoard& m_bd;
  GoRegionFlags m_flags;
  GoRegionFlags m_computedFlags;
  GoPointSet m_points;
  SgBlackWhite m_color;
  SgVectorOf<GoBlock> m_blocks;
  SgVectorOf<GoChain> m_chains;
  GoEyeCount m_eyes;
  GoPoint m_vitalPoint;
  int m_1vcDepth;
  SgMiaiStrategy m_miaiStrategy;
  bool ProtectedCuts(const GoBoard& board) const;
  bool ComputeIs1Vital() const;
  bool StaticIs1VitalAndConnected() const;
  void ComputeNakade();
  void ComputeSingleBlockEyeSpace();
  void ComputeMultipleBlockEyeSpace();
  void ComputeEyeSpace();
  void InteriorEmpty(SgVector<GoPoint>* interiorEmpty, int maxNu) const;

#ifndef NDEBUG

  static int s_allocated, s_freed;
#endif
};
std::ostream& operator<<(std::ostream& stream, const GoRegion& r);

#endif

