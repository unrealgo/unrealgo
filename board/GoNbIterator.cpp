
#include "platform/SgSystem.h"
#include "board/GoNbIterator.h"

using namespace std;

SgNb4Iterator::Precomp::Precomp() {
  for (GoGrid row = 1; row <= GO_MAX_SIZE; ++row)
    for (GoGrid col = 1; col <= GO_MAX_SIZE; ++col) {
      GoPoint p = GoPointUtil::Pt(col, row);
      m_nb[p][0] = p - GO_NORTH_SOUTH;
      m_nb[p][1] = p - GO_WEST_EAST;
      m_nb[p][2] = p + GO_WEST_EAST;
      m_nb[p][3] = p + GO_NORTH_SOUTH;
    }
}

const SgNb4Iterator::Precomp SgNb4Iterator::s_precomp;


const int SgNb4DiagIterator::s_diag_offset[4] = {
    -GO_NORTH_SOUTH - GO_WEST_EAST,
    -GO_NORTH_SOUTH + GO_WEST_EAST,
    +GO_NORTH_SOUTH - GO_WEST_EAST,
    +GO_NORTH_SOUTH + GO_WEST_EAST
};



const int SgNb8Iterator::s_nb8_offset[8] = {
    -GO_NORTH_SOUTH - GO_WEST_EAST,
    -GO_NORTH_SOUTH,
    -GO_NORTH_SOUTH + GO_WEST_EAST,
    -GO_WEST_EAST,
    +GO_WEST_EAST,
    +GO_NORTH_SOUTH - GO_WEST_EAST,
    +GO_NORTH_SOUTH,
    +GO_NORTH_SOUTH + GO_WEST_EAST
};

