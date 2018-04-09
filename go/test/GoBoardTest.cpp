

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include <platform/SgDebug.h>
#include <GtpEngine.h>
#include "GoBoard.h"
#include "GoSetupUtil.h"
#include "board/SgWrite.h"

using GoPointUtil::Pt;
using namespace std;

void showboard(const GoBoard& bd) {
}

namespace {

BOOST_AUTO_TEST_CASE(GoBoardTest_Constructor_Setup) {

  GoSetup setup;
  setup.m_player = SG_WHITE;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddWhite(Pt(2, 2));
  GoBoard bd(9, setup);
  BOOST_CHECK_EQUAL(bd.ToPlay(), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 3);
  BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(1, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(2, 1)), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(Pt(2, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 1)), 5);

  cout << "liberties at point(2,2):" << bd.NumLiberties(Pt(2, 2)) << endl;

  bd.CheckConsistency();
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Anchor) {

  GoSetup setup;
  setup.AddBlack(Pt(4, 1));
  setup.AddBlack(Pt(4, 2));
  setup.AddBlack(Pt(3, 2));
  setup.AddBlack(Pt(1, 2));
  GoBoard bd(9, setup);
  showboard(bd);
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(4, 1)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(4, 2)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(3, 2)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(1, 2)), Pt(1, 2));
  bd.Play(Pt(2, 2), SG_BLACK);

  BOOST_CHECK_EQUAL(bd.Anchor(Pt(4, 1)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(4, 2)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(3, 2)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(2, 2)), Pt(4, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(1, 2)), Pt(4, 1));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Anchor_2) {

  GoSetup setup;
  setup.AddBlack(Pt(1, 2));
  setup.AddBlack(Pt(2, 1));
  GoBoard bd(9, setup);
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(1, 2)), Pt(1, 2));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(2, 1)), Pt(2, 1));
  bd.Play(Pt(1, 1), SG_BLACK);

  BOOST_CHECK_EQUAL(bd.Anchor(Pt(1, 1)), Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(1, 2)), Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.Anchor(Pt(2, 1)), Pt(1, 1));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_CanCapture) {
  std::string s("XO..O.\n"
                    ".XOO..\n"
                    "......\n"
                    "......\n"
                    "......\n"
                    "......");
  int boardSize;
  GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
  setup.m_player = SG_BLACK;
  GoBoard bd(boardSize, setup);
  BOOST_CHECK(bd.IsColor(Pt(1, 6), SG_BLACK));
  BOOST_CHECK(bd.IsColor(Pt(2, 6), SG_WHITE));
  BOOST_CHECK(bd.IsColor(Pt(2, 5), SG_BLACK));
  BOOST_CHECK(bd.CanCapture(Pt(3, 6), SG_BLACK));
  BOOST_CHECK(!bd.CanCapture(Pt(3, 6), SG_WHITE));
  BOOST_CHECK(!bd.CanCapture(Pt(4, 6), SG_BLACK));
  BOOST_CHECK(!bd.CanCapture(Pt(1, 5), SG_BLACK));
  BOOST_CHECK(bd.CanCapture(Pt(1, 5), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_CanUndo) {
  GoBoard bd;
  BOOST_CHECK(!bd.CanUndo());
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(bd.CanUndo());
  bd.Undo();
  BOOST_CHECK(!bd.CanUndo());
}

BOOST_AUTO_TEST_CASE(GoBoardTest_CapturedStones) {
  GoBoard bd;
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.CapturingMove());
  BOOST_CHECK_EQUAL(bd.NuCapturedStones(), 0);
  BOOST_CHECK_EQUAL(bd.CapturedStones().Length(), 0);
  bd.Play(Pt(1, 2), SG_WHITE);
  BOOST_CHECK(!bd.CapturingMove());
  BOOST_CHECK_EQUAL(bd.NuCapturedStones(), 0);
  BOOST_CHECK_EQUAL(bd.CapturedStones().Length(), 0);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.CapturingMove());
  BOOST_CHECK_EQUAL(bd.NuCapturedStones(), 1);
  BOOST_CHECK(bd.CapturedStones().Contains(Pt(1, 1)));
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(bd.CapturingMove());
  BOOST_CHECK_EQUAL(bd.NuCapturedStones(), 1);
  BOOST_CHECK(bd.CapturedStones().Contains(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Defaults) {
  GoBoard bd;
  BOOST_CHECK(bd.KoModifiesHash());
  BOOST_CHECK(!bd.KoRepetitionAllowed());
  BOOST_CHECK(!bd.AnyRepetitionAllowed());
}

BOOST_AUTO_TEST_CASE(GoBoardTest_GetHashCode) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_BLACK);
  SgHashCode h1 = bd.GetHashCode();
  bd.Play(Pt(1, 2), SG_WHITE);
  SgHashCode h2 = bd.GetHashCode();
  BOOST_CHECK(h2 != h1);
  bd.Play(Pt(2, 1), SG_WHITE);
  SgHashCode h3 = bd.GetHashCode();
  BOOST_CHECK(h3 != h1);
  BOOST_CHECK(h3 != h2);
  bd.Play(GO_PASS, SG_BLACK);
  SgHashCode h4 = bd.GetHashCode();
  BOOST_CHECK(h4 == h3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCode(), h3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCode(), h2);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCode(), h1);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_GetHashCode_EmptyPosition) {
  {
    GoBoard bd(2);
    BOOST_CHECK(bd.GetHashCode().IsZero());
  }
  {
    GoBoard bd(8);
    BOOST_CHECK(bd.GetHashCode().IsZero());
  }
  {
    GoBoard bd(9);
    BOOST_CHECK(bd.GetHashCode().IsZero());
  }
  if (GO_MAX_SIZE >= 19) {
    GoBoard bd(19);
    BOOST_CHECK(bd.GetHashCode().IsZero());
  }
}

