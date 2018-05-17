
#include "platform/SgSystem.h"
#include "GoUctSearch.h"

#include <fstream>
#include <iostream>
#include "GoBoardUtil.h"
#include "GoNodeUtil.h"
#include "GoUctUtil.h"
#include "platform/SgDebug.h"
#include "SgGameWriter.h"
#include "SgNode.h"
#include "UctTreeUtil.h"

namespace {
const int MOVERANGE = GO_PASS + 1;

SgNode *AppendChild(SgNode *node, const std::string &comment) {
  SgNode *child = node->NewRightMostSon();
  child->AddComment(comment);
  return child;
}

SgNode *AppendChild(SgNode *node, SgBlackWhite color, GoPoint move) {
  SgNode *child = node->NewRightMostSon();
  SgPropID propId =
      (color == SG_BLACK ? SG_PROP_MOVE_BLACK : SG_PROP_MOVE_WHITE);
  child->Add(new SgPropMove(propId, move));
  return child;
}

void AppendGame(SgNode *node, UctValueType gameNumber, std::size_t threadId,
                SgBlackWhite toPlay, const UctGameInfo &info) {
  DBG_ASSERT(node != 0);
  {
    std::ostringstream comment;
    comment << "Thread " << threadId << '\n'
            << "Game " << gameNumber << '\n';
    node = AppendChild(node, comment.str());
  }
  size_t nuMovesInTree = info.m_inTreeSequence.size();
  for (size_t i = 0; i < nuMovesInTree; ++i) {
    node = AppendChild(node, toPlay, info.m_inTreeSequence[i]);
    toPlay = SgOppBW(toPlay);
  }
  SgNode *lastInTreeNode = node;
  SgBlackWhite lastInTreeToPlay = toPlay;
  for (size_t i = 0; i < info.m_eval.size(); ++i) {
    node = lastInTreeNode;
    toPlay = lastInTreeToPlay;
    std::ostringstream comment;
    comment << "Playout " << i << '\n'
            << "Eval " << info.m_eval[i] << '\n'
            << "Aborted " << info.m_aborted[i] << '\n';
    node = AppendChild(node, comment.str());
    for (size_t j = nuMovesInTree; j < info.m_sequence[i].size(); ++j) {
      node = AppendChild(node, toPlay, info.m_sequence[i][j]);
      toPlay = SgOppBW(toPlay);
    }
  }
}

} // namespace

GoUctState::AssertionHandler::AssertionHandler(const GoUctState &state)
    : m_state(state) {}

void GoUctState::AssertionHandler::Run() {
  m_state.Dump(SgDebug());
}

GoUctState::GoUctState(unsigned int threadId, const GoBoard &bd)
    : UctThreadState(threadId, MOVERANGE),
      m_assertionHandler(*this),
      m_uctBd(bd),
      m_synchronizer(bd),
      m_gameLength(0) {
  m_synchronizer.SetSubscriber(m_bd);
  m_isInPlayout = false;
}

void GoUctState::Dump(std::ostream &out) const {
  out << "GoUctState[" << thread_id << "] ";
  if (m_isInPlayout)
    out << "playout board:\n" << m_uctBd;
  else
    out << "board:\n" << m_bd;
}

void GoUctState::Execute(GoMove move) {
  DBG_ASSERT(!m_isInPlayout);
  DBG_ASSERT(move == GO_PASS || !m_bd.Occupied(move));
  // Temporarily switch ko rule to SIMPLEKO to avoid slow full board
  // repetition test in GoBoard::Play()
  GoRestoreKoRule restoreKoRule(m_bd);
  m_bd.Rules().SetKoRule(GoRules::SIMPLEKO);
  m_bd.Play(move);
  DBG_ASSERT(!m_bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL));
  ++m_gameLength;
}

