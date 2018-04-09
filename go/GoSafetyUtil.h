

#ifndef GO_SAFETYUTIL_H
#define GO_SAFETYUTIL_H

#include <string>
#include "board/GoBlackWhite.h"
#include "board/GoBoardColor.h"
#include "SgMiaiStrategy.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class GoBoard;
class GoRegion;
class GoRegionBoard;
class GoBWSet;
class GoPointSet;
namespace GoSafetyUtil {

void AddToSafe(const GoBoard& board, const GoPointSet& pts,
               SgBlackWhite color, GoBWSet* safe, const std::string& reason,
               int depth, bool addBoundary);
bool ExtendedIsTerritory(const GoBoard& board, GoRegionBoard* regions,
                         const GoPointSet& pts,
                         const GoPointSet& safe, SgBlackWhite color,
                         std::string& reason);
GoPointSet FindDamePoints(const GoBoard& board, const GoPointSet& empty,
                          const GoBWSet& safe);
void FindDameAndUnsurroundablePoints(const GoBoard& bd,
                                     const GoPointSet& empty,
                                     const GoBWSet& safe,
                                     GoPointSet* dame,
                                     GoPointSet* unsurroundable);
SgEmptyBlackWhite GetWinner(const GoBoard& bd);
bool IsTerritory(const GoBoard& board, const GoPointSet& pts,
                 const GoPointSet& safe, SgBlackWhite color,
                 std::string* reason);
void ReduceToAnchors(const GoBoard& board, const GoPointSet& stones,
                     SgVector<GoPoint>* anchors);
bool Find2Libs(GoPoint p, GoPointSet* libs);
bool Find2BestLibs(GoPoint p, const GoPointSet& libs,
                   GoPointSet interior, SgMiaiPair* miaiPair);
bool ExtendedMightMakeLife(const GoBoard& board, GoRegionBoard* regions,
                           const GoPointSet& area, const GoPointSet& safe,
                           SgBlackWhite color);
bool MightMakeLife(const GoBoard& board, const GoPointSet& area,
                   const GoPointSet& safe, SgBlackWhite color);
void WriteStatistics(const std::string& heading,
                     const GoRegionBoard* regions,
                     const GoBWSet* safe);

}

#endif
