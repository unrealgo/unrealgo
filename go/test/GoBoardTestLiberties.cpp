

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "GoBoard.h"
#include "GoSetupUtil.h"
#include "board/SgWrite.h"

using GoPointUtil::Pt;
using namespace std;

namespace {

BOOST_AUTO_TEST_CASE(GoBoardTest_Liberties) {

  GoSetup setup;
  setup.m_player = SG_WHITE;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddWhite(Pt(2, 2));
  setup.AddBlack(Pt(5, 5));
  setup.AddBlack(Pt(5, 6));
  setup.AddBlack(Pt(6, 5));
  setup.AddBlack(Pt(6, 6));
  GoBoard bd(9, setup);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 3);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 4);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(1, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(2, 1)), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(2, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(5, 6)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 1)), 5);

  cout << "liberties at point(2,2):" << bd.NumLiberties(Pt(2, 2)) << endl;
  cout << "liberties at point(5,5):" << bd.NumLiberties(Pt(5, 5)) << endl;
  cout << "liberties at point(5,6):" << bd.NumLiberties(Pt(5, 6)) << endl;
  cout << "liberties at point(6,5):" << bd.NumLiberties(Pt(6, 5)) << endl;
  cout << "liberties at point(6,6):" << bd.NumLiberties(Pt(6, 6)) << endl;

  bd.CheckConsistency();
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Liberties2) {

  GoSetup setup;
  setup.m_player = SG_WHITE;
  setup.AddWhite(Pt(3, 4));
  setup.AddWhite(Pt(4, 3));
  setup.AddWhite(Pt(4, 5));
  setup.AddWhite(Pt(5, 2));
  setup.AddWhite(Pt(6, 3));
  setup.AddWhite(Pt(6, 5));
  setup.AddWhite(Pt(7, 4));
  setup.AddBlack(Pt(4, 4));
  setup.AddBlack(Pt(5, 3));
  setup.AddBlack(Pt(5, 5));
  setup.AddBlack(Pt(6, 4));
  GoBoard bd(9, setup);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 7);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 4);

  cout << "liberties at point(3,4):" << bd.NumLiberties(Pt(3, 4)) << endl;
  cout << "liberties at point(4,4):" << bd.NumLiberties(Pt(4, 4)) << endl;
  cout << "liberties at point(5,3):" << bd.NumLiberties(Pt(5, 3)) << endl;
  cout << "liberties at point(5,5):" << bd.NumLiberties(Pt(5, 5)) << endl;
  cout << "liberties at point(6,4):" << bd.NumLiberties(Pt(6, 4)) << endl;

  bd.CheckConsistency();
}

}

