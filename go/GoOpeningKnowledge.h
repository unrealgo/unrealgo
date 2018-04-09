

#ifndef GO_OPENING_KNOWLEDGE_H
#define GO_OPENING_KNOWLEDGE_H

#include <vector>
#include "GoBoard.h"
#include "board/GoPoint.h"

namespace GoOpeningKnowledge {
typedef std::pair<GoPoint, int> MoveBonusPair;
std::vector<GoPoint> FindCornerMoves(const GoBoard& bd);
std::vector<MoveBonusPair> FindSideExtensions(const GoBoard& bd);

}

#endif
