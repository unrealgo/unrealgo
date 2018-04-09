//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "platform/SgSystem.h"

#include <sstream>
#include <vector>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "platform/SgDebug.h"
#include "UctSearch.h"
#include "UctTreeUtil.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {


const bool WRITE = false;

static const size_t NO_NODE = numeric_limits<size_t>::max();


const UctNode *GetNode(const UctSearchTree &tree, GoMove move1) {
  return UctTreeUtil::FindChildWithMove(tree, tree.Root(), move1);
}



//----------------------------------------------------------------------------


struct TestNode {
  size_t m_father;
  size_t m_child;
  size_t m_sibling;
  GoMove m_move;
  float m_eval;
  bool m_isLeaf;
};

//----------------------------------------------------------------------------


class TestThreadState
    : public UctThreadState {
 public:
  TestThreadState(unsigned int threadId, const vector<TestNode> &nodes);



  // @{

  void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures);
  void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures, int maxSteps);
  UctValueType Evaluate();
  void Apply(GoMove);
  void Execute(GoMove move);
  void ExecutePlayout(GoMove move);
  bool GenerateAllMoves(UctValueType count, vector<UctMoveInfo> &moves,
                        UctProvenType &provenType);
  GoMove GeneratePlayoutMove(bool &skipRaveUpdate);
  bool TrompTaylorPassWins();
  const GoBoard &Board();
  void StartSearch();
  void TakeBackInTree(size_t nuMoves);
  void TakeBackPlayout(size_t nuMoves);
  SgBlackWhite ToPlay() const;
  UctValueType FinalScore();
  bool WinTheGame();
  GoMove LastMove();
  // @} // @name

 private:
  GoBoard m_board;
  size_t m_currentNode;
  SgBlackWhite m_toPlay;
  const vector<TestNode> &m_nodes;
  const TestNode &CurrentNode() const;
  const TestNode &Node(size_t index) const;
  void TakeBack(size_t nuMoves);
};

TestThreadState::TestThreadState(unsigned int threadId,
                                 const vector<TestNode> &nodes)
    : UctThreadState(threadId),
      m_currentNode(0),
      m_toPlay(SG_BLACK),
      m_nodes(nodes) {}

inline const TestNode &TestThreadState::CurrentNode() const {
  return Node(m_currentNode);
}

void TestThreadState::CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures, int maxSteps) {
}

void TestThreadState::CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures) {
}

void TestThreadState::Apply(GoMove move) {
}

void TestThreadState::Execute(GoMove move) {
  if (WRITE)
    SgDebug() << "TestUctSearch::Execute: " << move << '\n';
  m_toPlay = SgOppBW(m_toPlay);
  size_t child = CurrentNode().m_child;
  while (child != NO_NODE) {
    if (Node(child).m_move == move) {
      m_currentNode = child;
      return;
    }
    child = Node(child).m_sibling;
  }
  DBG_ASSERT(false);
}

void TestThreadState::ExecutePlayout(GoMove move) {
  Execute(move);
}

UctValueType TestThreadState::Evaluate() {
  if (WRITE)
    SgDebug() << "TestUctSearch::Evaluate " << m_currentNode << ": "
              << CurrentNode().m_eval << '\n';
  DBG_ASSERT(CurrentNode().m_isLeaf);
  return CurrentNode().m_eval;
}

bool TestThreadState::GenerateAllMoves(UctValueType count,
                                       vector<UctMoveInfo> &moves,
                                       UctProvenType &provenType) {
  SuppressUnused(provenType);
  if (WRITE)
    SgDebug() << "TestUctSearch::Generate " << m_currentNode << ": ";
  size_t child = CurrentNode().m_child;
  while (child != NO_NODE) {
    if (WRITE)
      SgDebug() << Node(child).m_move << ' ';
    moves.push_back(UctMoveInfo(Node(child).m_move));
    child = Node(child).m_sibling;
  }

  // Test knowledge expansion: give all children boost of one win,
  // remove last child, and add new move with move 100 and (value, count)
  // of (1.0, 10)
  if (count > 0) {
    moves.pop_back();
    for (size_t i = 0; i < moves.size(); ++i) {
      moves[i].uct_value = 1.0;
      moves[i].visit_count = 1;
    }
    moves.push_back(UctMoveInfo(100, 1.0, 10, 0, 0));
  }

  if (WRITE)
    SgDebug() << '\n';

  return false;
}

GoMove TestThreadState::GeneratePlayoutMove(bool &skipRaveUpdate) {
  SuppressUnused(skipRaveUpdate);
  // Search does not use randomness
  vector<UctMoveInfo> moves;
  UctProvenType provenType = PROVEN_NONE;
  GenerateAllMoves(0, moves, provenType);
  if (moves.empty())
    return GO_NULLMOVE;
  else
    return moves.begin()->uct_move;
}

