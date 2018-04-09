//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <vector>
#include <boost/test/auto_unit_test.hpp>
#include "platform/SgDebug.h"
#include "lib/SgHashTable.h"
#include "SgABSearch.h"
#include "SgSearchControl.h"
#include "lib/SgVector.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {


class TestSearch
    : public SgABSearch {
 public:

  bool m_write;

  static const size_t NO_NODE;
  TestSearch();
  ~TestSearch();

  void AddNode(size_t father, GoMove move, int eval);
  bool CheckDepthLimitReached() const;
  void Generate(SgVector<GoMove> *moves, int depth);
  int Evaluate(bool *isExact, int depth);
  bool Execute(GoMove move, int *delta, int depth);

  size_t LastEvaluated() const;
  string MoveString(GoMove move) const;
  void TakeBack();
  SgBlackWhite GetToPlay() const;
  void SetToPlay(SgBlackWhite toPlay);
  SgHashCode GetHashCode() const;
  bool EndOfGame() const;

 private:

  struct TestNode {
    size_t m_father;
    size_t m_child;
    size_t m_sibling;
    GoMove m_move;
    int m_eval;
  };
  size_t m_currentNode;
  size_t m_lastEvaluated;
  SgBlackWhite m_toPlay;
  vector<TestNode> m_nodes;
  TestNode &CurrentNode();
  const TestNode &CurrentNode() const;
  TestNode &Node(size_t index);
  const TestNode &Node(size_t index) const;
};
const size_t TestSearch::NO_NODE = numeric_limits<size_t>::max();

TestSearch::TestSearch()
    : SgABSearch(0),
      m_write(false),
      m_currentNode(0),
      m_lastEvaluated(NO_NODE),
      m_toPlay(SG_BLACK) {}

TestSearch::~TestSearch() {}

void TestSearch::AddNode(size_t father, GoMove move, int eval) {
  TestNode node;
  size_t index = m_nodes.size();
  if (father != NO_NODE) {
    DBG_ASSERT(father < m_nodes.size());
    size_t child = m_nodes[father].m_child;
    if (child == NO_NODE)
      m_nodes[father].m_child = index;
    else {
      while (m_nodes[child].m_sibling != NO_NODE)
        child = m_nodes[child].m_sibling;
      m_nodes[child].m_sibling = index;
    }
  }
  node.m_father = father;
  node.m_child = NO_NODE;
  node.m_sibling = NO_NODE;
  node.m_move = move;
  node.m_eval = eval;
  m_nodes.push_back(node);
}

bool TestSearch::CheckDepthLimitReached() const {
  return true;
}

inline TestSearch::TestNode &TestSearch::CurrentNode() {
  return Node(m_currentNode);
}

inline const TestSearch::TestNode &TestSearch::CurrentNode() const {
  return Node(m_currentNode);
}

void TestSearch::Generate(SgVector<GoMove> *moves, int depth) {
  SuppressUnused(depth);
  if (m_write)
    SgDebug() << "TestSearch::Generate " << m_currentNode << ": ";
  size_t child = CurrentNode().m_child;
  while (child != NO_NODE) {
    if (m_write)
      SgDebug() << Node(child).m_move << ' ';
    moves->PushBack(Node(child).m_move);
    child = Node(child).m_sibling;
  }
  if (m_write)
    SgDebug() << '\n';
}

int TestSearch::Evaluate(bool *isExact, int depth) {
  SuppressUnused(depth);
  *isExact = false;
  if (m_write)
    SgDebug() << "TestSearch::Evaluate " << m_currentNode << ": "
              << CurrentNode().m_eval << '\n';
  m_lastEvaluated = m_currentNode;
  return CurrentNode().m_eval;
}

bool TestSearch::Execute(GoMove move, int *delta, int depth) {
  SuppressUnused(depth);
  if (m_write)
    SgDebug() << "TestSearch::Execute: " << move << '\n';
  DBG_ASSERT(*delta == SgABSearch::DEPTH_UNIT);
  SuppressUnused(delta);
  m_toPlay = SgOppBW(m_toPlay);
  size_t child = CurrentNode().m_child;
  while (child != NO_NODE) {
    if (Node(child).m_move == move) {
      m_currentNode = child;
      return true;
    }
    child = Node(child).m_sibling;
  }
  DBG_ASSERT(false);
  return false;
}

inline size_t TestSearch::LastEvaluated() const {
  return m_lastEvaluated;
}

string TestSearch::MoveString(GoMove move) const {
  ostringstream buffer;
  buffer << move;
  return buffer.str();
}

