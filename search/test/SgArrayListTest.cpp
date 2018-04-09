//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "lib/SgArrayList.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(GoArrayListTest_PushBack) {
  GoArrayList<int, 10> a;
  BOOST_CHECK_EQUAL(a.Length(), 0);
  a.PushBack(1);
  BOOST_CHECK_EQUAL(a.Length(), 1);
  BOOST_CHECK_EQUAL(a[0], 1);
  a.PushBack(2);
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_PushBack_List) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  GoArrayList<int, 10> b;
  b.PushBack(1);
  b.PushBack(2);
  b.PushBack(3);
  a.PushBackList(b);
  BOOST_CHECK_EQUAL(a.Length(), 5);
  BOOST_CHECK_EQUAL(b.Length(), 3);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
  BOOST_CHECK_EQUAL(a[2], 1);
  BOOST_CHECK_EQUAL(a[3], 2);
  BOOST_CHECK_EQUAL(a[4], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Assign) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  GoArrayList<int, 10> b;
  b = a;
  BOOST_CHECK_EQUAL(b.Length(), 3);
  BOOST_REQUIRE(b.Length() == 3);
  BOOST_CHECK_EQUAL(b[0], 1);
  BOOST_CHECK_EQUAL(b[1], 2);
  BOOST_CHECK_EQUAL(b[2], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Clear) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.Clear();
  BOOST_CHECK_EQUAL(a.Length(), 0);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_ConstructorCopy) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  GoArrayList<int, 10> b(a);
  BOOST_CHECK_EQUAL(b.Length(), 3);
  BOOST_REQUIRE(b.Length() == 3);
  BOOST_CHECK_EQUAL(b[0], 1);
  BOOST_CHECK_EQUAL(b[1], 2);
  BOOST_CHECK_EQUAL(b[2], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_ConstructorDefault) {
  GoArrayList<int, 10> a;
  BOOST_CHECK_EQUAL(a.Length(), 0);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_ConstructorValue) {
  GoArrayList<int, 10> a(5);
  BOOST_CHECK_EQUAL(a.Length(), 1);
  BOOST_REQUIRE(a.Length() == 1);
  BOOST_CHECK_EQUAL(a[0], 5);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Contains) {
  GoArrayList<int, 10> a;
  BOOST_CHECK(!a.Contains(5));
  a.PushBack(1);
  a.PushBack(2);
  BOOST_CHECK(!a.Contains(5));
  a.PushBack(5);
  BOOST_CHECK(a.Contains(5));
  a.PushBack(6);
  BOOST_CHECK(a.Contains(5));
  a.PushBack(5);
  BOOST_CHECK(a.Contains(5));
  BOOST_CHECK(!a.Contains(10));
  BOOST_CHECK(!a.Contains(0));
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Elements) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
  BOOST_CHECK_EQUAL(a[2], 3);
  const GoArrayList<int, 10> &b = a;
  BOOST_CHECK_EQUAL(b[0], 1);
  BOOST_CHECK_EQUAL(b[1], 2);
  BOOST_CHECK_EQUAL(b[2], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Equals) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  BOOST_CHECK(a == a);
  BOOST_CHECK(!(a != a));
  GoArrayList<int, 10> b;
  b.PushBack(1);
  b.PushBack(2);
  b.PushBack(3);
  BOOST_CHECK(b == a);
  BOOST_CHECK(!(b != a));
  BOOST_CHECK(a == b);
  BOOST_CHECK(!(a != b));
  GoArrayList<int, 10> c;
  c.PushBack(1);
  c.PushBack(2);
  c.PushBack(2);
  BOOST_CHECK(c != a);
  BOOST_CHECK(!(c == a));
  BOOST_CHECK(c != b);
  BOOST_CHECK(!(c == b));
  GoArrayList<int, 10> d;
  d.PushBack(1);
  d.PushBack(2);
  BOOST_CHECK(d != a);
  BOOST_CHECK(!(d == a));
  BOOST_CHECK(d != c);
  BOOST_CHECK(!(d == c));
  GoArrayList<int, 10> e;
  e.PushBack(1);
  e.PushBack(2);
  e.PushBack(3);
  e.PushBack(4);
  BOOST_CHECK(e != a);
  BOOST_CHECK(!(e == a));
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Exclude_1) {
  GoArrayList<int, 3> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  BOOST_CHECK(a.Exclude(1));
  BOOST_CHECK(!a.Exclude(5));
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_CHECK_EQUAL(a[0], 3);
  BOOST_CHECK_EQUAL(a[1], 2);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Exclude_2) {
  GoArrayList<int, 3> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  BOOST_CHECK(a.Exclude(2));
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Exclude_3) {
  GoArrayList<int, 3> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  BOOST_CHECK(a.Exclude(3));
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Include) {
  GoArrayList<int, 10> a;
  a.PushBack(2);
  a.PushBack(1);
  a.PushBack(3);
  bool result = a.Include(5);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(a.Length(), 4);
  BOOST_CHECK_EQUAL(a[0], 2);
  BOOST_CHECK_EQUAL(a[1], 1);
  BOOST_CHECK_EQUAL(a[2], 3);
  BOOST_CHECK_EQUAL(a[3], 5);
  result = a.Include(5);
  BOOST_CHECK_EQUAL(result, false);
  BOOST_CHECK_EQUAL(a.Length(), 4);
  BOOST_CHECK_EQUAL(a[0], 2);
  BOOST_CHECK_EQUAL(a[1], 1);
  BOOST_CHECK_EQUAL(a[2], 3);
  BOOST_CHECK_EQUAL(a[3], 5);
  result = a.Include(3);
  BOOST_CHECK_EQUAL(result, false);
  BOOST_CHECK_EQUAL(a.Length(), 4);
  result = a.Include(-5);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(a.Length(), 5);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Intersect) {
  GoArrayList<int, 10> a;
  GoArrayList<int, 10> b;
  GoArrayList<int, 10> c = a.Intersect(b);
  BOOST_CHECK_EQUAL(c.Length(), 0);
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  c = a.Intersect(b);
  BOOST_CHECK_EQUAL(c.Length(), 0);
  b.PushBack(2);
  b.PushBack(4);
  b.PushBack(3);
  c = a.Intersect(b);
  BOOST_CHECK_EQUAL(c.Length(), 2);
  BOOST_REQUIRE(c.Length() == 2);
  BOOST_CHECK(c.Contains(2));
  BOOST_CHECK(c.Contains(3));
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_IsEmpty) {
  GoArrayList<int, 10> a;
  BOOST_CHECK(a.IsEmpty());
  a.PushBack(1);
  BOOST_CHECK(!a.IsEmpty());
  a.Clear();
  BOOST_CHECK(a.IsEmpty());
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Iterator) {
  GoArrayList<int, 10> a;
  GoArrayList<int, 10>::Iterator i(a);
  BOOST_CHECK(!i);
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  GoArrayList<int, 10>::Iterator j(a);
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 1);
  ++j;
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 2);
  ++j;
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 3);
  ++j;
  BOOST_CHECK(!j);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Last) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  BOOST_CHECK_EQUAL(a.Last(), 1);
  a.PushBack(2);
  BOOST_CHECK_EQUAL(a.Last(), 2);
  a.PushBack(3);
  BOOST_CHECK_EQUAL(a.Last(), 3);
  // Check that last returns a reference, not a copy
  a.Last() = 5;
  BOOST_CHECK_EQUAL(a.Last(), 5);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Length) {
  GoArrayList<int, 10> a;
  BOOST_CHECK_EQUAL(a.Length(), 0);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_NonConstIterator) {
  GoArrayList<int, 10> a;
  GoArrayList<int, 10>::NonConstIterator i(a);
  BOOST_CHECK(!i);
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  GoArrayList<int, 10>::NonConstIterator j(a);
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 1);
  *j = 0;
  BOOST_CHECK(j);
  ++j;
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 2);
  *j = 0;
  ++j;
  BOOST_CHECK(j);
  BOOST_CHECK_EQUAL(*j, 3);
  *j = 0;
  ++j;
  BOOST_CHECK(!j);
  BOOST_CHECK_EQUAL(a.Length(), 3);
  BOOST_CHECK_EQUAL(a[0], 0);
  BOOST_CHECK_EQUAL(a[1], 0);
  BOOST_CHECK_EQUAL(a[2], 0);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_PopBack) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  a.PopBack();
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_REQUIRE(a.Length() == 2);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
  a.PopBack();
  BOOST_CHECK_EQUAL(a.Length(), 1);
  BOOST_REQUIRE(a.Length() == 1);
  BOOST_CHECK_EQUAL(a[0], 1);
  a.PopBack();
  BOOST_CHECK_EQUAL(a.Length(), 0);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_RemoveFirst) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  a.RemoveFirst(2);
  BOOST_CHECK_EQUAL(a.Length(), 2);
  BOOST_REQUIRE(a.Length() == 2);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 3);
  a.RemoveFirst(1);
  BOOST_CHECK_EQUAL(a.Length(), 1);
  BOOST_REQUIRE(a.Length() == 1);
  BOOST_CHECK_EQUAL(a[0], 3);
  a.RemoveFirst(1);
  BOOST_CHECK_EQUAL(a.Length(), 1);
  BOOST_REQUIRE(a.Length() == 1);
  BOOST_CHECK_EQUAL(a[0], 3);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Resize) {
  GoArrayList<int, 10> a;
  a.PushBack(7);
  a.Resize(5);
  BOOST_CHECK_EQUAL(a.Length(), 5);
  BOOST_CHECK_EQUAL(a[0], 7);
  a.Resize(0);
  BOOST_CHECK_EQUAL(a.Length(), 0);
  a.Resize(10);
  BOOST_CHECK_EQUAL(a.Length(), 10);
  BOOST_CHECK_EQUAL(a[0], 7);
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_SameElements) {
  GoArrayList<int, 10> a;
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(3);
  GoArrayList<int, 10> b;
  b.PushBack(2);
  b.PushBack(1);
  b.PushBack(3);
  BOOST_CHECK(a.SameElements(b));
  b.Clear();
  BOOST_CHECK(!a.SameElements(b));
  b.PushBack(2);
  b.PushBack(1);
  b.PushBack(3);
  b.PushBack(4);
  BOOST_CHECK(!a.SameElements(b));
}

BOOST_AUTO_TEST_CASE(GoArrayListTest_Sort) {
  GoArrayList<int, 10> a;
  a.PushBack(3);
  a.PushBack(1);
  a.PushBack(2);
  a.PushBack(4);
  a.Sort();
  BOOST_CHECK_EQUAL(a.Length(), 4);
  BOOST_CHECK_EQUAL(a[0], 1);
  BOOST_CHECK_EQUAL(a[1], 2);
  BOOST_CHECK_EQUAL(a[2], 3);
  BOOST_CHECK_EQUAL(a[3], 4);
}

} // namespace

//----------------------------------------------------------------------------

