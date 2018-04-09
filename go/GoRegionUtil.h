

#ifndef GO_REGIONUTIL_H
#define GO_REGIONUTIL_H

#include "board/GoBlackWhite.h"
#include "lib/SgVector.h"
#include "board/GoPoint.h"

class GoBoard;
class GoPointSet;
namespace GoRegionUtil {

bool StaticIs1VitalAndConnected(const GoBoard& board,
                                const GoPointSet& pts,
                                SgBlackWhite color);
bool Has2SureLiberties(const GoBoard& board, const GoPointSet& pts,
                       SgBlackWhite color,
                       const SgVector<GoPoint>& boundaryAnchors);
bool Has2IPorEyes(const GoBoard& board, const GoPointSet& pts,
                  SgBlackWhite color,
                  const SgVector<GoPoint>& boundaryAnchors);
bool IsSingleBlock(const GoBoard& board, const GoPointSet& pts,
                   SgBlackWhite color);
bool IsSmallRegion(const GoBoard& board, const GoPointSet& pts,
                   SgBlackWhite opp);
void FindCurrentAnchors(const GoBoard& board,
                        const SgVector<GoPoint>& origAnchors,
                        SgVector<GoPoint>* currentAnchors);

}

#endif
