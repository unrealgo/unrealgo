
#include "GoUctEstimatorStat.h"

#include <boost/format.hpp>
#include "GoModBoard.h"
#include "GoUctSearch.h"
#include "UctTreeUtil.h"

using boost::format;

void GoUctEstimatorStat::Compute(GoUctSearch &search,
                                 std::size_t trueValueMaxGames,
                                 std::size_t maxGames,
                                 std::size_t stepSize,
                                 const std::string &fileName) {
  double maxTime = std::numeric_limits<double>::max();
  std::vector<UctMoveInfo> moves;
  search.GenerateAllMoves(moves);
  GoArray<UctValueType, GO_PASS + 1> trueValues;
  for (UctMoveInfo info : moves) {
    GoPoint p = info.uct_move;
    GoModBoard modBoard(search.Board());
    modBoard.Board().Play(p);
    std::vector<GoMove> sequence;
    UctValueType value =
        search.StartSearchThread(UctValueType(trueValueMaxGames), maxTime, sequence);
    trueValues[p] = UctSearch::InverseEstimate(value);
    modBoard.Board().Undo();
  }

  search.PreStartSearch();
  if (search.GetMpiSynchronizer()->IsRootProcess()) {
    std::ofstream out(fileName.c_str(), std::ios::app);
    for (size_t n = 0; n < maxGames; n += stepSize) {
      search.PlayGame();
      for (UctMoveInfo info : moves) {
        GoPoint p = info.uct_move;
        const UctSearchTree &tree = search.Tree();
        const UctNode *child =
            UctTreeUtil::FindChildWithMove(tree, tree.Root(), p);
        if (child == 0)
          continue;
        out << (format("%1$d\t"
                           "%2$.2f\t"
                           "%3$d\t"
                           "%4$.2f\t"
                           "%5$d\t"
                           "%6$.2f\n"
        )
            % n
            % trueValues[p]
            % child->MoveCount()
            % (child->HasMean() ?
               UctSearch::InverseEstimate(child->Mean()) : 0)
            % child->RaveCount()
            % (child->HasRaveValue() ? child->RaveValue() : 0)
        );
      }
    }
  }
  search.EndSearch();
}
