//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <boost/test/auto_unit_test.hpp>
#include "board/GoPointSet.h"

using namespace std;
using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgPointSetTest_AllAdjacentTo) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  BOOST_CHECK(!b.AllAdjacentTo(a));
  b.Exclude(Pt(1, 1));
  b.Include(Pt(1, GO_MAX_SIZE));
  BOOST_CHECK(!b.AllAdjacentTo(a));
  b.Clear();
  b.Include(Pt(1, 2));
  BOOST_CHECK(b.AllAdjacentTo(a));
  b.Include(Pt(3, 2));
  BOOST_CHECK(b.AllAdjacentTo(a));
  b.Include(Pt(5, 5));
  BOOST_CHECK(!b.AllAdjacentTo(a));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Adjacent) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  BOOST_CHECK(!b.Adjacent(a));
  b.Include(Pt(1, GO_MAX_SIZE));
  BOOST_CHECK(!b.Adjacent(a));
  b.Include(Pt(1, 2));
  BOOST_CHECK(b.Adjacent(a));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_AdjacentOnlyTo) {
  const int LAST = GO_MAX_SIZE;
  const int SECOND_LAST = LAST - 1;
  GoPointSet a;
  a.Include(Pt(LAST, 1));
  a.Include(Pt(LAST, 2));
  GoPointSet b;
  b.Include(Pt(LAST, 1));
  b.Include(Pt(LAST, 2));
  b.Include(Pt(LAST, 3));
  b.Include(Pt(SECOND_LAST, 1));
  BOOST_CHECK(!a.AdjacentOnlyTo(b, LAST));
  b.Include(Pt(SECOND_LAST, 2));
  BOOST_CHECK(a.AdjacentOnlyTo(b, LAST));
  b.Include(Pt(SECOND_LAST, 3));
  BOOST_CHECK(a.AdjacentOnlyTo(b, LAST));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_AdjacentTo) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  BOOST_CHECK(!a.AdjacentTo(Pt(1, 1)));
  BOOST_CHECK(!a.AdjacentTo(Pt(1, GO_MAX_SIZE)));
  BOOST_CHECK(a.AdjacentTo(Pt(1, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Adjacent8To) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  BOOST_CHECK(!a.Adjacent8To(Pt(1, GO_MAX_SIZE)));
  BOOST_CHECK(a.Adjacent8To(Pt(1, 1)));
  BOOST_CHECK(a.Adjacent8To(Pt(1, 2)));
  BOOST_CHECK(a.Adjacent8To(Pt(3, 3)));
}

void SgPointSetTestAllPointsAtSize(int boardSize) {
  const GoPointSet &s = GoPointSet::AllPoints(boardSize);
  BOOST_CHECK_EQUAL(s.Size(), boardSize * boardSize);
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_AllPoints) {
  SgPointSetTestAllPointsAtSize(GO_MIN_SIZE);
  SgPointSetTestAllPointsAtSize(9);
  if (GO_MAX_SIZE >= 10)
    SgPointSetTestAllPointsAtSize(10);
  SgPointSetTestAllPointsAtSize(GO_MAX_SIZE);
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_And) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  GoPointSet c(a & b);
  BOOST_CHECK_EQUAL(c.Size(), 1);
  BOOST_CHECK(c.Contains(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_AndAssign) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  a &= b;
  BOOST_CHECK_EQUAL(a.Size(), 1);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Assign) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(3, 3));
  b = a;
  BOOST_CHECK_EQUAL(b.Size(), 2);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
  BOOST_CHECK(a.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Border) {
  GoPointSet a;
  a.Include(Pt(GO_MAX_SIZE, 1));
  a.Include(Pt(GO_MAX_SIZE, 2));
  GoPointSet b = a.Border(GO_MAX_SIZE);
  BOOST_CHECK_EQUAL(b.Size(), 3);
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE, 3)));
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE - 1, 1)));
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE - 1, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Border8) {
  GoPointSet a;
  a.Include(Pt(GO_MAX_SIZE, 1));
  a.Include(Pt(GO_MAX_SIZE, 2));
  GoPointSet b = a.Border8(GO_MAX_SIZE);
  BOOST_CHECK_EQUAL(b.Size(), 4);
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE, 3)));
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE - 1, 1)));
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE - 1, 2)));
  BOOST_CHECK(b.Contains(Pt(GO_MAX_SIZE - 1, 3)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Center) {
  GoPointSet a;
  a.Include(Pt(GO_MAX_SIZE, 1));
  a.Include(Pt(GO_MAX_SIZE, 2));
  a.Include(Pt(GO_MAX_SIZE, 3));
  BOOST_CHECK_EQUAL(a.Center(), Pt(GO_MAX_SIZE, 2));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Clear) {
  GoPointSet a;
  a.Include(Pt(GO_MAX_SIZE, 1));
  a.Include(Pt(GO_MAX_SIZE, 2));
  a.Include(Pt(GO_MAX_SIZE, 3));
  a.Clear();
  BOOST_CHECK_EQUAL(a.Size(), 0);
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Component) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(1, 2));
  a.Include(Pt(2, 1));
  a.Include(Pt(1, GO_MAX_SIZE));
  GoPointSet b = a.Component(Pt(1, 1));
  BOOST_CHECK_EQUAL(b.Size(), 3);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
  BOOST_CHECK(a.Contains(Pt(1, 2)));
  BOOST_CHECK(a.Contains(Pt(2, 1)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_ConnComp) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(1, 2));
  a.Include(Pt(2, 1));
  a.Include(Pt(1, GO_MAX_SIZE));
  GoPointSet b = a.ConnComp(Pt(1, 1));
  BOOST_CHECK_EQUAL(b.Size(), 3);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
  BOOST_CHECK(a.Contains(Pt(1, 2)));
  BOOST_CHECK(a.Contains(Pt(2, 1)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Disjoint) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  BOOST_CHECK(a.Disjoint(b));
  b.Include(Pt(3, 3));
  BOOST_CHECK(a.Disjoint(b));
  b.Include(Pt(2, 2));
  BOOST_CHECK(!a.Disjoint(b));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_EnclosingRect) {
  GoPointSet a;
  a.Include(Pt(GO_MAX_SIZE, 1));
  a.Include(Pt(GO_MAX_SIZE, 3));
  a.Include(Pt(GO_MAX_SIZE - 1, 2));
  BOOST_CHECK_EQUAL(a.EnclosingRect(),
                    GoRect(GO_MAX_SIZE - 1, GO_MAX_SIZE, 1, 3));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Equals) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  b.Include(Pt(2, 2));
  BOOST_CHECK(a == b);
  BOOST_CHECK(!(a != b));
  b.Exclude(Pt(2, 2));
  BOOST_CHECK(!(a == b));
  BOOST_CHECK(a != b);
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Grow) {
  GoPointSet a;
  a.Include(Pt(8, 1));
  a.Include(Pt(8, 2));
  a.Include(Pt(9, 1));
  a.Include(Pt(9, 2));
  a.Grow(9);
  BOOST_CHECK_EQUAL(a.Size(), 8);
  BOOST_CHECK(a.Contains(Pt(7, 1)));
  BOOST_CHECK(a.Contains(Pt(7, 2)));
  BOOST_CHECK(a.Contains(Pt(8, 1)));
  BOOST_CHECK(a.Contains(Pt(8, 2)));
  BOOST_CHECK(a.Contains(Pt(8, 3)));
  BOOST_CHECK(a.Contains(Pt(9, 1)));
  BOOST_CHECK(a.Contains(Pt(9, 2)));
  BOOST_CHECK(a.Contains(Pt(9, 3)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_GrowNewArea) {
  GoPointSet a;
  a.Include(Pt(8, 1));
  a.Include(Pt(8, 2));
  a.Include(Pt(9, 1));
  a.Include(Pt(9, 2));
  GoPointSet newArea;
  a.Grow(&newArea, 9);
  BOOST_CHECK_EQUAL(a.Size(), 8);
  BOOST_CHECK(a.Contains(Pt(7, 1)));
  BOOST_CHECK(a.Contains(Pt(7, 2)));
  BOOST_CHECK(a.Contains(Pt(8, 1)));
  BOOST_CHECK(a.Contains(Pt(8, 2)));
  BOOST_CHECK(a.Contains(Pt(8, 3)));
  BOOST_CHECK(a.Contains(Pt(9, 1)));
  BOOST_CHECK(a.Contains(Pt(9, 2)));
  BOOST_CHECK(a.Contains(Pt(9, 3)));
  BOOST_CHECK_EQUAL(newArea.Size(), 4);
  BOOST_CHECK(newArea.Contains(Pt(7, 1)));
  BOOST_CHECK(newArea.Contains(Pt(7, 2)));
  BOOST_CHECK(newArea.Contains(Pt(8, 3)));
  BOOST_CHECK(newArea.Contains(Pt(9, 3)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Grow8) {
  GoPointSet a;
  a.Include(Pt(8, 1));
  a.Include(Pt(8, 2));
  a.Include(Pt(9, 1));
  a.Include(Pt(9, 2));
  a.Grow8(9);
  BOOST_CHECK_EQUAL(a.Size(), 9);
  BOOST_CHECK(a.Contains(Pt(7, 1)));
  BOOST_CHECK(a.Contains(Pt(7, 2)));
  BOOST_CHECK(a.Contains(Pt(7, 3)));
  BOOST_CHECK(a.Contains(Pt(8, 1)));
  BOOST_CHECK(a.Contains(Pt(8, 2)));
  BOOST_CHECK(a.Contains(Pt(8, 3)));
  BOOST_CHECK(a.Contains(Pt(9, 1)));
  BOOST_CHECK(a.Contains(Pt(9, 2)));
  BOOST_CHECK(a.Contains(Pt(9, 3)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_IsCloseTo) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  BOOST_CHECK(a.IsCloseTo(Pt(1, 1)));
  BOOST_CHECK(a.IsCloseTo(Pt(1, 2)));
  BOOST_CHECK(a.IsCloseTo(Pt(1, 3)));
  BOOST_CHECK(a.IsCloseTo(Pt(1, 4)));
  BOOST_CHECK(!a.IsCloseTo(Pt(1, 5)));
  BOOST_CHECK(a.IsCloseTo(Pt(1, 1)));
  BOOST_CHECK(a.IsCloseTo(Pt(2, 1)));
  BOOST_CHECK(a.IsCloseTo(Pt(3, 1)));
  BOOST_CHECK(a.IsCloseTo(Pt(4, 1)));
  BOOST_CHECK(!a.IsCloseTo(Pt(5, 1)));
  BOOST_CHECK(a.IsCloseTo(Pt(4, 4)));
  BOOST_CHECK(!a.IsCloseTo(Pt(5, 5)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_IsConnected) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(1, 2));
  a.Include(Pt(2, 1));
  BOOST_CHECK(a.IsConnected());
  a.Include(Pt(1, GO_MAX_SIZE));
  BOOST_CHECK(!a.IsConnected());
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_IsEmpty) {
  GoPointSet a;
  BOOST_CHECK(a.IsEmpty());
  a.Include(Pt(1, 1));
  BOOST_CHECK(!a.IsEmpty());
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Kernel) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(1, 2));
  a.Include(Pt(1, 3));
  a.Include(Pt(2, 1));
  a.Include(Pt(2, 2));
  a.Include(Pt(2, 3));
  a.Include(Pt(3, 1));
  a.Include(Pt(3, 2));
  a.Include(Pt(3, 3));
  GoPointSet k = a.Kernel(GO_MAX_SIZE);
  BOOST_CHECK_EQUAL(k.Size(), 4);
  BOOST_CHECK(k.Contains(Pt(1, 1)));
  BOOST_CHECK(k.Contains(Pt(1, 2)));
  BOOST_CHECK(k.Contains(Pt(2, 1)));
  BOOST_CHECK(k.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_MaxOverlap) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(2, 2));
  b.Include(Pt(3, 3));
  BOOST_CHECK(!a.MaxOverlap(b, 0));
  BOOST_CHECK(a.MaxOverlap(b, 1));
  BOOST_CHECK(a.MaxOverlap(b, 2));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_MinOverlap) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(2, 2));
  b.Include(Pt(3, 3));
  BOOST_CHECK(a.MinOverlap(b, 0));
  BOOST_CHECK(a.MinOverlap(b, 1));
  BOOST_CHECK(!a.MinOverlap(b, 2));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Minus) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  GoPointSet c(a - b);
  BOOST_CHECK_EQUAL(c.Size(), 1);
  BOOST_CHECK(c.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_MinusAssign) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  a -= b;
  BOOST_CHECK_EQUAL(a.Size(), 1);
  BOOST_CHECK(a.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Or) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  GoPointSet c(a | b);
  BOOST_CHECK_EQUAL(c.Size(), 2);
  BOOST_CHECK(c.Contains(Pt(1, 1)));
  BOOST_CHECK(c.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_OrAssign) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  a |= b;
  BOOST_CHECK_EQUAL(a.Size(), 2);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
  BOOST_CHECK(a.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Overlaps) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(3, 3));
  BOOST_CHECK(!a.Overlaps(b));
  BOOST_CHECK(!b.Overlaps(a));
  b.Include(Pt(2, 2));
  BOOST_CHECK(a.Overlaps(b));
  BOOST_CHECK(b.Overlaps(a));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_PointOf) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  BOOST_CHECK_EQUAL(a.PointOf(), Pt(1, 1));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_SubsetOf) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  BOOST_CHECK(b.SubsetOf(a));
  b.Include(Pt(1, 1));
  BOOST_CHECK(b.SubsetOf(a));
  b.Include(Pt(2, 2));
  BOOST_CHECK(b.SubsetOf(a));
  b.Include(Pt(3, 3));
  BOOST_CHECK(!b.SubsetOf(a));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_SgPointSet_SgVector) {
  SgVector<GoPoint> a;
  GoPointSet b(a);
  BOOST_CHECK_EQUAL(b.Size(), 0);
  a.PushBack(Pt(1, 1));
  a.PushBack(Pt(2, 2));
  a.PushBack(Pt(3, 3));
  GoPointSet c(a);
  BOOST_CHECK_EQUAL(c.Size(), 3);
  SgVector<GoPoint> d;
  BOOST_CHECK_EQUAL(d.Length(), 0);
  c.ToVector(&d);
  BOOST_CHECK_EQUAL(d.Length(), 3);
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_SupersetOf) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  BOOST_CHECK(!b.SupersetOf(a));
  b.Include(Pt(1, 1));
  BOOST_CHECK(!b.SupersetOf(a));
  b.Include(Pt(2, 2));
  BOOST_CHECK(b.SupersetOf(a));
  b.Include(Pt(3, 3));
  BOOST_CHECK(b.SupersetOf(a));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Swap) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(3, 3));
  b.Include(Pt(4, 4));
  b.Include(Pt(5, 5));
  a.Swap(b);
  BOOST_CHECK_EQUAL(a.Size(), 3);
  BOOST_REQUIRE(a.Size() == 3);
  BOOST_CHECK(a.Contains(Pt(3, 3)));
  BOOST_CHECK(a.Contains(Pt(4, 4)));
  BOOST_CHECK(a.Contains(Pt(5, 5)));
  BOOST_CHECK_EQUAL(b.Size(), 2);
  BOOST_REQUIRE(b.Size() == 2);
  BOOST_CHECK(b.Contains(Pt(1, 1)));
  BOOST_CHECK(b.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Write) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  ostringstream out;
  a.Write(out, 5);
  BOOST_CHECK_EQUAL(out.str(),
                    "-----\n"
                        "-----\n"
                        "-----\n"
                        "-@---\n"
                        "@----\n");
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_Xor) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  GoPointSet c(a ^ b);
  BOOST_CHECK_EQUAL(c.Size(), 1);
  BOOST_CHECK(c.Contains(Pt(2, 2)));
}

BOOST_AUTO_TEST_CASE(SgPointSetTest_XorAssign) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(2, 2));
  GoPointSet b;
  b.Include(Pt(1, 1));
  a ^= b;
  BOOST_CHECK_EQUAL(a.Size(), 1);
  BOOST_CHECK(a.Contains(Pt(2, 2)));
}

}

