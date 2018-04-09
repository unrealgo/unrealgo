

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "GoEvalArray.h"

#include "GoBoard.h"
#include "board/GoPoint.h"

using GoPointUtil::Pt;

namespace {

BOOST_AUTO_TEST_CASE(GoEvalArrayTest_GoEvalArray) {
  GoEvalArray<int> eval(0);
  BOOST_CHECK_EQUAL(eval[0], 0);
  BOOST_CHECK_EQUAL(eval[GO_PASS], 0);
  BOOST_CHECK_EQUAL(eval[Pt(5, 5)], 0);
  eval[GO_PASS] = 9;
  BOOST_CHECK_EQUAL(eval[GO_PASS], 9);
}

BOOST_AUTO_TEST_CASE(GoEvalArrayTest_GoEvalArray_2) {
  GoEvalArray<int> eval(5);
  BOOST_CHECK_EQUAL(eval[0], 5);
  BOOST_CHECK_EQUAL(eval[GO_PASS], 5);
  BOOST_CHECK_EQUAL(eval[Pt(5, 5)], 5);
}

}

