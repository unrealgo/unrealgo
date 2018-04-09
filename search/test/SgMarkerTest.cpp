//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "SgMarker.h"

using namespace std;
using GoPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

#ifndef NDEBUG

BOOST_AUTO_TEST_CASE(SgMarker_Reserve) {
  SgMarker marker;
  SgReserveMarker reserveMarker(marker);
  BOOST_CHECK(marker.markerInUse());
}
#endif

BOOST_AUTO_TEST_CASE(SgMarker_Clear) {
  SgMarker marker;
  marker.Include(Pt(1, 1));
  BOOST_CHECK(marker.Contains(Pt(1, 1)));
  marker.Clear();
  BOOST_CHECK(!marker.Contains(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(SgMarker_Contains) {
  SgMarker marker;
  BOOST_CHECK(!marker.Contains(Pt(1, 1)));
  marker.Include(Pt(1, 1));
  BOOST_CHECK(marker.Contains(Pt(1, 1)));
}

BOOST_AUTO_TEST_CASE(SgMarker_NewMark) {
  SgMarker marker;
  BOOST_CHECK(marker.NewMark(Pt(1, 1)));
  BOOST_CHECK(!marker.NewMark(Pt(1, 1)));
  BOOST_CHECK(!marker.NewMark(Pt(1, 1)));
}

} // namespace

//----------------------------------------------------------------------------