inline TestSearch::TestNode &TestSearch::Node(size_t index) {
  DBG_ASSERT(index < m_nodes.size());
  return m_nodes[index];
}

inline const TestSearch::TestNode &TestSearch::Node(size_t index) const {
  DBG_ASSERT(index < m_nodes.size());
  return m_nodes[index];
}

void TestSearch::TakeBack() {
  if (m_write)
    SgDebug() << "TestSearch::TakeBack\n";
  m_toPlay = SgOppBW(m_toPlay);
  m_currentNode = CurrentNode().m_father;
  DBG_ASSERT(m_currentNode != NO_NODE);
}

SgBlackWhite TestSearch::GetToPlay() const {
  return m_toPlay;
}

SgHashCode TestSearch::GetHashCode() const {
  return SgHashCode(static_cast<unsigned int>(m_currentNode));
}

bool TestSearch::EndOfGame() const {
  return false;
}

void TestSearch::SetToPlay(SgBlackWhite toPlay) {
  m_toPlay = toPlay;
}

} // namespace

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(SgSearchTest_Simple) {
  TestSearch search;
  // Add nodes. Parameters: father, move, eval
  search.AddNode(TestSearch::NO_NODE, GO_NULLMOVE, 0);
  search.AddNode(0, 1, 0);
  search.AddNode(0, 2, 0);
  search.AddNode(0, 3, 0);
  search.AddNode(1, 4, 3);
  search.AddNode(1, 5, 10);
  search.AddNode(2, 6, 4);
  search.AddNode(3, 7, 5);
  search.AddNode(3, 8, 6);
  search.AddNode(3, 9, 7);
  SgVector<GoMove> sequence;
  int value = search.IteratedSearch(1, 10, -1000, 1000, &sequence, true, 0);
  BOOST_CHECK_EQUAL(value, 5);
  BOOST_CHECK_EQUAL(sequence.Length(), 2);
  BOOST_CHECK_EQUAL(sequence[0], 3);
  BOOST_CHECK_EQUAL(sequence[1], 7);
}


BOOST_AUTO_TEST_CASE(SgSearchTest_IncompleteIteration) {
  TestSearch search;
  //search.m_write = true;
  // Add nodes. Parameters: father, move, eval
  search.AddNode(TestSearch::NO_NODE, GO_NULLMOVE, 0);
  search.AddNode(0, 1, -2);
  search.AddNode(0, 2, -5);
  search.AddNode(0, 3, -1);
  search.AddNode(1, 4, 6);
  SgSearchControl *control = new SgNodeSearchControl(6);
  search.SetSearchControl(control);
  SgVector<GoMove> sequence;
  int value = search.IteratedSearch(1, 10, -1000, 1000, &sequence, true, 0);
  BOOST_CHECK_EQUAL(search.LastEvaluated(), 3u);
  BOOST_CHECK_EQUAL(search.Statistics().NumNodes(), 6);
  BOOST_CHECK_EQUAL(value, 5);
  BOOST_CHECK_EQUAL(sequence.Length(), 1);
  BOOST_CHECK_EQUAL(sequence[0], 2);
  delete control;
}


BOOST_AUTO_TEST_CASE(SgSearchTest_NewBestInIncompleteIteration) {
  TestSearch search;
  //search.m_write = true;
  // Add nodes. Parameters: father, move, eval
  search.AddNode(TestSearch::NO_NODE, GO_NULLMOVE, 0);
  search.AddNode(0, 1, -2);
  search.AddNode(0, 2, -5);
  search.AddNode(0, 3, -1);
  search.AddNode(1, 4, 4);
  search.AddNode(1, 5, 3);
  search.AddNode(2, 6, 1);
  search.AddNode(3, 7, 1);
  SgSearchControl *control = new SgNodeSearchControl(11);
  search.SetSearchControl(control);
  SgVector<GoMove> sequence;
  int value = search.IteratedSearch(1, 10, -1000, 1000, &sequence, true, 0);
  BOOST_CHECK_EQUAL(search.LastEvaluated(), 6u);
  BOOST_CHECK_EQUAL(search.Statistics().NumNodes(), 11);
  BOOST_CHECK_EQUAL(value, 3);
  BOOST_CHECK_EQUAL(sequence.Length(), 2);
  BOOST_CHECK_EQUAL(sequence[0], 1);
  BOOST_CHECK_EQUAL(sequence[1], 5);
  delete control;
}

} // namespace

//----------------------------------------------------------------------------

