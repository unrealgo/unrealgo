//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "SgMiaiStrategy.h"

#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "board/GoPointSet.h"
#include "board/GoPointSetUtil.h"
#include "SgStrategy.h"

using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgMiaiStrategyTest_SgMiaiStrategy_1) {
  SgMiaiStrategy m(SG_BLACK);
  BOOST_CHECK_EQUAL(m.Status(), SGSTRATEGY_ACHIEVED);
  BOOST_CHECK(m.OpenThreats().IsEmpty());
}

BOOST_AUTO_TEST_CASE(SgMiaiStrategyTest_SgMiaiStrategy_2) {

  GoPoint p1(Pt(3, 3));
  GoPoint p2(Pt(5, 3));
  SgMiaiPair p(p1, p2);
  SgMiaiStrategy s(SG_BLACK);
  s.AddPair(p);

  GoPointSet set1 = s.Dependency();
  GoPointSet set2;
  set2.Include(p1);
  set2.Include(p2);
  BOOST_CHECK_EQUAL(set1, set2);
  BOOST_CHECK_EQUAL(s.Status(), SGSTRATEGY_ACHIEVED);
  BOOST_CHECK(s.OpenThreats().IsEmpty());
  s.ExecuteMove(p1, SG_WHITE);
  BOOST_CHECK_EQUAL(s.Status(), SGSTRATEGY_THREATENED);
  BOOST_CHECK(s.OpenThreats().IsLength(1));
  BOOST_CHECK_EQUAL(s.OpenThreatMove(), p2);
  SgMiaiStrategy s2(s);
  s.ExecuteMove(p2, SG_BLACK);
  BOOST_CHECK_EQUAL(s.Status(), SGSTRATEGY_ACHIEVED);
  set1 = s.Dependency();
  GoPointSet emptySet;
  BOOST_CHECK_EQUAL(set1, emptySet);
  s2.ExecuteMove(p2, SG_WHITE);
  BOOST_CHECK_EQUAL(s2.Status(), SGSTRATEGY_FAILED);
  set1 = s2.Dependency();
  BOOST_CHECK_EQUAL(set1, emptySet);
}

}


