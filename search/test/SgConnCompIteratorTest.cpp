//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <boost/test/auto_unit_test.hpp>
#include "board/GoConnCompIterator.h"

using namespace std;
using GoPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(SgPointSetTestConnCompIterator) {
  GoPointSet a;
  a.Include(Pt(1, 1));
  a.Include(Pt(1, 2));
  a.Include(Pt(2, 1));
  a.Include(Pt(1, GO_MAX_SIZE));
  GoConnCompIterator it(a, GO_MAX_SIZE);
  BOOST_CHECK(it);
  GoPointSet b = *it;
  BOOST_CHECK_EQUAL(b.Size(), 3);
  BOOST_CHECK(a.Contains(Pt(1, 1)));
  BOOST_CHECK(a.Contains(Pt(1, 2)));
  BOOST_CHECK(a.Contains(Pt(2, 1)));
  ++it;
  BOOST_CHECK(it);
  b = *it;
  BOOST_CHECK_EQUAL(b.Size(), 1);
  BOOST_CHECK(a.Contains(Pt(1, GO_MAX_SIZE)));
  ++it;
  BOOST_CHECK(!it);
}

} // namespace

//----------------------------------------------------------------------------

