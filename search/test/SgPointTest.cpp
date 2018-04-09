//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include "SgAutoUnitTest.h"
#include "board/GoPoint.h"

using namespace std;
using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgPointTest_Col) {
  BOOST_CHECK_EQUAL(GoPointUtil::Col(GoPointUtil::Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(GoPointUtil::Col(GoPointUtil::Pt(5, 3)), 5);
  if (GO_MAX_SIZE >= 19) {
    BOOST_CHECK_EQUAL(GoPointUtil::Col(GoPointUtil::Pt(19, 3)), 19);
    BOOST_CHECK_EQUAL(GoPointUtil::Col(GoPointUtil::Pt(19, 19)), 19);
  }
}

BOOST_AUTO_TEST_CASE(SgPointTest_Pt) {
  if (GO_MAX_SIZE == 19) {
    BOOST_CHECK_EQUAL(GoPointUtil::Pt(19, 19), 399);
    BOOST_CHECK_EQUAL(GoPointUtil::Pt(1, 1), 21);
  }
}

BOOST_AUTO_TEST_CASE(SgPointTest_Point2Index) {
  if (GO_MAX_SIZE == 19) {
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(1, 1)), 0);
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(2, 1)), 1);
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(3, 1)), 2);
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(1, 2)), 19);
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(1, 3)), 38);
    BOOST_CHECK_EQUAL(GoPointUtil::Point2Index(Pt(19, 19)), 360);
  }
}

