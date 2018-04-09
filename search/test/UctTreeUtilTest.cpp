//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "UctSearch.h"
#include "UctTreeUtil.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SgUctTreeUtilTest_ApplyFilter) {
  UctSearchTree tree;
  tree.CreateAllocators(1);
  tree.SetMaxNodes(10);
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  const UctNode &root = tree.Root();
  tree.CreateChildren(0, root, moves);

  vector<GoMove> rootFilter;
  rootFilter.push_back(20);

  tree.ApplyFilter(tree, 0, root, rootFilter);
  BOOST_CHECK_EQUAL(root.NumChildren(), 2);
  UctChildNodeIterator it(tree, root);
  BOOST_REQUIRE(it);
  BOOST_CHECK_EQUAL((*it).Move(), 10);
  ++it;
  BOOST_REQUIRE(it);
  BOOST_CHECK_EQUAL((*it).Move(), 30);
}

BOOST_AUTO_TEST_CASE(SgUctTreeUtilTest_ExtractSubtree) {

  UctSearchTree tree;
  tree.CreateAllocators(1);
  tree.SetMaxNodes(10);
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  const UctNode *node;
  node = &tree.Root();
  tree.CreateChildren(0, *node, moves);

  node = UctTreeUtil::FindChildWithMove(tree, *node, 20);
  moves.clear();
  moves.push_back(UctMoveInfo(40));
  moves.push_back(UctMoveInfo(50));
  tree.CreateChildren(0, *node, moves);

  node = UctTreeUtil::FindChildWithMove(tree, *node, 50);
  moves.clear();
  moves.push_back(UctMoveInfo(60));
  moves.push_back(UctMoveInfo(70));
  tree.CreateChildren(0, *node, moves);

  BOOST_REQUIRE_NO_THROW(tree.CheckConsistency());

  UctSearchTree target;
  target.CreateAllocators(1);
  target.SetMaxNodes(10);
  vector<GoMove> sequence;
  sequence.push_back(20);
  sequence.push_back(50);
  UctTreeUtil::ExtractSubtree(tree, target, sequence, false);
  BOOST_REQUIRE_NO_THROW(target.CheckConsistency());
  BOOST_CHECK_EQUAL(3u, target.NuNodes());

  node = UctTreeUtil::FindChildWithMove(target, target.Root(), 60);
  BOOST_CHECK(node != 0);
  node = UctTreeUtil::FindChildWithMove(target, target.Root(), 70);
  BOOST_CHECK(node != 0);
}


BOOST_AUTO_TEST_CASE(SgUctTreeUtilTest_ExtractSubtree_Overflow) {
  UctSearchTree tree;
  tree.CreateAllocators(2);
  tree.SetMaxNodes(10); // 5 nodes per allocator
  vector<UctMoveInfo> moves;
  moves.push_back(UctMoveInfo(10));
  moves.push_back(UctMoveInfo(20));
  moves.push_back(UctMoveInfo(30));
  const UctNode *node;
  node = &tree.Root();
  tree.CreateChildren(0, *node, moves); // 3 nodes into allocator 0

  node = UctTreeUtil::FindChildWithMove(tree, *node, 10);
  moves.clear();
  moves.push_back(UctMoveInfo(40));
  moves.push_back(UctMoveInfo(50));
  tree.CreateChildren(0, *node, moves); // 2 nodes into allocator 0

  node = UctTreeUtil::FindChildWithMove(tree, *node, 40);
  moves.clear();
  moves.push_back(UctMoveInfo(60));
  moves.push_back(UctMoveInfo(70));
  moves.push_back(UctMoveInfo(80));
  tree.CreateChildren(1, *node, moves); // 3 nodes into allocator 1

  // In the buggy implementation, the target allocator was cycling
  // while traversing to the source tree, therefore target allocator 0
  // had 6 nodes.

  UctSearchTree target;
  target.CreateAllocators(2);
  target.SetMaxNodes(10);
  vector<GoMove> sequence;
  UctTreeUtil::ExtractSubtree(tree, target, sequence, false);

  // We don't care, if ExtractSubtree cuts the target tree or ensures
  // that the target nodes are distributed evenly, as long as none of the
  // target allocators overflows.
  // Note that in the old implementation this triggered an assertion in
  // debug mode, but in release mode the allocator capacity was exceeded,
  // which triggered a reallocation in the vector and invalidated
  // all node pointer members in the UctNode elements.

  BOOST_CHECK(target.NuNodes(0) <= 5);
  BOOST_CHECK(target.NuNodes(1) <= 5);
}

} // namespace

//----------------------------------------------------------------------------
