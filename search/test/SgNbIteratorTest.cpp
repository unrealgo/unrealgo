//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "board/GoNbIterator.h"

using namespace std;
using GoPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(SgNb4IteratorTest) {
  GoPoint prev = 0;
  for (SgNb4Iterator it(Pt(5, 5)); it; ++it) {
    BOOST_CHECK(*it > prev);
    prev = *it;
  }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(SgNb4DiagIteratorTest) {
  GoPoint prev = 0;
  for (SgNb4DiagIterator it(Pt(5, 5)); it; ++it) {
    BOOST_CHECK(*it > prev);
    prev = *it;
  }
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(SgBoardNb8IteratorTest) {
  GoPoint prev = 0;
  for (SgNb8Iterator it(Pt(5, 5)); it; ++it) {
    BOOST_CHECK(*it > prev);
    prev = *it;
  }
}

//----------------------------------------------------------------------------

} // namespace

