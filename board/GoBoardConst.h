
#ifndef SG_BOARDCONST_H
#define SG_BOARDCONST_H

#include <boost/shared_ptr.hpp>
#include "lib/Array.h"
#include "board/GoNbIterator.h"
#include "board/GoPoint.h"
#include "board/GoPointArray.h"
#include "board/GoPointIterator.h"
#include "board/GoPointSet.h"

class GoBoardConst {
 public:
  explicit GoBoardConst(GoGrid size);
  
  void ChangeSize(GoGrid newSize);
  
  GoGrid Size() const;
  
  GoGrid Line(GoPoint p) const;
  
  GoGrid Pos(GoPoint p) const;
  
  int Up(GoPoint p) const;
  
  int Left(GoPoint p) const;
  
  int Right(GoPoint p) const;
  
  int Side(GoPoint p, int i) const;
  
  const GoPointSet &Corners() const;
  
  const GoPointSet &Edges() const;
  
  const GoPointSet &Centers() const;
  
  const GoPointSet &SideExtensions() const;
  
  const GoPointSet &LineSet(GoGrid line) const;
  
  int FirstBoardPoint() const;
  
  int LastBoardPoint() const;
  
  const GoPoint *NeighborIterAddress(GoPoint p) const;
  const GoPoint *BoardIterAddress() const;
  const GoPoint *BoardIterEnd() const;
  const GoPoint *LineIterAddress(GoGrid line) const;
  const GoPoint *CornerIterAddress() const;

 private:
  struct BoardConstImpl {
    explicit BoardConstImpl(GoGrid size);
    GoGrid m_size;
    int m_firstBoardPoint;
    int m_lastBoardPoint;
    GoPointArray<GoGrid> m_gridToLine;
    GoPointArray<GoGrid> m_gridToPos;
    
    GoPoint m_neighbors[GO_MAXPOINT][5];
    GoPointArray<int> m_up;
    GoPointSet m_corners;
    GoPointSet m_edges;
    GoPointSet m_centers;
    GoPointSet m_sideExtensions;
    GoPoint m_cornerIter[4 + 1];
    GoPointSet m_line[(GO_MAX_SIZE / 2) + 1];
    GoPoint *m_lineIterAddress[(GO_MAX_SIZE / 2) + 1];
    GoPoint m_boardIter[GO_MAX_ONBOARD + 1];
    GoPoint *m_boardIterEnd;
    GoPoint m_lineIter[GO_MAX_SIZE * GO_MAX_SIZE + (GO_MAX_SIZE / 2) + 1];
    int m_side[2][2 * (GO_NORTH_SOUTH + GO_WEST_EAST) + 1];
  };
  boost::shared_ptr<BoardConstImpl> m_const;
  void Create(GoGrid size);
  static GoArray<boost::shared_ptr<BoardConstImpl>, GO_MAX_SIZE + 1> s_const;
};

inline GoGrid GoBoardConst::Size() const {
  return m_const->m_size;
}

inline GoGrid GoBoardConst::Line(GoPoint p) const {
  return m_const->m_gridToLine[p];
}

inline const GoPoint *GoBoardConst::NeighborIterAddress(GoPoint p) const {
  return m_const->m_neighbors[p];
}

inline GoGrid GoBoardConst::Pos(GoPoint p) const {
  return m_const->m_gridToPos[p];
}

inline int GoBoardConst::Up(GoPoint p) const {
  return m_const->m_up[p];
}

inline int GoBoardConst::Left(GoPoint p) const {
  return m_const->m_side[0][m_const->m_up[p] + (GO_NORTH_SOUTH + GO_WEST_EAST)];
}

inline int GoBoardConst::Right(GoPoint p) const {
  return m_const->m_side[1][m_const->m_up[p] + (GO_NORTH_SOUTH + GO_WEST_EAST)];
}

inline int GoBoardConst::Side(GoPoint p, int i) const {
  return m_const->m_side[i][m_const->m_up[p] + (GO_NORTH_SOUTH + GO_WEST_EAST)];
}

inline const GoPointSet &GoBoardConst::Corners() const {
  return m_const->m_corners;
}

inline const GoPointSet &GoBoardConst::Edges() const {
  return m_const->m_edges;
}

inline const GoPointSet &GoBoardConst::Centers() const {
  return m_const->m_centers;
}

inline const GoPointSet &GoBoardConst::SideExtensions() const {
  return m_const->m_sideExtensions;
}

inline const GoPointSet &GoBoardConst::LineSet(GoGrid line) const {
  return m_const->m_line[line - 1];
}

inline int GoBoardConst::FirstBoardPoint() const {
  return m_const->m_firstBoardPoint;
}

inline int GoBoardConst::LastBoardPoint() const {
  return m_const->m_lastBoardPoint;
}

inline const GoPoint *GoBoardConst::BoardIterAddress() const {
  return m_const->m_boardIter;
}

inline const GoPoint *GoBoardConst::BoardIterEnd() const {
  return m_const->m_boardIterEnd;
}

inline const GoPoint *GoBoardConst::LineIterAddress(GoGrid line) const {
  return m_const->m_lineIterAddress[line - 1];
}

inline const GoPoint *GoBoardConst::CornerIterAddress() const {
  return m_const->m_cornerIter;
}


class SgLineIterator
    : public GoPointIterator {
 public:
  SgLineIterator(const GoBoardConst &boardConst, GoGrid line)
      : GoPointIterator(boardConst.LineIterAddress(line)) {}
};

class SgCornerIterator
    : public GoPointIterator {
 public:
  explicit SgCornerIterator(const GoBoardConst &boardConst)
      : GoPointIterator(boardConst.CornerIterAddress()) {}
};

class SgSideIterator {
 public:
  SgSideIterator(const GoBoardConst &boardConst, GoPoint p)
      : m_boardConst(boardConst),
        m_p(p),
        m_index(0) {
    DBG_ASSERT(m_boardConst.Side(m_p, 0) != 0);
  }

  
  void operator++() {
    ++m_index;
  }

  
  int operator*() const {
    return m_boardConst.Side(m_p, m_index);
  }

  
  explicit operator bool() const {
    return m_index <= 1;
  }

 private:
  const GoBoardConst &m_boardConst;
  GoPoint m_p;
  int m_index;
};

class SgNbIterator
    : public GoPointIterator {
 public:
  SgNbIterator(const GoBoardConst &boardConst, GoPoint p);
};

inline SgNbIterator::SgNbIterator(const GoBoardConst &boardConst, GoPoint p)
    : GoPointIterator(boardConst.NeighborIterAddress(p)) {}

#endif
