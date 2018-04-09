//----------------------------------------------------------------------------
/** @file GoUctBoardTest.cpp
    Unit tests for GoUctBoard. */
//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "GoUctBoard.h"

using GoPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(GoUctBoardTest_Constant) {
  /** North-South: offset of vertical neighbors. */
  BOOST_CHECK_EQUAL(GO_NORTH_SOUTH, GO_MAX_SIZE + 1);
  BOOST_CHECK_EQUAL(GO_MAXPOINT, GO_MAX_SIZE * GO_MAX_SIZE + 3 * (GO_MAX_SIZE + 1));
}

BOOST_AUTO_TEST_CASE(GoUctBoardTest_Point) {
  BOOST_CHECK_EQUAL(Pt(1, 1), GO_NORTH_SOUTH * 1 + 1);
  BOOST_CHECK_EQUAL(Pt(2, 1), GO_NORTH_SOUTH * 1 + 2);
  BOOST_CHECK_EQUAL(Pt(1, 2), GO_NORTH_SOUTH * 2 + 1);
  BOOST_CHECK_EQUAL(Pt(GO_MAX_SIZE, GO_MAX_SIZE), GO_NORTH_SOUTH * GO_MAX_SIZE + GO_MAX_SIZE);
}

/** Copied and adapted from GoBoardTest_GetLastMove.
    Parts removed that use Undo() */
BOOST_AUTO_TEST_CASE(GoUctBoardTest_GetLastMove) {
  GoBoard board(9);
  GoUctBoard bd(board);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Play(Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Play(Pt(2, 2));
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(2, 2));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), Pt(1, 1));
  bd.Play(GO_PASS);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_PASS);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), Pt(2, 2));
}

/** Copied from GoBoardTest_IsLibertyOfBlock */
BOOST_AUTO_TEST_CASE(GoUctBoardTest_IsLibertyOfBlock) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  GoBoard board(9, setup);
  GoUctBoard bd(board);
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(1, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(2, 1))));
  BOOST_CHECK(!bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(3, 2), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(2, 3), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(!bd.IsLibertyOfBlock(Pt(2, 3), bd.Anchor(Pt(1, 2))));
}

} // namespace

//----------------------------------------------------------------------------

