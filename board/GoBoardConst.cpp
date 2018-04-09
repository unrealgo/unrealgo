
#include "platform/SgSystem.h"
#include "board/GoBoardConst.h"

#include <algorithm>

using namespace std;
using GoPointUtil::Pt;

GoBoardConst::BoardConstImpl::BoardConstImpl(GoGrid size)
    : m_size(size),
      m_firstBoardPoint(Pt(1, 1)),
      m_lastBoardPoint(Pt(size, size)) {
  m_gridToLine.Fill(0);
  m_gridToPos.Fill(0);
  m_up.Fill(0);
  for (GoGrid line = 0; line <= GO_MAX_SIZE / 2; ++line)
    m_line[line].Clear();
  m_boardIterEnd = m_boardIter;
  GoGrid halfSize = (m_size + 1) / 2;
  for (GoGrid row = 1; row <= m_size; ++row) {
    for (GoGrid col = 1; col <= m_size; ++col) {
      GoPoint p = Pt(col, row);
      GoGrid lineRow = row > halfSize ? m_size + 1 - row : row;
      GoGrid lineCol = col > halfSize ? m_size + 1 - col : col;
      GoGrid line = min(lineRow, lineCol);
      GoGrid pos = max(lineRow, lineCol);
      m_gridToLine[p] = line;
      m_gridToPos[p] = pos;
      m_line[line - 1].Include(p);
      const int MAX_EDGE = m_size > 11 ? 4 : 3;
      if (line <= MAX_EDGE) {
        if (pos <= MAX_EDGE + 1)
          m_corners.Include(p);
        else
          m_edges.Include(p);
      } else
        m_centers.Include(p);

      int nuNb = 0;
      if (row > 1)
        m_neighbors[p][nuNb++] = Pt(col, row - 1);
      if (col > 1)
        m_neighbors[p][nuNb++] = Pt(col - 1, row);
      if (col < m_size)
        m_neighbors[p][nuNb++] = Pt(col + 1, row);
      if (row < m_size)
        m_neighbors[p][nuNb++] = Pt(col, row + 1);
      m_neighbors[p][nuNb] = GO_ENDPOINT;

      *(m_boardIterEnd++) = p;
    }
  }
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p) {
    if (m_gridToLine[p] != 0) {
      GoGrid line = m_gridToLine[p];
      if (m_gridToLine[p - GO_NORTH_SOUTH] == line + 1)
        m_up[p] = -GO_NORTH_SOUTH;
      if (m_gridToLine[p - GO_WEST_EAST] == line + 1)
        m_up[p] = -GO_WEST_EAST;
      if (m_gridToLine[p + GO_WEST_EAST] == line + 1)
        m_up[p] = GO_WEST_EAST;
      if (m_gridToLine[p + GO_NORTH_SOUTH] == line + 1)
        m_up[p] = GO_NORTH_SOUTH;
      if (m_up[p] == 0) {
        if (m_gridToLine[p - GO_NORTH_SOUTH - GO_WEST_EAST] == line + 1)
          m_up[p] = -GO_NORTH_SOUTH - GO_WEST_EAST;
        if (m_gridToLine[p - GO_NORTH_SOUTH + GO_WEST_EAST] == line + 1)
          m_up[p] = -GO_NORTH_SOUTH + GO_WEST_EAST;
        if (m_gridToLine[p + GO_NORTH_SOUTH - GO_WEST_EAST] == line + 1)
          m_up[p] = GO_NORTH_SOUTH - GO_WEST_EAST;
        if (m_gridToLine[p + GO_NORTH_SOUTH + GO_WEST_EAST] == line + 1)
          m_up[p] = GO_NORTH_SOUTH + GO_WEST_EAST;
      }
    }
  }
  {
    for (int i = 0; i <= 2 * (GO_NORTH_SOUTH + GO_WEST_EAST); ++i) {
      m_side[0][i] = 0;
      m_side[1][i] = 0;
    }
    m_side[0][-GO_NORTH_SOUTH + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_WEST_EAST;
    m_side[1][-GO_NORTH_SOUTH + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_WEST_EAST;
    m_side[0][-GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_NORTH_SOUTH;
    m_side[1][-GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_NORTH_SOUTH;
    m_side[0][+GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_NORTH_SOUTH;
    m_side[1][+GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_NORTH_SOUTH;
    m_side[0][+GO_NORTH_SOUTH + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_WEST_EAST;
    m_side[1][+GO_NORTH_SOUTH + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_WEST_EAST;
    m_side[0][-GO_NORTH_SOUTH - GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_WEST_EAST;
    m_side[1][-GO_NORTH_SOUTH - GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_NORTH_SOUTH;
    m_side[0][-GO_NORTH_SOUTH + GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_NORTH_SOUTH;
    m_side[1][-GO_NORTH_SOUTH + GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_WEST_EAST;
    m_side[0][+GO_NORTH_SOUTH - GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_NORTH_SOUTH;
    m_side[1][+GO_NORTH_SOUTH - GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = -GO_WEST_EAST;
    m_side[0][+GO_NORTH_SOUTH + GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_WEST_EAST;
    m_side[1][+GO_NORTH_SOUTH + GO_WEST_EAST + (GO_NORTH_SOUTH + GO_WEST_EAST)] = +GO_NORTH_SOUTH;
  }
  *m_boardIterEnd = GO_ENDPOINT;
  {
    int lineIndex = 0;
    for (GoGrid line = 1; line <= (GO_MAX_SIZE / 2) + 1; ++line) {
      m_lineIterAddress[line - 1] = &m_lineIter[lineIndex];
      for (GoPoint p = m_firstBoardPoint; p <= m_lastBoardPoint; ++p) {
        if (m_gridToLine[p] == line)
          m_lineIter[lineIndex++] = p;
      }
      m_lineIter[lineIndex++] = GO_ENDPOINT;
      DBG_ASSERT(lineIndex
                    <= GO_MAX_SIZE * GO_MAX_SIZE + (GO_MAX_SIZE / 2) + 1);
    }
    DBG_ASSERT(lineIndex == m_size * m_size + (GO_MAX_SIZE / 2) + 1);
  }
  m_cornerIter[0] = Pt(1, 1);
  m_cornerIter[1] = Pt(m_size, 1);
  m_cornerIter[2] = Pt(1, m_size);
  m_cornerIter[3] = Pt(m_size, m_size);
  m_cornerIter[4] = GO_ENDPOINT;

  m_sideExtensions = m_line[2 - 1] | m_line[3 - 1] | m_line[4 - 1];
  m_sideExtensions.Exclude(Pt(2, 2));
  m_sideExtensions.Exclude(Pt(2, m_size + 1 - 2));
  m_sideExtensions.Exclude(Pt(m_size + 1 - 2, 2));
  m_sideExtensions.Exclude(Pt(m_size + 1 - 2, m_size + 1 - 2));
  if (m_size > 2) {
    m_sideExtensions.Exclude(Pt(3, 3));
    m_sideExtensions.Exclude(Pt(3, m_size + 1 - 3));
    m_sideExtensions.Exclude(Pt(m_size + 1 - 3, 3));
    m_sideExtensions.Exclude(Pt(m_size + 1 - 3, m_size + 1 - 3));
    if (m_size > 3) {
      m_sideExtensions.Exclude(Pt(4, 4));
      m_sideExtensions.Exclude(Pt(4, m_size + 1 - 4));
      m_sideExtensions.Exclude(Pt(m_size + 1 - 4, 4));
      m_sideExtensions.Exclude(Pt(m_size + 1 - 4, m_size + 1 - 4));
    }
  }
}

GoArray<boost::shared_ptr<GoBoardConst::BoardConstImpl>, GO_MAX_SIZE + 1> GoBoardConst::s_const;

void GoBoardConst::Create(GoGrid size) {
  DBG_ASSERT_GRIDRANGE(size);
  if (!s_const[size])
    s_const[size] =
        boost::shared_ptr<BoardConstImpl>(new BoardConstImpl(size));
  m_const = s_const[size];
  DBG_ASSERT(m_const);
}

GoBoardConst::GoBoardConst(GoGrid size) {
  DBG_ASSERT_GRIDRANGE(size);
  Create(size);
}

void GoBoardConst::ChangeSize(GoGrid newSize) {
  DBG_ASSERT_GRIDRANGE(newSize);
  DBG_ASSERT(m_const);
  GoGrid oldSize = m_const->m_size;
  if (newSize != oldSize)
    Create(newSize);
}