inline const TestNode &TestThreadState::Node(size_t index) const {
  DBG_ASSERT(index < m_nodes.size());
  return m_nodes[index];
}

bool TestThreadState::TrompTaylorPassWins() {
  return false;
}

const GoBoard &TestThreadState::Board() {
  return m_board;
}

void TestThreadState::StartSearch() {}

void TestThreadState::TakeBack(size_t nuMoves) {
  if (WRITE)
    SgDebug() << "TestUctSearch::TakeBack\n";
  for (size_t i = 1; i <= nuMoves; ++i) {
    m_toPlay = SgOppBW(m_toPlay);
    m_currentNode = CurrentNode().m_father;
    DBG_ASSERT(m_currentNode != NO_NODE);
  }
}

void TestThreadState::TakeBackInTree(size_t nuMoves) {
  TakeBack(nuMoves);
}

void TestThreadState::TakeBackPlayout(size_t nuMoves) {
  TakeBack(nuMoves);
}

SgBlackWhite TestThreadState::ToPlay() const {
  return m_toPlay;
}

UctValueType TestThreadState::FinalScore() {
  return 0;
}

bool TestThreadState::WinTheGame() {
  return false;
}

GoMove TestThreadState::LastMove() {
  return GO_NULLMOVE;
}

//----------------------------------------------------------------------------

class TestThreadStateFactory
    : public UctThreadStateFactory {
 public:
  TestThreadStateFactory(const vector<TestNode> &nodes);
  UctThreadState *Create(unsigned int threadId, const UctSearch &search);

 private:
  const vector<TestNode> &m_nodes;
};

TestThreadStateFactory::TestThreadStateFactory(const vector<TestNode> &nodes)
    : m_nodes(nodes) {}

UctThreadState *TestThreadStateFactory::Create(unsigned int threadId,
                                                 const UctSearch &search) {
  SuppressUnused(search);
  return new TestThreadState(threadId, m_nodes);
}

//----------------------------------------------------------------------------


class TestUctSearch
    : public UctSearch {
 public:
  TestUctSearch();
  ~TestUctSearch();


  // @{


  void AddLeafNode(size_t father, GoMove move, float eval);

  void AddNode(size_t father, GoMove move);

  // @} // @name


  // @{

  string MoveString(GoMove move) const;
  UctValueType UnknownEval() const;

  // @} // @name

 private:
  vector<TestNode> m_nodes;
  void AddNode(size_t father, GoMove move, bool isLeaf, float eval);
};

TestUctSearch::TestUctSearch()
    : UctSearch(new TestThreadStateFactory(m_nodes), 0) {}

TestUctSearch::~TestUctSearch() {}

inline void TestUctSearch::AddLeafNode(size_t father, GoMove move, float eval) {
  const bool isLeaf = true;
  AddNode(father, move, isLeaf, eval);
}

inline void TestUctSearch::AddNode(size_t father, GoMove move) {
  const bool isLeaf = false;
  const float dummyEval = 0;
  AddNode(father, move, isLeaf, dummyEval);
}

void TestUctSearch::AddNode(size_t father, GoMove move, bool isLeaf,
                            float eval) {
  DBG_ASSERT(father == NO_NODE || !m_nodes[father].m_isLeaf);
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
  node.m_isLeaf = isLeaf;
  m_nodes.push_back(node);
}

string TestUctSearch::MoveString(GoMove move) const {
  ostringstream buffer;
  buffer << move;
  return buffer.str();
}

UctValueType TestUctSearch::UnknownEval() const {
  // UnknownEval() is only called by UctSearch if maximum game length was
  // exceeded, which should not happen with Search
  DBG_ASSERT(false);
  return 0;
}

} // namespace

//----------------------------------------------------------------------------