BOOST_AUTO_TEST_CASE(SgPointTest_Row) {
  BOOST_CHECK_EQUAL(GoPointUtil::Row(GoPointUtil::Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(GoPointUtil::Row(GoPointUtil::Pt(5, 3)), 3);
  if (GO_MAX_SIZE >= 19) {
    BOOST_CHECK_EQUAL(GoPointUtil::Row(GoPointUtil::Pt(19, 3)), 3);
    BOOST_CHECK_EQUAL(GoPointUtil::Row(GoPointUtil::Pt(19, 19)), 19);
  }
}

void SgReadPointTest(GoPoint point, const char *s) {
  istringstream in(s);
  GoPoint p;
  in >> GoReadPoint(p);
  BOOST_REQUIRE(!in.fail());
  BOOST_CHECK_EQUAL(p, point);
}


BOOST_AUTO_TEST_CASE(SgReadPointTest_AllValid) {
  SgReadPointTest(Pt(1, 1), "A1");
  SgReadPointTest(Pt(8, 1), "H1");
  SgReadPointTest(Pt(9, 1), "J1");
  if (GO_MAX_SIZE >= 19)
    SgReadPointTest(Pt(19, 19), "T19");
  SgReadPointTest(GO_PASS, "PASS");
}


BOOST_AUTO_TEST_CASE(SgReadPointTest_Invalid) {
  istringstream in("123");
  GoPoint p;
  in >> GoReadPoint(p);
  BOOST_CHECK(!in);
}


BOOST_AUTO_TEST_CASE(SgReadPointTest_Two) {
  istringstream in("A1 B2");
  GoPoint p1;
  GoPoint p2;
  in >> GoReadPoint(p1) >> GoReadPoint(p2);
  BOOST_REQUIRE(in);
  BOOST_CHECK_EQUAL(p1, Pt(1, 1));
  BOOST_CHECK_EQUAL(p2, Pt(2, 2));
}

void SgWritePointTest(GoPoint p, const char *s) {
  ostringstream out;
  out << GoWritePoint(p);
  BOOST_CHECK_EQUAL(out.str(), s);
}

BOOST_AUTO_TEST_CASE(SgWritePointTestAll) {
  SgWritePointTest(Pt(1, 1), "A1");
  SgWritePointTest(Pt(8, 1), "H1");
  SgWritePointTest(Pt(9, 1), "J1");
  if (GO_MAX_SIZE >= 19)
    SgWritePointTest(Pt(19, 19), "T19");
  SgWritePointTest(GO_PASS, "PASS");
}

void SgWriteMoveTest(GoPoint p, SgBlackWhite color, const char *s) {
  ostringstream out;
  out << GoWriteMove(p, color);
  BOOST_CHECK_EQUAL(out.str(), s);
}

BOOST_AUTO_TEST_CASE(SgWriteMoveTestAll) {
  SgWriteMoveTest(Pt(1, 1), SG_BLACK, "B A1 ");
  SgWriteMoveTest(Pt(1, 1), SG_WHITE, "W A1 ");
  SgWriteMoveTest(Pt(8, 1), SG_BLACK, "B H1 ");
  SgWriteMoveTest(Pt(8, 1), SG_WHITE, "W H1 ");
  SgWriteMoveTest(Pt(9, 1), SG_BLACK, "B J1 ");
  SgWriteMoveTest(Pt(9, 1), SG_WHITE, "W J1 ");
  if (GO_MAX_SIZE >= 19) {
    SgWriteMoveTest(Pt(19, 19), SG_BLACK, "B T19 ");
    SgWriteMoveTest(Pt(19, 19), SG_WHITE, "W T19 ");
  }
  SgWriteMoveTest(GO_PASS, SG_BLACK, "B PASS ");
  SgWriteMoveTest(GO_PASS, SG_WHITE, "W PASS ");
}

BOOST_AUTO_TEST_CASE(SgPointUtilTest_AreAdjacent) {
  using GoPointUtil::AreAdjacent;
  BOOST_CHECK(AreAdjacent(Pt(1, 1), Pt(2, 1)));
  BOOST_CHECK(AreAdjacent(Pt(2, 1), Pt(1, 1)));
  BOOST_CHECK(AreAdjacent(Pt(1, 1), Pt(1, 2)));
  BOOST_CHECK(AreAdjacent(Pt(1, 2), Pt(1, 1)));
  BOOST_CHECK(!AreAdjacent(Pt(1, 1), Pt(2, 2)));
  BOOST_CHECK(!AreAdjacent(Pt(1, 1), Pt(1, 3)));
  BOOST_CHECK(!AreAdjacent(Pt(1, 1), Pt(1, GO_MAX_SIZE)));
}

BOOST_AUTO_TEST_CASE(SgPointUtilTest_Rotate) {
  using GoPointUtil::Rotate;
  BOOST_CHECK_EQUAL(Rotate(0, Pt(1, 2), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(1, Pt(1, 2), 9), Pt(9, 2));
  BOOST_CHECK_EQUAL(Rotate(2, Pt(1, 2), 9), Pt(1, 8));
  BOOST_CHECK_EQUAL(Rotate(3, Pt(1, 2), 9), Pt(2, 1));
  BOOST_CHECK_EQUAL(Rotate(4, Pt(1, 2), 9), Pt(8, 1));
  BOOST_CHECK_EQUAL(Rotate(5, Pt(1, 2), 9), Pt(2, 9));
  BOOST_CHECK_EQUAL(Rotate(6, Pt(1, 2), 9), Pt(9, 8));
  BOOST_CHECK_EQUAL(Rotate(7, Pt(1, 2), 9), Pt(8, 9));
}

BOOST_AUTO_TEST_CASE(SgPointUtilTest_InvRotation) {
  using GoPointUtil::Rotate;
  using GoPointUtil::InvRotation;
  BOOST_CHECK_EQUAL(Rotate(InvRotation(0), Pt(1, 2), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(1), Pt(9, 2), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(2), Pt(1, 8), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(3), Pt(2, 1), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(4), Pt(8, 1), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(5), Pt(2, 9), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(6), Pt(9, 8), 9), Pt(1, 2));
  BOOST_CHECK_EQUAL(Rotate(InvRotation(7), Pt(8, 9), 9), Pt(1, 2));
}

}


