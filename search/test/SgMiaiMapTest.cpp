//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "SgMiaiMap.h"
#include "SgMiaiStrategy.h"
#include "SgStrategy.h"

#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "board/GoPointSet.h"
#include "board/GoPointSetUtil.h"

using GoPointUtil::Pt;
//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SgStrategyTest_SgMiaiMap) {

  GoPoint p1(Pt(3, 3));
  GoPoint p2(Pt(5, 3));
  SgMiaiPair p(p1, p2);
  SgMiaiStrategy s(SG_BLACK);
  s.AddPair(p);

  SgMiaiMap m;
  m.ConvertFromSgMiaiStrategy(s);
  BOOST_CHECK_EQUAL(m.Status(), SGSTRATEGY_ACHIEVED);
  m.ExecuteMove(p1, SG_BLACK);
  BOOST_CHECK_EQUAL(m.Status(), SGSTRATEGY_THREATENED);
  BOOST_CHECK_EQUAL(m.ForcedMove(), p2);

  // todo: convert strategy with open threat.
}

} // namespace
//----------------------------------------------------------------------------

