//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "board/GoRect.h"

using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgRectTestAccessors) {
  GoRect rect(2, 5, 7, 9);
  BOOST_CHECK_EQUAL(rect.Left(), 2);
  BOOST_CHECK_EQUAL(rect.Right(), 5);
  BOOST_CHECK_EQUAL(rect.Top(), 7);
  BOOST_CHECK_EQUAL(rect.Bottom(), 9);
}

BOOST_AUTO_TEST_CASE(SgRectTestCenter) {
  GoRect rect(2, 6, 7, 9);
  GoPoint center = rect.Center();
  BOOST_CHECK_EQUAL(GoPointUtil::Col(center), 4);
  BOOST_CHECK_EQUAL(GoPointUtil::Row(center), 8);
}

BOOST_AUTO_TEST_CASE(SgRectTestContains) {
  GoRect rect(2, 5, 7, 9);
  GoRect rect1(1, 4, 7, 9);
  BOOST_CHECK(!rect.Contains(rect1));
  GoRect rect2(4, 11, 7, 9);
  BOOST_CHECK(!rect.Contains(rect2));
  GoRect rect3(2, 5, 3, 9);
  BOOST_CHECK(!rect.Contains(rect3));
  GoRect rect4(2, 5, 7, 14);
  BOOST_CHECK(!rect.Contains(rect4));
  GoRect rect5(2, 5, 7, 9);
  BOOST_CHECK(rect.Contains(rect5));
  GoRect rect6(3, 4, 8, 9);
  BOOST_CHECK(rect.Contains(rect6));
  GoRect rect7(1, 9, 1, 12);
  BOOST_CHECK(!rect.Contains(rect7));
  BOOST_CHECK(rect7.Contains(rect));
}

BOOST_AUTO_TEST_CASE(SgRectTestDefaultConstructor) {
  GoRect rect;
  BOOST_CHECK(rect.IsEmpty());
}

BOOST_AUTO_TEST_CASE(SgRectTestInclude) {
  GoRect rect;
  rect.Include(Pt(1, 8));
  BOOST_CHECK_EQUAL(rect.Left(), 1);
  BOOST_CHECK_EQUAL(rect.Right(), 1);
  BOOST_CHECK_EQUAL(rect.Top(), 8);
  BOOST_CHECK_EQUAL(rect.Bottom(), 8);

  rect.Include(Pt(5, 2));
  BOOST_CHECK_EQUAL(rect.Left(), 1);
  BOOST_CHECK_EQUAL(rect.Right(), 5);
  BOOST_CHECK_EQUAL(rect.Top(), 2);
  BOOST_CHECK_EQUAL(rect.Bottom(), 8);
}

BOOST_AUTO_TEST_CASE(SgRectTestIncludeRect) {
  GoRect rect(2, 5, 7, 9);
  GoRect rect2(1, 4, 8, 11);
  rect.Include(rect2);
  BOOST_CHECK_EQUAL(rect.Left(), 1);
  BOOST_CHECK_EQUAL(rect.Right(), 5);
  BOOST_CHECK_EQUAL(rect.Top(), 7);
  BOOST_CHECK_EQUAL(rect.Bottom(), 11);
}

BOOST_AUTO_TEST_CASE(SgRectTestIncludeXY) {
  GoRect rect;
  rect.IncludeXY(-1, 8);
  BOOST_CHECK_EQUAL(rect.Left(), -1);
  BOOST_CHECK_EQUAL(rect.Right(), -1);
  BOOST_CHECK_EQUAL(rect.Top(), 8);
  BOOST_CHECK_EQUAL(rect.Bottom(), 8);
  rect.IncludeXY(-5, -6);
  BOOST_CHECK_EQUAL(rect.Left(), -5);
  BOOST_CHECK_EQUAL(rect.Right(), -1);
  BOOST_CHECK_EQUAL(rect.Top(), -6);
  BOOST_CHECK_EQUAL(rect.Bottom(), 8);
}

BOOST_AUTO_TEST_CASE(SgRectTestInRect) {
  GoRect rect(2, 5, 7, 9);
  BOOST_CHECK(!rect.InRect(Pt(1, 8)));
  BOOST_CHECK(!rect.InRect(Pt(2, 6)));
  BOOST_CHECK(rect.InRect(Pt(2, 7)));
  BOOST_CHECK(rect.InRect(Pt(2, 8)));
  BOOST_CHECK(rect.InRect(Pt(2, 9)));
  if (GO_MAX_SIZE >= 10) {
    BOOST_CHECK(!rect.InRect(Pt(2, 10)));
    BOOST_CHECK(!rect.InRect(Pt(5, 10)));
  }
  BOOST_CHECK(!rect.InRect(Pt(3, 5)));
  BOOST_CHECK(rect.InRect(Pt(5, 7)));
  BOOST_CHECK(rect.InRect(Pt(5, 9)));
}

BOOST_AUTO_TEST_CASE(SgRectTestMirrorX) {
  GoRect rect(2, 5, 7, 9);
  rect.MirrorX(19);
  BOOST_CHECK_EQUAL(rect.Left(), 15);
  BOOST_CHECK_EQUAL(rect.Right(), 18);
  BOOST_CHECK_EQUAL(rect.Top(), 7);
  BOOST_CHECK_EQUAL(rect.Bottom(), 9);
}

BOOST_AUTO_TEST_CASE(SgRectTestMirrorY) {
  GoRect rect(2, 5, 7, 9);
  rect.MirrorY(19);
  BOOST_CHECK_EQUAL(rect.Left(), 2);
  BOOST_CHECK_EQUAL(rect.Right(), 5);
  BOOST_CHECK_EQUAL(rect.Top(), 11);
  BOOST_CHECK_EQUAL(rect.Bottom(), 13);
}

BOOST_AUTO_TEST_CASE(SgRectTestOverlaps) {
  GoRect rect(2, 5, 7, 9);
  GoRect rect1(1, 4, 7, 9);
  BOOST_CHECK(rect.Overlaps(rect1));
  GoRect rect2(4, 11, 7, 9);
  BOOST_CHECK(rect.Overlaps(rect2));
  GoRect rect3(2, 5, 3, 9);
  BOOST_CHECK(rect.Overlaps(rect3));
  GoRect rect4(2, 5, 7, 14);
  BOOST_CHECK(rect.Overlaps(rect4));
  GoRect rect5(2, 5, 7, 9);
  BOOST_CHECK(rect.Overlaps(rect5));
  GoRect rect6(3, 4, 8, 9);
  BOOST_CHECK(rect.Overlaps(rect6));
  GoRect rect7(1, 9, 1, 12);
  BOOST_CHECK(rect.Overlaps(rect7));
  GoRect rect8(6, 9, 7, 9);
  BOOST_CHECK(!rect.Overlaps(rect8));
}

BOOST_AUTO_TEST_CASE(SgRectTestSwapXY) {
  GoRect rect(2, 5, 7, 9);
  rect.SwapXY();
  BOOST_CHECK_EQUAL(rect.Left(), 7);
  BOOST_CHECK_EQUAL(rect.Right(), 9);
  BOOST_CHECK_EQUAL(rect.Top(), 2);
  BOOST_CHECK_EQUAL(rect.Bottom(), 5);
}

}

