

#ifndef GO_INFLUENCE_H
#define GO_INFLUENCE_H

#include "GoBoard.h"
#include "board/GoPointArray.h"

namespace GoInfluence {

const int DISTANCE_INFINITE = 99;
void FindDistanceToStones(const GoBoard& board, SgBlackWhite color,
                          GoPointArray<int>& distance);
void FindInfluence(const GoBoard& board, int nuExpand, int nuShrink,
                   GoBWSet* result);
int Influence(const GoBoard& board, SgBlackWhite color, int nuExpand,
              int nuShrink);
void ComputeInfluence(const GoBoard& board, const GoBWSet& stopPts,
                      SgBWArray<GoPointArray<int> >* influence);

}

#endif

