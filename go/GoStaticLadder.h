

#ifndef GO_STATICLADDER_H
#define GO_STATICLADDER_H

#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"

class GoBoard;
namespace GoStaticLadder {

bool IsEdgeLadder(const GoBoard& bd, GoPoint target, SgBlackWhite toPlay);
bool IsLadder(const GoBoard& bd, GoPoint target, SgBlackWhite toPlay);
}

#endif