BOOST_AUTO_TEST_CASE(GoBoardTest_GetHashCodeInclToPlay) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_BLACK);
  SgHashCode h1 = bd.GetHashCodeInclToPlay();
  bd.Play(Pt(1, 2), SG_WHITE);
  SgHashCode h2 = bd.GetHashCodeInclToPlay();
  BOOST_CHECK(h2 != h1);
  bd.Play(Pt(2, 1), SG_WHITE);
  SgHashCode h3 = bd.GetHashCodeInclToPlay();
  BOOST_CHECK(h3 != h1);
  BOOST_CHECK(h3 != h2);
  bd.Play(GO_PASS, SG_BLACK);
  SgHashCode h4 = bd.GetHashCodeInclToPlay();
  BOOST_CHECK(h4 != h1);
  BOOST_CHECK(h4 != h2);
  BOOST_CHECK(h4 != h3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCodeInclToPlay(), h3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCodeInclToPlay(), h2);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCodeInclToPlay(), h1);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_GetLastMove) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Play(Pt(2, 2), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(2, 2));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), Pt(1, 1));
  bd.Play(GO_PASS, SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_PASS);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), Pt(2, 2));
  bd.SetToPlay(SG_BLACK);
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(2, 2));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), Pt(1, 1));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetLastMove(), Pt(1, 1));
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetLastMove(), GO_NULLMOVE);
  BOOST_CHECK_EQUAL(bd.Get2ndLastMove(), GO_NULLMOVE);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_InAtari) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 1));
  GoBoard bd(9, setup);
  BOOST_CHECK(!bd.InAtari(Pt(1, 1)));
  setup.AddWhite(Pt(1, 2));
  bd.Init(9, setup);
  bd.SetToPlay(SG_BLACK);
  BOOST_CHECK(bd.InAtari(Pt(1, 1)));
  bd.SetToPlay(SG_WHITE);
  BOOST_CHECK(bd.InAtari(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Init) {
  GoBoard bd(9);
  bd.Play(Pt(3, 3), SG_BLACK);
  bd.Init(5);
  BOOST_CHECK_EQUAL(bd.Size(), 5);
  BOOST_CHECK(bd.IsBorder(Pt(6, 6)));
  for (GoBoard::Iterator it(bd); it; ++it)
    BOOST_CHECK_MESSAGE(bd.GetColor(*it) == SG_EMPTY,
                        "Point " << GoWritePoint(*it) << " not empty");
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsFirst) {
  GoBoard bd;
  BOOST_CHECK(bd.IsFirst(Pt(1, 1)));
  bd.Play(Pt(2, 2), SG_BLACK);
  BOOST_CHECK(bd.IsFirst(Pt(1, 1)));
  bd.Play(Pt(2, 1), SG_WHITE);
  bd.Play(Pt(3, 1), SG_BLACK);
  bd.Play(Pt(1, 2), SG_WHITE);
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(GO_PASS, SG_WHITE);
  bd.Play(GO_PASS, SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(!bd.IsFirst(Pt(1, 1)));
  bd.Play(GO_PASS, SG_BLACK);
  bd.Play(GO_PASS, SG_WHITE);
  BOOST_CHECK(!bd.IsFirst(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal) {
  GoBoard bd;
  BOOST_CHECK(bd.IsLegal(Pt(1, 1), SG_BLACK));
  BOOST_CHECK(bd.IsLegal(Pt(1, 1), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_Pass) {
  GoBoard bd;
  BOOST_CHECK(bd.IsLegal(GO_PASS, SG_BLACK));
  BOOST_CHECK(bd.IsLegal(GO_PASS, SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_Ko) {

  GoSetup setup;
  setup.AddWhite(Pt(1, 1));
  setup.AddWhite(Pt(2, 2));
  setup.AddWhite(Pt(3, 1));
  setup.AddBlack(Pt(1, 2));
  GoBoard bd(9, setup);
  bd.Play(Pt(2, 1), SG_BLACK);

  BOOST_CHECK(!bd.IsLegal(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_KoRepetition) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  setup.AddBlack(Pt(3, 1));
  GoBoard bd(9, setup);
  BOOST_REQUIRE(!bd.KoRepetitionAllowed());
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.IsLegal(Pt(2, 1), SG_WHITE));
  bd.AllowKoRepetition(true);
  BOOST_CHECK(bd.IsLegal(Pt(2, 1), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_KoRepetition_2) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  setup.AddBlack(Pt(3, 1));
  GoBoard bd(9, setup);
  BOOST_REQUIRE(!bd.KoRepetitionAllowed());
  bd.Rules().SetKoRule(GoRules::SIMPLEKO);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.IsLegal(Pt(2, 1), SG_WHITE));
  bd.Play(Pt(5, 5), SG_WHITE);
  bd.Undo();
  BOOST_CHECK(!bd.IsLegal(Pt(2, 1), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_KoRepetition_3) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 2));
  setup.AddWhite(Pt(2, 3));
  setup.AddWhite(Pt(3, 1));
  setup.AddWhite(Pt(3, 3));
  setup.AddWhite(Pt(4, 3));
  setup.AddWhite(Pt(5, 1));
  setup.AddWhite(Pt(5, 2));
  setup.AddWhite(Pt(5, 3));
  setup.AddBlack(Pt(2, 1));
  setup.AddBlack(Pt(4, 1));
  setup.AddBlack(Pt(4, 2));
  GoBoard bd(9, setup);
  BOOST_REQUIRE(bd.IsLegal(Pt(3, 2), SG_BLACK));
  bd.Play(Pt(3, 2), SG_BLACK);
  BOOST_CHECK(bd.IsLegal(Pt(3, 1), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_PositionalSuperko) {
  GoBoard bd(9);
  bd.Play(Pt(2, 8), SG_BLACK);
  bd.Play(Pt(1, 8), SG_WHITE);
  bd.Play(Pt(3, 8), SG_BLACK);
  bd.Play(Pt(2, 9), SG_WHITE);
  bd.Play(Pt(4, 9), SG_BLACK);
  bd.Play(Pt(3, 9), SG_WHITE);
  bd.Play(Pt(1, 9), SG_BLACK);
  bd.Rules().SetKoRule(GoRules::POS_SUPERKO);
  BOOST_CHECK(!bd.IsLegal(Pt(2, 9), SG_WHITE));
  bd.Rules().SetKoRule(GoRules::SUPERKO);
  BOOST_CHECK(bd.IsLegal(Pt(2, 9), SG_WHITE));
  bd.Rules().SetKoRule(GoRules::SIMPLEKO);
  BOOST_CHECK(bd.IsLegal(Pt(2, 9), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_Occupied) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  GoBoard bd(9, setup);
  BOOST_CHECK(!bd.IsLegal(Pt(1, 2), SG_BLACK));
  BOOST_CHECK(!bd.IsLegal(Pt(1, 2), SG_WHITE));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_Suicide) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  GoBoard bd(9, setup);
  BOOST_REQUIRE(!bd.Rules().AllowSuicide());
  BOOST_CHECK(!bd.IsLegal(Pt(1, 1), SG_BLACK));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLegal_Suicide_2) {
  GoBoard bd;
  BOOST_REQUIRE(!bd.Rules().AllowSuicide());
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(1, 2), SG_WHITE);
  bd.Play(Pt(1, 3), SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(!bd.IsLegal(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsLibertyOfBlock) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  GoBoard bd(9, setup);
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(1, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(2, 1))));
  BOOST_CHECK(!bd.IsLibertyOfBlock(Pt(1, 1), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(3, 2), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(bd.IsLibertyOfBlock(Pt(2, 3), bd.Anchor(Pt(2, 2))));
  BOOST_CHECK(!bd.IsLibertyOfBlock(Pt(2, 3), bd.Anchor(Pt(1, 2))));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsSuicide_1) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  GoBoard bd(9, setup);
  bd.SetToPlay(SG_BLACK);
  BOOST_CHECK(bd.IsSuicide(Pt(1, 1)));
  BOOST_CHECK(!bd.IsSuicide(Pt(3, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsSuicide_2) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  bd.Play(Pt(1, 2), SG_WHITE);
  BOOST_CHECK(bd.IsSuicide(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsSuicide_3) {
  GoBoard bd(9);
  bd.Play(Pt(1, 2), SG_BLACK);
  bd.Play(Pt(1, 1), SG_WHITE);
  bd.Play(Pt(2, 2), SG_BLACK);
  bd.Play(Pt(3, 1), SG_BLACK);
  BOOST_CHECK(bd.IsSuicide(Pt(2, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_IsValidPoint) {
  GoBoard bd(GO_MAX_SIZE);
  BOOST_CHECK(bd.IsValidPoint(Pt(1, 1)));
  BOOST_CHECK(bd.IsValidPoint(Pt(1, GO_MAX_SIZE)));
  BOOST_CHECK(bd.IsValidPoint(Pt(GO_MAX_SIZE, 1)));
  BOOST_CHECK(bd.IsValidPoint(Pt(GO_MAX_SIZE, GO_MAX_SIZE)));
  BOOST_CHECK(!bd.IsValidPoint(Pt(1, 1) - GO_NORTH_SOUTH));
  BOOST_CHECK(!bd.IsValidPoint(Pt(1, 1) - GO_WEST_EAST));
  BOOST_CHECK(!bd.IsValidPoint(Pt(1, GO_MAX_SIZE) + GO_NORTH_SOUTH));
  BOOST_CHECK(!bd.IsValidPoint(Pt(1, GO_MAX_SIZE) - GO_WEST_EAST));
  BOOST_CHECK(!bd.IsValidPoint(Pt(GO_MAX_SIZE, 1) - GO_NORTH_SOUTH));
  BOOST_CHECK(!bd.IsValidPoint(Pt(GO_MAX_SIZE, 1) + GO_WEST_EAST));
  BOOST_CHECK(!bd.IsValidPoint(Pt(GO_MAX_SIZE, GO_MAX_SIZE) + GO_NORTH_SOUTH));
  BOOST_CHECK(!bd.IsValidPoint(Pt(GO_MAX_SIZE, GO_MAX_SIZE) + GO_WEST_EAST));
  BOOST_CHECK(!bd.IsValidPoint(0));
  BOOST_CHECK(!bd.IsValidPoint(GO_PASS));
  BOOST_CHECK(!bd.IsValidPoint(GO_ENDPOINT));
  BOOST_CHECK(!bd.IsValidPoint(GO_NULLMOVE));
  BOOST_CHECK(!bd.IsValidPoint(GO_NULLPOINT));
  BOOST_CHECK(!bd.IsValidPoint(2 * GO_MAXPOINT));
}

void GoBoardTest_Ko(bool allowKoRepetition, bool koModifiesHash) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 2));
  setup.AddWhite(Pt(1, 1));
  setup.AddWhite(Pt(2, 2));
  setup.AddWhite(Pt(3, 1));
  GoBoard bd(9, setup);
  bd.AllowKoRepetition(allowKoRepetition);
  bd.SetKoModifiesHash(koModifiesHash);
  bd.SetKoLoser(SG_BLACK);
  BOOST_CHECK_EQUAL(bd.KoLevel(), 0);
  const int maxKoLevel = GoBoard::MAX_KOLEVEL;
  for (int i = 1; i < maxKoLevel + 5; ++i) {
    bd.Play(Pt(2, 1), SG_BLACK);
    if (i > 1) {
      BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
      BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
    }
    bd.Play(Pt(1, 1), SG_WHITE);
    BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
    if (!allowKoRepetition
        || (koModifiesHash && i > maxKoLevel))
      BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
    else
      BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));

    if (!allowKoRepetition)
      BOOST_CHECK_EQUAL(bd.KoLevel(), 0);
    else if (koModifiesHash && i > maxKoLevel)
      BOOST_CHECK_EQUAL(bd.KoLevel(), maxKoLevel);
    else
      BOOST_CHECK_EQUAL(bd.KoLevel(), i);
  }
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Ko_RepetitionModifyHash) {
  GoBoardTest_Ko(true, true);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Ko_RepetitionNoModifyHash) {
  GoBoardTest_Ko(true, false);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Ko_NoRepetitionModifyHash) {
  GoBoardTest_Ko(false, true);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Ko_NoRepetitionNoModifyHash) {
  GoBoardTest_Ko(false, false);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_KoModifiesHash) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  setup.AddBlack(Pt(3, 1));
  GoBoard bd(9, setup);
  bd.AllowKoRepetition(true);
  BOOST_CHECK(bd.KoModifiesHash());
  SgHashCode code = bd.GetHashCode();
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.GetHashCode() != code);
  bd.Undo();
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.GetHashCode(), code);
  bd.SetKoModifiesHash(false);
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.GetHashCode(), code);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_KoPoint) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 2));
  setup.AddWhite(Pt(1, 1));
  setup.AddWhite(Pt(2, 2));
  setup.AddWhite(Pt(3, 1));
  GoBoard bd(9, setup);
  BOOST_CHECK_EQUAL(bd.KoPoint(), GO_NULLPOINT);
  bd.Play(Pt(2, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.KoPoint(), Pt(1, 1));
  bd.Play(Pt(1, 3), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.KoPoint(), GO_NULLPOINT);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.KoPoint(), Pt(1, 1));
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.KoPoint(), GO_NULLPOINT);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsCapturing_1) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 1));
  setup.AddWhite(Pt(1, 2));
  GoBoard bd(9, setup);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_CAPTURING));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsCapturing_2) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 2));
  setup.AddBlack(Pt(2, 1));
  GoBoard bd(9, setup);
  bd.Play(Pt(1, 1), SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_CAPTURING));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsIllegal) {
  GoBoard bd;
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
  bd.Play(Pt(2, 2), SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsIllegal_KoRepetition) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  setup.AddBlack(Pt(2, 2));
  setup.AddBlack(Pt(3, 1));
  GoBoard bd(9, setup);
  BOOST_REQUIRE(!bd.KoRepetitionAllowed());
  bd.Play(Pt(1, 1), SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
  bd.Undo();
  bd.Play(Pt(5, 5), SG_WHITE);
  bd.Undo();
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
  bd.Undo();
  bd.AllowKoRepetition(true);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsIllegal_Suicide) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  GoBoard bd(9, setup);
  bd.Rules().SetAllowSuicide(false);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_LastMoveInfo_IsRepetition) {
  GoBoard bd;
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(1, 2), SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(2, 2), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(3, 1), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(GO_PASS, SG_WHITE);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(GO_PASS, SG_BLACK);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK(bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
  bd.Play(Pt(3, 2), SG_BLACK);
  BOOST_CHECK(!bd.LastMoveInfo(GO_MOVEFLAG_REPETITION));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Line_9) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.Line(Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(bd.Line(Pt(3, 4)), 3);
  BOOST_CHECK_EQUAL(bd.Line(Pt(8, 5)), 2);
  BOOST_CHECK_EQUAL(bd.Line(Pt(5, 8)), 2);
  BOOST_CHECK_EQUAL(bd.Line(Pt(9, 9)), 1);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Line_10) {
  if (GO_MAX_SIZE >= 10) {
    GoBoard bd(10);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 4)), 4);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 5)), 5);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 6)), 5);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 7)), 4);
    BOOST_CHECK_EQUAL(bd.Line(Pt(4, 5)), 4);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 5)), 5);
    BOOST_CHECK_EQUAL(bd.Line(Pt(6, 5)), 5);
    BOOST_CHECK_EQUAL(bd.Line(Pt(7, 5)), 4);
  }
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Line_19) {
  if (GO_MAX_SIZE >= 19) {
    GoBoard bd(19);
    BOOST_CHECK_EQUAL(bd.Line(Pt(1, 1)), 1);
    BOOST_CHECK_EQUAL(bd.Line(Pt(3, 4)), 3);
    BOOST_CHECK_EQUAL(bd.Line(Pt(18, 5)), 2);
    BOOST_CHECK_EQUAL(bd.Line(Pt(5, 18)), 2);
    BOOST_CHECK_EQUAL(bd.Line(Pt(19, 19)), 1);
  }
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_1) {
  GoSetup setup;
  setup.AddBlack(Pt(1, 3));
  GoBoard bd(9, setup);
  bd.Play(Pt(2, 2), SG_WHITE);
  bd.Play(Pt(1, 2), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 2)), 3);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_2) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 2);
  bd.Undo();
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 2);
  bd.Play(Pt(1, 2), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 3);
  bd.Undo();
  bd.Play(Pt(1, 2), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 3);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_3) {
  GoSetup setup;
  setup.AddBlack(Pt(2, 2));
  setup.AddBlack(Pt(3, 1));
  setup.AddWhite(Pt(1, 2));
  setup.AddWhite(Pt(2, 1));
  GoBoard bd(9, setup);
  bd.Play(Pt(1, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 1);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 2)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 1)), 3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 1)), 1);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 2)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 1)), 2);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_4) {
  GoBoard bd(9);
  bd.Play(Pt(5, 5), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 4);
  bd.Play(Pt(4, 4), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(4, 4)), 4);
  bd.Play(Pt(5, 4), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 4)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(4, 4)), 3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(4, 4)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 4);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_5) {
  GoBoard bd(9);
  bd.Play(Pt(5, 5), SG_BLACK);
  bd.Play(Pt(6, 5), SG_BLACK);
  bd.Play(Pt(8, 5), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 6);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 6);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(8, 5)), 4);
  bd.Play(Pt(7, 5), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 10);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 10);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(7, 5)), 10);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(8, 5)), 10);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 6);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 6);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(8, 5)), 4);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_6) {
  GoBoard bd(9);
  bd.Play(Pt(5, 5), SG_BLACK);
  bd.Play(Pt(6, 5), SG_BLACK);
  bd.Play(Pt(7, 5), SG_BLACK);
  bd.Play(Pt(6, 6), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 7);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 7);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(7, 5)), 7);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 6)), 3);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 8);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 8);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(7, 5)), 8);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 6);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(6, 5)), 6);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(5, 5)), 4);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumLiberties_7) {
  GoBoard bd(9);
  bd.Play(Pt(1, 1), SG_WHITE);
  bd.Play(Pt(3, 1), SG_WHITE);
  bd.Play(Pt(1, 2), SG_WHITE);
  bd.Play(Pt(3, 2), SG_WHITE);
  bd.Play(Pt(2, 3), SG_WHITE);
  bd.Play(Pt(2, 2), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 1)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 2)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 3)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 2)), 1);
  bd.Play(Pt(2, 1), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 1)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 1)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 2)), 5);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 3)), 4);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 1)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 1)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(1, 2)), 2);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(3, 2)), 4);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 3)), 3);
  BOOST_CHECK_EQUAL(bd.NumLiberties(Pt(2, 2)), 1);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_NumPrisoners) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 0);
  bd.Play(Pt(1, 2), SG_BLACK);
  bd.Play(Pt(1, 1), SG_WHITE);
  bd.Play(Pt(2, 1), SG_BLACK);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 1);
  bd.Play(Pt(9, 8), SG_WHITE);
  bd.Play(Pt(9, 9), SG_BLACK);
  bd.Play(Pt(8, 9), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 1);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 1);
  bd.Undo();
  bd.Undo();
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 1);
  bd.Undo();
  bd.Undo();
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 0);
  bd.Play(Pt(1, 2), SG_BLACK);
  bd.Play(Pt(9, 9), SG_WHITE);
  bd.Play(Pt(2, 1), SG_BLACK);
  bd.Play(Pt(1, 1), SG_WHITE);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 1);
  bd.Undo();
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_BLACK), 0);
  BOOST_CHECK_EQUAL(bd.NumPrisoners(SG_WHITE), 0);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Occupied) {
  GoSetup setup;
  setup.AddWhite(Pt(1, 2));
  setup.AddBlack(Pt(2, 1));
  GoBoard bd(9, setup);
  BOOST_CHECK(bd.Occupied(Pt(1, 2)));
  BOOST_CHECK(bd.Occupied(Pt(2, 1)));
  GoPoint borderPoint = Pt(1, 1) - GO_WEST_EAST;
  BOOST_REQUIRE(bd.IsBorder(borderPoint));
  BOOST_CHECK(!bd.Occupied(borderPoint));
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Play_SingleStone) {
  GoBoard bd(9);
  GoPoint p = Pt(1, 2);
  bd.Play(p, SG_BLACK);
  BOOST_CHECK_EQUAL(bd.Anchor(p), p);
  BOOST_CHECK_EQUAL(bd.NumLiberties(p), 3);
  BOOST_CHECK_EQUAL(bd.NumStones(p), 1);
}

BOOST_AUTO_TEST_CASE(GoBoardTest_Pos_9) {
  GoBoard bd(9);
  BOOST_CHECK_EQUAL(bd.Pos(Pt(1, 1)), 1);
  BOOST_CHECK_EQUAL(bd.Pos(Pt(3, 4)), 4);
  BOOST_CHECK_EQUAL(bd.Pos(Pt(8, 5)), 5);
  BOOST_CHECK_EQUAL(bd.Pos(Pt(5, 8)), 5);
  BOOST_CHECK_EQUAL(bd.Pos(Pt(9, 9)), 1);
}

}