namespace {


BOOST_AUTO_TEST_CASE(SgUctSearchTest_Simple) {
  TestUctSearch search;
  search.SetExpandThreshold(1);
  //search.m_write = true;

  // Add nodes. Parameters: father, move ( = target node), [eval]
  search.AddNode(NO_NODE, GO_NULLMOVE);
  search.AddNode(0, 1);
  search.AddNode(0, 2);
  search.AddNode(0, 3);
  search.AddNode(0, 4);
  search.AddLeafNode(1, 5, 0.f);
  search.AddLeafNode(1, 6, 1.f);
  search.AddLeafNode(2, 7, 1.f);
  search.AddLeafNode(2, 8, 1.f);
  search.AddLeafNode(3, 9, 1.f);
  search.AddLeafNode(3, 10, 0.f);
  search.AddLeafNode(4, 11, 0.f);
  search.AddLeafNode(4, 12, 0.f);

  search.PreStartSearch();


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(0.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(2u, sequence.size());
    BOOST_CHECK_EQUAL(1, sequence[0]);
    BOOST_CHECK_EQUAL(5, sequence[1]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(1u, tree.NuNodes());
    BOOST_CHECK_EQUAL(1u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), tree.Root().Mean(), 1e-3f);
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(0.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(2u, sequence.size());
    BOOST_CHECK_EQUAL(1, sequence[0]);
    BOOST_CHECK_EQUAL(5, sequence[1]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(5u, tree.NuNodes());
    BOOST_CHECK_EQUAL(2u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 1)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(1.0), GetNode(tree, 1)->Mean(), 1e-3f);
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(1.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(2u, sequence.size());
    BOOST_CHECK_EQUAL(2, sequence[0]);
    BOOST_CHECK_EQUAL(7, sequence[1]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(5u, tree.NuNodes());
    BOOST_CHECK_EQUAL(3u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.33), tree.Root().Mean(), 2.f);
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 2)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), GetNode(tree, 2)->Mean(), 1e-3f);
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(1.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(2u, sequence.size());
    BOOST_CHECK_EQUAL(3, sequence[0]);
    BOOST_CHECK_EQUAL(9, sequence[1]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(5u, tree.NuNodes());
    BOOST_CHECK_EQUAL(4u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.5), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 3)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), GetNode(tree, 3)->Mean(), 1e-3f);
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(0.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(2u, sequence.size());
    BOOST_CHECK_EQUAL(4, sequence[0]);
    BOOST_CHECK_EQUAL(11, sequence[1]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(5u, tree.NuNodes());
    BOOST_CHECK_EQUAL(5u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.4), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 4)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(1.0), GetNode(tree, 4)->Mean(), 1e-3f);
  }
}

//----------------------------------------------------------------------------


BOOST_AUTO_TEST_CASE(SgUctSearchTest_Knowledge) {
  TestUctSearch search;
  search.SetExpandThreshold(1);
  std::vector<UctValueType> thresholds(1, 4);

  // Add nodes. Parameters: father, move ( = target node), [eval]
  search.AddNode(NO_NODE, GO_NULLMOVE);
  search.AddLeafNode(0, 1, 1.f);
  search.AddLeafNode(0, 2, 0.f);
  search.AddLeafNode(0, 3, 0.f);

  search.PreStartSearch();


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(0.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(1u, sequence.size());
    BOOST_CHECK_EQUAL(1, sequence[0]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(1u, tree.NuNodes());
    BOOST_CHECK_EQUAL(1u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(PROVEN_NONE, tree.Root().ProvenType());
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(0.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(1u, sequence.size());
    BOOST_CHECK_EQUAL(1, sequence[0]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(4u, tree.NuNodes());
    BOOST_CHECK_EQUAL(2u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(PROVEN_NONE, tree.Root().ProvenType());
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 1)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(1.0), GetNode(tree, 1)->Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(PROVEN_NONE, GetNode(tree, 1)->ProvenType());
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(1.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(1u, sequence.size());
    BOOST_CHECK_EQUAL(2, sequence[0]);
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(4u, tree.NuNodes());
    BOOST_CHECK_EQUAL(3u, tree.Root().MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.33), tree.Root().Mean(), 2.f);
    BOOST_CHECK_EQUAL(PROVEN_WIN, tree.Root().ProvenType());
    BOOST_CHECK_EQUAL(1u, GetNode(tree, 2)->MoveCount());
    BOOST_CHECK_CLOSE(UctValueType(0.0), GetNode(tree, 2)->Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(PROVEN_LOSS, GetNode(tree, 2)->ProvenType());
  }


  search.PlayGame();
  {
    const UctGameInfo &info = search.LastGameInfo();
    BOOST_CHECK_CLOSE(UctValueType(1.0), info.m_eval[0], 1e-3f);
    const vector<GoMove> &sequence = info.m_sequence[0];
    BOOST_CHECK_EQUAL(0u, sequence.size());
    const UctSearchTree &tree = search.Tree();
    BOOST_CHECK_EQUAL(4u, tree.NuNodes());
    BOOST_CHECK_EQUAL(4u, tree.Root().MoveCount());
    BOOST_CHECK_EQUAL(2u, tree.Root().PosCount());
    BOOST_CHECK_CLOSE(UctValueType(0.5), tree.Root().Mean(), 1e-3f);
    BOOST_CHECK_EQUAL(PROVEN_WIN, tree.Root().ProvenType());
  }
}
}

//----------------------------------------------------------------------------
