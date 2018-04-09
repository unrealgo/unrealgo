

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include <algorithm>
#include "GoOpeningKnowledge.h"

#include "GoBoard.h"
#include "GoSetupUtil.h"

using GoPointUtil::Pt;
using namespace GoOpeningKnowledge;

namespace {

BOOST_AUTO_TEST_CASE(GoOpeningKnowledgeTest_CornerMoves) {
  if (GO_MAX_SIZE == 19) {
    GoBoard bd(GO_MAX_SIZE);
    std::vector<GoPoint> moves(FindCornerMoves(bd));

    BOOST_CHECK_EQUAL(moves.size(), 4 * 8u);
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(3, 3)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(3, 4)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(3, 5)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(4, 3)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(4, 4)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(4, 5)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(5, 3)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(5, 4)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(5, 5)) ==
        moves.end());

    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(17, 3)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(3, 17)) !=
        moves.end());
    BOOST_CHECK(std::find(moves.begin(), moves.end(), Pt(17, 17)) !=
        moves.end());
    std::sort(moves.begin(), moves.end());
    const std::vector<GoPoint>::iterator last =
        std::unique(moves.begin(), moves.end());
    BOOST_CHECK(moves.end() == last);
  }
}

BOOST_AUTO_TEST_CASE(GoOpeningKnowledgeTest_SideExtensions) {
  if (GO_MAX_SIZE == 19) {
    GoBoard bd(GO_MAX_SIZE);
    std::vector<MoveBonusPair> moves(FindSideExtensions(bd));

    BOOST_CHECK_EQUAL(moves.size(), 13 * 4u);
  }
}

}

