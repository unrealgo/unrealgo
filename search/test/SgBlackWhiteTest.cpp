//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "board/GoBlackWhite.h"

using namespace std;

namespace {

BOOST_AUTO_TEST_CASE(SgBWIteratorTest) {
  SgBWIterator i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, SG_BLACK);
  ++i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, SG_WHITE);
  ++i;
  BOOST_CHECK(!i);
}

}

