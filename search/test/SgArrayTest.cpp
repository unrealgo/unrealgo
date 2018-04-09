//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "lib/Array.h"

using namespace std;

namespace {

BOOST_AUTO_TEST_CASE(GoArrayTest_Assign) {
  GoArray<int, 3> a(5);
  GoArray<int, 3> b;
  b = a;
  BOOST_CHECK_EQUAL(b[0], 5);
  BOOST_CHECK_EQUAL(b[1], 5);
  BOOST_CHECK_EQUAL(b[2], 5);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_Bytes) {
  GoArray<int, 10> a;
  GoArray<char, 500> b;
  GoArray<double, 200> c;
  BOOST_CHECK_EQUAL(a.Bytes(), 40);
  BOOST_CHECK_EQUAL(b.Bytes(), 500);
  BOOST_CHECK_EQUAL(c.Bytes(), 1600);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_CopyConstructor) {
  GoArray<int, 3> a(5);
  GoArray<int, 3> b(a);
  BOOST_CHECK_EQUAL(b[0], 5);
  BOOST_CHECK_EQUAL(b[1], 5);
  BOOST_CHECK_EQUAL(b[2], 5);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_Elements) {
  GoArray<int, 3> a;
  a[0] = 5;
  a[1] = 4;
  a[2] = 3;
  BOOST_CHECK_EQUAL(a[0], 5);
  BOOST_CHECK_EQUAL(a[1], 4);
  BOOST_CHECK_EQUAL(a[2], 3);
  const GoArray<int, 3> &b = a;
  BOOST_CHECK_EQUAL(b[0], 5);
  BOOST_CHECK_EQUAL(b[1], 4);
  BOOST_CHECK_EQUAL(b[2], 3);
}

BOOST_AUTO_TEST_CASE(SgSArrayTest_Iterator) {
  GoArray<int, 3> a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  GoArray<int, 3>::Iterator i(a);
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 1);
  ++i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 2);
  ++i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 3);
  ++i;
  BOOST_CHECK(!i);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_MaxValue) {
  GoArray<int, 3> a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  BOOST_CHECK_EQUAL(a.MaxValue(), 3);
  a[1] = 4;
  BOOST_CHECK_EQUAL(a.MaxValue(), 4);
  a[1] = -4;
  BOOST_CHECK_EQUAL(a.MaxValue(), 3);
  a[2] = 0;
  BOOST_CHECK_EQUAL(a.MaxValue(), 1);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_MinValue) {
  GoArray<int, 3> a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  BOOST_CHECK_EQUAL(a.MinValue(), 1);
  a[0] = 4;
  BOOST_CHECK_EQUAL(a.MinValue(), 2);
  a[2] = -4;
  BOOST_CHECK_EQUAL(a.MinValue(), -4);
  a[1] = 6;
  a[2] = 6;
  BOOST_CHECK_EQUAL(a.MinValue(), 4);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_MultiplyAssign) {
  GoArray<int, 3> a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  a *= 10;
  BOOST_CHECK_EQUAL(a[0], 10);
  BOOST_CHECK_EQUAL(a[1], 20);
  BOOST_CHECK_EQUAL(a[2], 30);
}

BOOST_AUTO_TEST_CASE(SgSArrayTestNonConstIterator) {
  GoArray<int, 3> a;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  GoArray<int, 3>::NonConstIterator i(a);
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 1);
  *i = 0;
  ++i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 2);
  *i = 0;
  ++i;
  BOOST_CHECK(i);
  BOOST_CHECK_EQUAL(*i, 3);
  *i = 0;
  ++i;
  BOOST_CHECK(!i);
  BOOST_CHECK_EQUAL(a[0], 0);
  BOOST_CHECK_EQUAL(a[1], 0);
  BOOST_CHECK_EQUAL(a[2], 0);
}

BOOST_AUTO_TEST_CASE(GoArrayTest_ValueConstructor) {
  GoArray<int, 3> a(5);
  BOOST_CHECK_EQUAL(a[0], 5);
  BOOST_CHECK_EQUAL(a[1], 5);
  BOOST_CHECK_EQUAL(a[2], 5);
}

}

