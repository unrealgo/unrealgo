
#ifndef GOUCT_ESTIMATORSTAT_H
#define GOUCT_ESTIMATORSTAT_H

#include <cstddef>
#include <string>
#include <vector>

class GoBoard;
class GoUctSearch;
namespace GoUctEstimatorStat {
void Compute(GoUctSearch &search, std::size_t trueValueMaxGames,
             std::size_t maxGames, std::size_t stepSize,
             const std::string &fileName);
}

#endif
