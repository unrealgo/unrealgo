//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "UctSearchTree.h"
#include "UctTreeUtil.h"

using std::vector;
using UctTreeUtil::FindChildWithMove;
using UctValueUtil::InverseValue;

//----------------------------------------------------------------------------

namespace {

const float EPSILON = 1e-6f;


BOOST_AUTO_TEST_CASE(SgUctMoveInfo_Add) {
  GoMove move = 0;
  const UctValueType v = 0.8;
  const UctValueType count = 10;
  UctMoveInfo info(move, InverseValue(v), count, v, count);
  info.Add(0.0, 10);
  BOOST_CHECK_CLOSE(info.uct_value, InverseValue(0.4), EPSILON);
  BOOST_CHECK_EQUAL(info.visit_count, 20);
  BOOST_CHECK_CLOSE(info.rave_value, 0.4, EPSILON);
  BOOST_CHECK_EQUAL(info.rave_cnt, 20);
}


BOOST_AUTO_TEST_CASE(SgUctMoveInfo_Add_2) {
  GoMove move = 0;
  const UctValueType v = 0.5;
  const UctValueType count = 1;
  UctMoveInfo info(move, InverseValue(v), count, v, count);
  info.Add(1.0, 3);
  BOOST_CHECK_CLOSE(info.uct_value, InverseValue(7.0 / 8), EPSILON);
  BOOST_CHECK_EQUAL(info.visit_count, 4);
  BOOST_CHECK_CLOSE(info.rave_value, 7.0 / 8, EPSILON);
  BOOST_CHECK_EQUAL(info.rave_cnt, 4);
}



BOOST_AUTO_TEST_CASE(SgUctTreeIteratorTest_Simple) {

  UctSearchTree tree;
  tree.CreateAllocators(1);
  tree.SetMaxNodes(10);
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  const UctNode &root = tree.Root();
  tree.CreateChildren(0, root, moves);
  const UctNode &node1 = *FindChildWithMove(tree, root, 10);
  const UctNode &node2 = *FindChildWithMove(tree, root, 20);
  const UctNode &node3 = *FindChildWithMove(tree, root, 30);

  moves.clear();
  moves.push_back(UctMoveInfo(40));
  tree.CreateChildren(0, node2, moves);
  const UctNode &node4 = *FindChildWithMove(tree, node2, 40);

  // Check that iteration works and is depth-first
  UctTreeIterator it(tree);
  BOOST_CHECK_EQUAL(&root, &(*it));
  ++it;
  BOOST_CHECK_EQUAL(&node1, &(*it));
  ++it;
  BOOST_CHECK_EQUAL(&node2, &(*it));
  ++it;
  BOOST_CHECK_EQUAL(&node4, &(*it));
  ++it;
  BOOST_CHECK_EQUAL(&node3, &(*it));
  ++it;
  BOOST_CHECK(!it);
}

BOOST_AUTO_TEST_CASE(SgUctTreeIteratorTest_Index) {

  UctSearchTree tree;
  tree.CreateAllocators(1);
  tree.SetMaxNodes(10);
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  moves.push_back(UctMoveInfo(25));
  moves.push_back(UctMoveInfo(35));
  const UctNode &root = tree.Root();
  tree.CreateChildren(0, root, moves);
  const UctNode &node1 = *FindChildWithMove(tree, root, 10);
  const UctNode &node2 = *FindChildWithMove(tree, root, 20);
  const UctNode &node3 = *FindChildWithMove(tree, root, 30);
  UctChildNodeIterator it(tree, root);

  ++it;
  ++it;
  ++it;

  const UctNode &cur = *it;
  size_t index = it() - root.FirstChild();
  BOOST_CHECK_EQUAL(index, 3);
  BOOST_CHECK_EQUAL(cur.Move(), 25);

}


BOOST_AUTO_TEST_CASE(SgUctTreeIteratorTest_OnlyRoot) {
  UctSearchTree tree;
  UctTreeIterator it(tree);
  BOOST_CHECK(it);
  BOOST_CHECK_EQUAL(&tree.Root(), &(*it));
  ++it;
  BOOST_CHECK(!it);
}


BOOST_AUTO_TEST_CASE(SgUctTreeIteratorTest_ApplyFilter) {
  UctSearchTree tree;
  tree.CreateAllocators(1);
  tree.SetMaxNodes(100);
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  const UctNode &root = tree.Root();
  tree.CreateChildren(0, root, moves);
  const UctNode &node1 = *FindChildWithMove(tree, root, 10);
  const UctNode &node2 = *FindChildWithMove(tree, root, 20);
  const UctNode &node3 = *FindChildWithMove(tree, root, 30);
  tree.AddGameResult(node1, &root, 1.f);
  tree.AddGameResult(node2, &root, 1.f);
  tree.AddGameResult(node3, &root, 0.f);
  tree.AddGameResult(node3, &root, 1.f);
  vector<GoMove> rootFilter;
  rootFilter.push_back(20);
  tree.ApplyFilter(tree, 0, root, rootFilter);
  BOOST_CHECK_EQUAL(root.NumChildren(), 2);
  UctChildNodeIterator it(tree, root);
  BOOST_CHECK_EQUAL((*it).Move(), 10);
  BOOST_CHECK_EQUAL((*it).MoveCount(), 1u);
  BOOST_CHECK_CLOSE((*it).Mean(), UctValueType(1.0), 1e-4);
  ++it;
  BOOST_CHECK_EQUAL((*it).Move(), 30);
  BOOST_CHECK_EQUAL((*it).MoveCount(), 2u);
  BOOST_CHECK_CLOSE((*it).Mean(), UctValueType(0.5), 1e-4);
}

BOOST_AUTO_TEST_CASE(SgUctTreeIteratorTest_TreeSwap) {
  UctSearchTree treeLeft;
  UctSearchTree treeRight;
  GoMove move = GO_MAX_SIZE / 2;
  treeLeft.Root() = UctNode(GO_PASS);
  treeLeft.Root().SetColor(SG_BLACK);
  treeRight.Root() = UctNode(move);
  treeRight.Root().SetColor(SG_WHITE);
  treeLeft.Swap(treeRight);
  BOOST_CHECK_EQUAL(treeLeft.Root().Move(), move);
  BOOST_CHECK_EQUAL(treeRight.Root().Move(), GO_PASS);

  BOOST_CHECK_EQUAL(treeLeft.Root().GetColor(), SG_WHITE);
  BOOST_CHECK_EQUAL(treeRight.Root().GetColor(), SG_BLACK);
}

} // namespace

//----------------------------------------------------------------------------
