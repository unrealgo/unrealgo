

#ifndef GO_REGIONBOARD_H
#define GO_REGIONBOARD_H

#include "GoBoard.h"
#include "GoRegion.h"

class GoBlock;
class GoChain;
class GoRegionBoard {
 public:

  explicit GoRegionBoard(const GoBoard& board);
  virtual ~GoRegionBoard();
  void Clear();
  void CheckConsistency() const;

  bool UpToDate() const {
    return !m_invalid
        && m_boardSize == m_board.Size()
        && m_code == Board().GetHashCode();
  }

  bool ComputedHealthy() const {
    return m_computedHealthy;
  }

  bool ChainsUpToDate() const {
    return UpToDate() && m_chainsCode == Board().GetHashCode();
  }

  void ExecuteMovePrologue();
  void OnExecutedMove(GoPlayerMove playerMove);
  void OnExecutedUncodedMove(int move, SgBlackWhite moveColor);
  void OnUndoneMove();

  SgVectorOf<GoBlock>& AllBlocks(SgBlackWhite color) {
    return m_allBlocks[color];
  }

  const SgVectorOf<GoBlock>& AllBlocks(SgBlackWhite color) const {
    return m_allBlocks[color];
  }

  SgVectorOf<GoChain>& AllChains(SgBlackWhite color) {
    return m_allChains[color];
  }

  const SgVectorOf<GoChain>& AllChains(SgBlackWhite color) const {
    return m_allChains[color];
  }

  SgVectorOf<GoRegion>& AllRegions(SgBlackWhite color) {
    return m_allRegions[color];
  }

  const SgVectorOf<GoRegion>& AllRegions(SgBlackWhite color) const {
    return m_allRegions[color];
  }

  const GoPointSet& All(SgBlackWhite color) const {
    return Board().All(color);
  }

  const GoPointSet& AllEmpty() const { return Board().AllEmpty(); }

  const GoPointSet& AllPoints() const { return Board().AllPoints(); }

  bool IsColor(GoPoint p, int c) const { return Board().IsColor(p, c); }

  void WriteBlocks(std::ostream& stream) const;
  void WriteRegions(std::ostream& stream) const;
  void GenBlocksRegions();
  void GenChains();
  void ReInitializeBlocksRegions();
  void SetComputedFlagForAll(GoRegionFlag flag);

  const GoBoard& Board() const {
    return m_board;
  }

  GoBlock* GetBlock(const GoPointSet& boundary,
                    SgBlackWhite color) const;

  GoRegion* PreviousRegionAt(GoPoint p, SgBlackWhite color) const {
    DBG_ASSERT(Board().Occupied(p));
    DBG_ASSERT(m_region[color][p] != 0);
    return m_region[color][p];
  }

  GoRegion* RegionAt(GoPoint p, SgBlackWhite color) const {
    DBG_ASSERT(UpToDate());
    DBG_ASSERT(!Board().IsColor(p, color));
    DBG_ASSERT(m_region[color][p] != 0);
    return m_region[color][p];
  }

  void RegionsAt(const GoPointSet& area, SgBlackWhite color,
                 SgVectorOf<GoRegion>* regions) const;
  void AdjacentRegions(const SgVector<GoPoint>& points, SgBlackWhite color,
                       SgVectorOf<GoRegion>* regions) const;
  void PreviousBlocksAt(const SgVector<GoPoint>& area, SgBlackWhite color,
                        SgVectorOf<GoBlock>* captures) const;

  GoBlock* BlockAt(GoPoint p) const {
    DBG_ASSERT(m_block[p] != 0);
    return m_block[p];
  }

  GoChain* ChainAt(GoPoint p) const;
  bool IsSafeBlock(GoPoint p) const;
  void SetToSafe(GoPoint p) const;
  void SetSafeFlags(const GoBWSet& safe);
  void SetComputedHealthy();
  static bool Init();
  static void Finish();

 private:

  void GenBlocks();
  void FindBlocksWithEye();
  GoBlock* GenBlock(GoPoint anchor, SgBlackWhite color);
  GoRegion* GenRegion(const GoPointSet& area, SgBlackWhite color);
  void UpdateBlock(int move, SgBlackWhite moveColor);
  void SetRegionArrays(GoRegion* r);
  void AddBlock(GoBlock* b, bool isExecute = true);
  void RemoveBlock(GoBlock* b, bool isExecute, bool removeFromRegions);
  void AddRegion(GoRegion* r, bool isExecute = true);
  void RemoveRegion(GoRegion* r, bool isExecute = true);
  void MergeAdjacentAndAddBlock(GoPoint move, SgBlackWhite capturedColor);
  GoRegion* MergeAll(const SgVectorOf<GoRegion>& regions,
                     const GoPointSet& captured, SgBlackWhite color);
  SgIncrementalStack m_stack;
  void PushRegion(int type, GoRegion* r);
  void PushStone(GoRegion* r, GoPoint move);
  void PushBlock(int type, GoBlock* b);
  void AppendStone(GoBlock* b, GoPoint move);
  const GoBoard& m_board;
  SgBWArray<GoPointArray<GoRegion*> > m_region;
  GoPointArray<GoBlock*> m_block;
  SgBWArray<SgVectorOf<GoBlock> > m_allBlocks;
  SgBWArray<SgVectorOf<GoChain> > m_allChains;
  SgBWArray<SgVectorOf<GoRegion> > m_allRegions;
  SgHashCode m_code;
  SgHashCode m_chainsCode;
  bool m_invalid;
  bool m_computedHealthy;
  int m_boardSize;
  static int s_alloc, s_free;

};

#endif
