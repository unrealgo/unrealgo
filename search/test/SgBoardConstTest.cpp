//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <boost/test/auto_unit_test.hpp>
#include "board/GoBoardConst.h"

using namespace std;
using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgBoardConstTest_Line) {
  GoBoardConst boardConst(8);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(1, 1) - GO_WEST_EAST), 0);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(1, 8)), 1);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(8, 1)), 1);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(8, 8)), 1);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(2, 5)), 2);
  BOOST_CHECK_EQUAL(boardConst.Line(Pt(5, 5)), 4);
}

BOOST_AUTO_TEST_CASE(SgBoardConstTest_Pos) {
  GoBoardConst boardConst(8);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(1, 1) - GO_WEST_EAST), 0);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(1, 8)), 1);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(8, 1)), 1);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(8, 8)), 1);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(2, 5)), 4);
  BOOST_CHECK_EQUAL(boardConst.Pos(Pt(5, 5)), 4);
}

BOOST_AUTO_TEST_CASE(SgBoardConstTest_Size) {
  if (GO_MAX_SIZE >= 19)
    BOOST_CHECK_EQUAL(GoBoardConst(19).Size(), 19);
  BOOST_CHECK_EQUAL(GoBoardConst(6).Size(), 6);
}

BOOST_AUTO_TEST_CASE(SgNbIteratorTest_Corner) {
  if (GO_MAX_SIZE >= 19) {
    GoBoardConst boardConst(19);
    SgNbIterator it(boardConst, Pt(1, 1));
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(2, 1));
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(1, 2));
    ++it;
    BOOST_CHECK(!it);
  }
}

BOOST_AUTO_TEST_CASE(SgNbIteratorTest_Edge) {
  GoBoardConst boardConst(9);
  SgNbIterator it(boardConst, Pt(9, 5));
  BOOST_CHECK(it);
  BOOST_CHECK_EQUAL(*it, Pt(9, 4));
  ++it;
  BOOST_CHECK(it);
  BOOST_CHECK_EQUAL(*it, Pt(8, 5));
  ++it;
  BOOST_CHECK(it);
  BOOST_CHECK_EQUAL(*it, Pt(9, 6));
  ++it;
  BOOST_CHECK(!it);
}

BOOST_AUTO_TEST_CASE(SgNbIteratorTest_Middle) {
  if (GO_MAX_SIZE >= 13) {
    GoBoardConst boardConst(13);
    SgNbIterator it(boardConst, Pt(3, 3));
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(3, 2));
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(2, 3));
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(4, 3));
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, Pt(3, 4));
    ++it;
    BOOST_CHECK(!it);
  }
}

}

