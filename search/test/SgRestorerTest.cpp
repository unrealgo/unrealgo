//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "platform/SgRestorer.h"

using namespace std;

namespace {

BOOST_AUTO_TEST_CASE(SgRestorerTestRestorer) {
  int x = 5;
  {
    SgRestorer<int> restorer(&x);
    BOOST_CHECK_EQUAL(x, 5);
    x = 10;
  }
  BOOST_CHECK_EQUAL(x, 5);
}

}

