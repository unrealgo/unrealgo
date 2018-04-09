
#ifndef GO_SORTEDMOVES_H
#define GO_SORTEDMOVES_H

#include "board/GoPoint.h"
#include "SgSortedMoves.h"

#define GO_SORTED_MOVES_DEFAULT 3
#define GO_SORTED_MOVES_MAX 20

class GoSortedMoves :
    public SgSortedMoves<GoMove, int, GO_SORTED_MOVES_MAX> {
 public:
  explicit GoSortedMoves(int maxNuMoves) :
      SgSortedMoves<GoMove, int, GO_SORTED_MOVES_MAX>(maxNuMoves) {
    Clear();
  }

  GoSortedMoves() :
      SgSortedMoves<GoMove, int,
                    GO_SORTED_MOVES_MAX>(GO_SORTED_MOVES_DEFAULT) {
    Clear();
  }

  void Clear() {
    SgSortedMoves<GoMove, int, GO_SORTED_MOVES_MAX>::Clear();
    SetInitLowerBound(1);
    SetLowerBound(1);
  }
};

#endif

