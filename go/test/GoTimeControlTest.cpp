

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "GoBoard.h"
#include "GoTimeControl.h"
#include "platform/SgDebug.h"
#include "SgTimeRecord.h"

using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(GoTimeControlTest_AbsoluteTime) {
  if (GO_MAX_SIZE >= 19) {
    GoSetup setup;
    for (int row = 1; row <= 19; row += 2)
      for (int col = 1; col <= 19; ++col)
        setup.AddBlack(Pt(row, col));
    for (int row = 2; row <= 18; row += 2)
      for (int col = 1; col <= 19; col += 3)
        setup.AddBlack(Pt(row, col));
    GoBoard bd(19, setup);
    GoTimeControl timeControl(bd);
    timeControl.SetFastOpenMoves(0);
    timeControl.SetFinalSpace(0.7f);

    SgTimeRecord timeRecord;
    double timeLeft = 10;
    timeRecord.SetTimeLeft(SG_BLACK, timeLeft);
    timeRecord.SetOTNumMoves(0);
    double timeForMove = timeControl.TimeForCurrentMove(timeRecord,
                                                        true);
    BOOST_CHECK_MESSAGE(timeForMove < 0.5 * timeLeft,
                        "timeForMove is " << timeForMove);
  }
}

BOOST_AUTO_TEST_CASE(GoTimeControlTest_EmptyBoard) {
  if (GO_MAX_SIZE >= 19) {
    GoBoard bd(19);
    GoTimeControl timeControl(bd);
    timeControl.SetFastOpenMoves(0);
    SgTimeRecord timeRecord;
    double timeLeft = 100;
    timeRecord.SetTimeLeft(SG_BLACK, timeLeft);
    timeRecord.SetOTNumMoves(0);
    double timeForMove = timeControl.TimeForCurrentMove(timeRecord,
                                                        true);
    BOOST_CHECK_CLOSE(timeForMove, 0.7407, 1e-2);
  }
}

}