void GoUctState::Apply(GoMove move) {
  // TODO
  DBG_ASSERT(!m_isInPlayout);
  DBG_ASSERT(move == GO_PASS || !m_bd.Occupied(move));

  GoRestoreKoRule restoreKoRule(m_bd);
  m_bd.Rules().SetKoRule(GoRules::SIMPLEKO);
  m_bd.Play(move);
  ++m_gameLength;
}

GoMove GoUctState::LastMove() {
  return m_bd.GetLastMove();
}

const GoBoard &GoUctState::Board() {
  return m_bd;
}

void GoUctState::ExecutePlayout(GoMove move) {
  DBG_ASSERT(m_isInPlayout);
  DBG_ASSERT(move == GO_PASS || !m_uctBd.Occupied(move));
  m_uctBd.Play(move);
  ++m_gameLength;
}

void GoUctState::GameStart() {
  m_isInPlayout = false;
  m_gameLength = 0;
}

void GoUctState::StartPlayout() {
  m_uctBd.Init(m_bd);
}

void GoUctState::StartPlayouts() {
  m_isInPlayout = true;
}

void GoUctState::Clear() {
  UctThreadState::Clear();
  m_isInPlayout = false;
  m_gameLength = 0;
  game_info.Clear();
  // TODO optimize ?  m_bd.Undo()?
  m_bd.Reset();
}

void GoUctState::StartSearch() {
  m_synchronizer.UpdateSubscriber();
}

void GoUctState::TakeBackInTree(std::size_t nuMoves) {
  for (size_t i = 0; i < nuMoves; ++i)
    m_bd.Undo();
}

void GoUctState::TakeBackPlayout(std::size_t nuMoves) {
  m_gameLength -= nuMoves;
}

GoUctSearch::GoUctSearch(GoBoard &bd, UctThreadStateFactory *factory)
    : UctSearch(factory, MOVERANGE),
      m_keepGames(false),
      m_liveGfxInterval(5000),
      m_nextLiveGfx(GOUCT_LIVEGFX_NONE),
      m_toPlay(SG_BLACK),
      m_bd(bd),
      m_root(0),
      m_liveGfx(GOUCT_LIVEGFX_NONE) {
}

GoUctSearch::~GoUctSearch() {
  if (m_root != 0)
    m_root->DeleteTree();
  m_root = 0;
}

std::string GoUctSearch::MoveString(GoMove move) const {
  return GoPointUtil::ToString(move);
}

bool GoUctSearch::NeedLiveGfx(UctValueType gameNumber) {
  if (gameNumber > m_nextLiveGfx) {
    m_nextLiveGfx = gameNumber + m_liveGfxInterval;
    return true;
  }
  return false;
}

void GoUctSearch::OnSearchIteration(UctValueType gameNumber,
                                    std::size_t threadId,
                                    const UctGameInfo &info) {
  UctSearch::OnSearchIteration(gameNumber, threadId, info);

  if (m_liveGfx != GOUCT_LIVEGFX_NONE && threadId == 0
      && NeedLiveGfx(gameNumber)) {
    DisplayGfx();
  }
  if (!LockFree() && m_root != 0)
    AppendGame(m_root, gameNumber, threadId, m_toPlay, info);
}

void GoUctSearch::DisplayGfx() {
  SgDebug() << "gogui-gfx:\n";
  switch (m_liveGfx) {
    case GOUCT_LIVEGFX_COUNTS:GoUctUtil::GfxBestMove(*this, m_toPlay, SgDebug());
      GoUctUtil::GfxMoveValues(*this, m_toPlay, SgDebug());
      GoUctUtil::GfxCounts(Tree(), SgDebug());
      GoUctUtil::GfxStatus(*this, SgDebug());
      break;
    case GOUCT_LIVEGFX_SEQUENCE:GoUctUtil::GfxSequence(*this, m_toPlay, SgDebug());
      GoUctUtil::GfxStatus(*this, SgDebug());
      break;
    case GOUCT_LIVEGFX_NONE:DBG_ASSERT(false);
      break;
  }
  SgDebug() << '\n';
}

