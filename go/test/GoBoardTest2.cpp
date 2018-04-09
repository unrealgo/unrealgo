

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "GoBoard.h"
#include "GoSetupUtil.h"
#include "board/SgWrite.h"

using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(GoBoardTest_Pos_19) {
  if (GO_MAX_SIZE >= 19) {
    GoBoard bd(19);
    BOOST_CHECK_EQUAL(bd.Pos(Pt(1, 1)), 1);
    BOOST_CHECK_EQUAL(bd.Pos(Pt(3, 4)), 4);
    BOOST_CHECK_EQUAL(bd.Pos(Pt(18, 5)), 5);
    BOOST_CHECK_EQUAL(bd.Pos(Pt(5, 18)), 5);
    BOOST_CHECK_EQUAL(bd.Pos(Pt(19, 19)), 1);
  }
}

BOOST_AUTO_TEST_CASE(GoBoardTest_SetToPlay) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.SetToPlay(SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Play(Pt(2, 1), SG_BLACK);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_ToPlay) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Play(Pt(3, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  bd.Play(GO_PASS, SG_WHITE);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMove) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(1, 1));
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(2, 1));
  bd.Play(Pt(3, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(3, 1));
  bd.Play(Pt(4, 4), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(4, 4));
  bd.Play(Pt(8, 4), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(8, 4));
  bd.Play(GO_PASS, SG_WHITE);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_PASS);
  bd.Play(GO_PASS, SG_BLACK);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_PASS);

  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_PASS);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(8, 4));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(4, 4));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(3, 1));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(2, 1));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(1, 1));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NullMove) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Undo) {
  GoBoard bd(9);
  BOOST_REQUIRE(bd.ToPlay() == SG_BLACK);
  bd.Play(Pt(1, 1), SG_WHITE);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_BLACK);
}

void GoBoardIteratorTest_AtSize(int size) {
  GoBoard bd(size);
  GoArrayList<GoPoint, GO_MAXPOINT> points;
  for (GoBoard::Iterator it(bd); it; ++it)
    points.PushBack(*it);
  BOOST_CHECK_EQUAL(points.Length(), size * size);
  for (int x = 1; x <= size; ++x)
    for (int y = 1; y <= size; ++y) {
      GoPoint p = Pt(x, y);
      BOOST_CHECK_MESSAGE(points.Contains(p),
                          "GoBoard::Iterator size " << size
                                                    << " missing " << GoWritePoint(p));
    }
}

BOOST_AUTO_TEST_CASE(GoBoardIteratorTest_All) {
  if (GO_MAX_SIZE >= 9)
    GoBoardIteratorTest_AtSize(9);
  if (GO_MAX_SIZE >= 10)
    GoBoardIteratorTest_AtSize(10);
  if (GO_MAX_SIZE >= 13)
    GoBoardIteratorTest_AtSize(13);
  if (GO_MAX_SIZE >= 19)
    GoBoardIteratorTest_AtSize(19);
}

BOOST_AUTO_TEST_CASE(GoBoardLibertyIteratorTest) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 3));
  GoBoard bd(9, setup);
  bd.Play(Pt(2, 2), SG_WHITE);
  bd.Play(Pt(1, 2), SG_BLACK);
  GoArrayList<GoPoint, GO_MAXPOINT> libs;
  for (GoBoard::LibertyIterator it(bd, Pt(1, 2)); it; ++it)
    libs.PushBack(*it);
  BOOST_CHECK_EQUAL(libs.Length(), 3);
  BOOST_CHECK(libs.Contains(Pt(1, 1)));
  BOOST_CHECK(libs.Contains(Pt(1, 4)));
  BOOST_CHECK(libs.Contains(Pt(2, 3)));
  libs.Clear();
  for (GoBoard::LibertyIterator it(bd, Pt(2, 2)); it; ++it)
    libs.PushBack(*it);
  BOOST_CHECK_EQUAL(libs.Length(), 3);
  BOOST_CHECK(libs.Contains(Pt(2, 1)));
  BOOST_CHECK(libs.Contains(Pt(2, 3)));
  BOOST_CHECK(libs.Contains(Pt(3, 2)));
}

BOOST_AUTO_TEST_CASE(GoBoardStoneIteratorTest) {
  GoSetup setup;
  setup.AddBlack(Pt(3, 2));
  setup.AddBlack(Pt(4, 3));
  GoBoard bd(9, setup);
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(1, 3), SG_WHITE);
  bd.Play(Pt(1, 2), SG_BLACK);
  bd.Play(Pt(2, 3), SG_WHITE);
  bd.Play(Pt(3, 3), SG_WHITE);
  bd.Play(Pt(2, 2), SG_BLACK);
  GoArrayList<GoPoint, GO_MAXPOINT> stones;
  for (GoBoard::StoneIterator it(bd, Pt(2, 2)); it; ++it)
    stones.PushBack(*it);
  BOOST_CHECK_EQUAL(stones.Length(), 4);
  BOOST_CHECK(stones.Contains(Pt(1, 1)));
  BOOST_CHECK(stones.Contains(Pt(1, 2)));
  BOOST_CHECK(stones.Contains(Pt(2, 2)));
  BOOST_CHECK(stones.Contains(Pt(3, 2)));
  stones.Clear();
  for (GoBoard::StoneIterator it(bd, Pt(4, 3)); it; ++it)
    stones.PushBack(*it);
  BOOST_CHECK_EQUAL(stones.Length(), 1);
  BOOST_CHECK(stones.Contains(Pt(4, 3)));
  stones.Clear();
  for (GoBoard::StoneIterator it(bd, Pt(1, 3)); it; ++it)
    stones.PushBack(*it);
  BOOST_CHECK_EQUAL(stones.Length(), 3);
  BOOST_CHECK(stones.Contains(Pt(1, 3)));
  BOOST_CHECK(stones.Contains(Pt(2, 3)));
  BOOST_CHECK(stones.Contains(Pt(3, 3)));
}
}

