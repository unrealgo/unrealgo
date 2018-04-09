

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "UctValue.h"

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(UctValueTest_IsPrecise) {
  using UctValueUtil::IsPrecise;
  {
    BOOST_CHECK(IsPrecise<double>(1.0));
    BOOST_CHECK(!IsPrecise<double>(std::numeric_limits<double>::max()));
    const int radix = std::numeric_limits<double>::radix;
    const int digits = std::numeric_limits<double>::digits;
    double count = pow(double(radix), digits) - 1;
    BOOST_CHECK(IsPrecise<double>(count));
    ++count;
    BOOST_CHECK(!IsPrecise<double>(count));
  }
  {
    BOOST_CHECK(IsPrecise<float>(1.0));
    BOOST_CHECK(!IsPrecise<float>(std::numeric_limits<float>::max()));
    const int radix = std::numeric_limits<float>::radix;
    const int digits = std::numeric_limits<float>::digits;
    float count = powf(float(radix), digits) - 1;
    BOOST_CHECK(IsPrecise<float>(count));
    ++count;
    BOOST_CHECK(!IsPrecise<float>(count));
  }
}

} // namespace

//----------------------------------------------------------------------------

