//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <boost/test/auto_unit_test.hpp>
#include "board/GoBWSet.h"

using namespace std;
using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(SgBWSetTest_Equals) {
  GoBWSet set1;
  GoBWSet set2;
  GoPoint p = Pt(5, 5);
  BOOST_CHECK_EQUAL(set1, set2);
  set1[SG_BLACK].Include(p);
  BOOST_CHECK(set1 != set2);
  set2[SG_WHITE].Include(p);
  BOOST_CHECK(set1 != set2);
  set1[SG_WHITE].Include(p);
  BOOST_CHECK(set1 != set2);
  set2[SG_BLACK].Include(p);
  BOOST_CHECK_EQUAL(set1, set2);
}

}

