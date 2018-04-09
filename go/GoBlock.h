

#ifndef GO_BLOCK_H
#define GO_BLOCK_H

#include <iosfwd>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "board/GoBlackWhite.h"
#include "platform/SgDebug.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class GoRegion;
class GoBlock {
 public:
  GoBlock(SgBlackWhite color, GoPoint anchor, const GoBoard& board)
      : m_has1Eye(false),
        m_bd(board),
        m_color(color),
        m_anchor(anchor),
        m_isSafe(false) {
    ++s_allocated;
    for (GoBoard::StoneIterator it(board, anchor); it; ++it)
      m_stones.Include(*it);
  }

  virtual ~GoBlock() { ++s_freed; }

  void CheckConsistency() const;
  virtual void Write(std::ostream& out) const;
  virtual void WriteID(std::ostream& out) const;

  SgBlackWhite Color() const {
    return m_color;
  }

  GoPoint Anchor() const {
    return m_anchor;
  }

  const GoPointSet& Stones() const {
    return m_stones;
  }

  int NuLiberties() const {
    return m_bd.NumLiberties(m_anchor);
  }

  GoPointSet Liberties() const {
    return m_stones.Border(m_bd.Size()) & m_bd.AllEmpty();
  }

  bool IsSafe() const {
    return m_isSafe;
  }

  virtual bool AllEmptyAreLiberties(const GoPointSet& area) const;

  bool HasLiberty(GoPoint lib) const {
    return m_bd.IsLibertyOfBlock(lib, m_anchor);
  }

  bool Has1Eye() const {
    return m_has1Eye;
  }

  void SetToSafe() {
    m_isSafe = true;
  }

  void TestFor1Eye(const GoRegion* r);

  virtual void ReInitialize() {
    m_isSafe = false;
    m_has1Eye = false;
    m_healthy.Clear();
  }

  void AddStone(GoPoint stone);
  void RemoveStone(GoPoint stone);

  void AddHealthy(GoRegion* r) {
    if (!m_healthy.Contains(r))
      m_healthy.PushBack(r);
#ifndef NDEBUG
    else
      SgDebug() << "DOUBLE ADD " << r << '\n';
#endif
  }

  void RemoveRegion(GoRegion* r) {
    m_healthy.Exclude(r);
  }

  bool ContainsHealthy(const GoRegion* r) const {
    return m_healthy.Contains(r);
  }

  const SgVectorOf<GoRegion>& Healthy() const {
    return m_healthy;
  }

  static void Finish();

 protected:
  SgVectorOf<GoRegion> m_healthy;
  bool m_has1Eye;
  const GoBoard& m_bd;

  GoBlock(const GoBlock* b, const GoPointSet& stones, const SgVectorOf<GoRegion>& healthy)
      : m_healthy(healthy),
        m_has1Eye(false),
        m_bd(b->m_bd),
        m_color(b->Color()),
        m_stones(stones),
        m_anchor(GO_NULLPOINT),
        m_isSafe(false) {
    ++s_allocated;
  }

 private:
  SgBlackWhite m_color;
  GoPointSet m_stones;
  GoPoint m_anchor;
  bool m_isSafe;

  // record num of all allocated and freed blocks
  static int s_allocated;
  static int s_freed;
};
std::ostream& operator<<(std::ostream& stream, const GoBlock& b);

#endif