void GoUctSearch::OnStartGamePlay() {
  int size = m_bd.Size();
  int maxGameLength = std::min(3 * size * size, GO_MAX_NUM_MOVES);
  SetMaxGameLength((size_t) maxGameLength);
}

void GoUctSearch::OnStartSelfPlay() {
  int size = m_bd.Size();
  int maxGameLength = std::min(3 * size * size, GO_MAX_NUM_MOVES);
  SetMaxGameLength((size_t) maxGameLength);
}

void GoUctSearch::OnStartSearch() {
  UctSearch::OnStartSearch();

  if (m_root != 0) {
    m_root->DeleteTree();
    m_root = 0;
  }
  if (m_keepGames) {
    m_root = GoNodeUtil::CreateRoot(m_bd);
    if (LockFree())
      SgWarning() <<
                  "GoUctSearch: keep games will be ignored"
                      " in lock free search\n";
  }
  m_toPlay = m_bd.ToPlay(); // Not needed if SetToPlay() was called
  for (SgBWIterator it; it; ++it)
    m_stones[*it] = m_bd.All(*it);
  int size = m_bd.Size();
  int maxGameLength = std::min(3 * size * size,
                               GO_MAX_NUM_MOVES - m_bd.MoveNumber());
  SetMaxGameLength((size_t) maxGameLength);
  m_boardHistory.SetFromBoard(m_bd);

  m_nextLiveGfx = m_liveGfxInterval;
}

void GoUctSearch::SaveGames(const std::string &fileName) const {
  if (GetMpiSynchronizer()->IsRootProcess()) {
    if (m_root == 0)
      throw SgException("No games to save");
    std::ofstream out(fileName.c_str());
    SgGameWriter writer(out);
    writer.WriteGame(*m_root, true, 0, 1, 19);
  }
}

void GoUctSearch::SaveTree(std::ostream &out, int maxDepth) const {
  GoUctUtil::SaveTree(Tree(), m_bd.Size(), m_stones, m_toPlay, out,
                      maxDepth);
}

SgBlackWhite GoUctSearch::ToPlay() const {
  return m_toPlay;
}

GoPoint GoUctSearchUtil::TrompTaylorPassCheck(GoPoint move,
                                              const GoUctSearch &search) {
  const GoBoard &bd = search.Board();
  bool lastMoveIsNotPASS = (bd.GetLastMove() != GO_PASS);
  bool isTrompTaylorRules = bd.Rules().CaptureDead();
  if (move != GO_PASS || !isTrompTaylorRules || !lastMoveIsNotPASS)
    return move;
  // current selected move is PASS, and last move is not PASS
  float komi = bd.Rules().Komi().ToFloat();
  float trompTaylorScore = GoBoardUtil::TrompTaylorScore(bd, komi);
  if (search.ToPlay() != SG_BLACK)
    trompTaylorScore *= -1;
  const UctSearchTree &tree = search.Tree();
  const UctNode &root = tree.Root();
  UctValueType value = root.Mean(); // expectation
  // score < 0, to_play will lose the game, trompTaylorWinValue = 0,
  UctValueType trompTaylorWinValue = (trompTaylorScore > 0 ? 1 : 0);
  if (value < trompTaylorWinValue)
    return move;
  SgDebug() << "GoUctSearchUtil::TrompTaylorPassCheck: bad pass move value="
            << value << " trompTaylorScore=" << trompTaylorScore << '\n';
  std::vector<GoMove> excludeMoves;
  excludeMoves.push_back(GO_PASS);
  const UctNode *bestChild = search.FindBestChild(root, &excludeMoves);
  if (bestChild == 0) {
    SgDebug() <<
              "GoUctSearchUtil::TrompTaylorPassCheck: "
                  "(no second best move found)\n";
    return move;
  }
  return bestChild->Move();
}
