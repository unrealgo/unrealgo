//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <bitset>
#include <boost/test/auto_unit_test.hpp>

using namespace std;

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(SgSystemTestGccBitSetBug) {
  bitset<1> set1;
  bitset<1> set2;
  set2.set(0);
  set1 |= set2;
  BOOST_CHECK(set1.test(0));
}

} // namespace

//----------------------------------------------------------------------------

