//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "board/GoPointArray.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(GoArrayTestConstructorDefault) {
  // Use placement new to avoid memory accidentally initialized with 0
  const size_t size = sizeof(GoPointArray<int>);
  char buffer[size];
  for (size_t i = 0; i < size; ++i)
    buffer[i] = 1;
  GoPointArray<int> *array = new(buffer) GoPointArray<int>(0);
  BOOST_CHECK_EQUAL((*array)[0], 0);
  BOOST_CHECK_EQUAL((*array)[GO_MAXPOINT - 1], 0);
}

} // namespace

//----------------------------------------------------------------------------

