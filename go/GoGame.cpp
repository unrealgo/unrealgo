

#include <platform/SgTime.h>
#include "platform/SgSystem.h"
#include "GoGame.h"

#include "GoBoardUtil.h"
#include "GoInit.h"
#include "GoNodeUtil.h"
#include "GoPlayer.h"
#include "SgNode.h"
#include "SgNodeUtil.h"
#include "SgProp.h"
#include "SgSearchStatistics.h"
#include "board/SgUtil.h"

using GoBoardUtil::PlayIfLegal;
using SgUtil::ForceInRange;

namespace {

void AddStatisticsToNode(const SgSearchStatistics* stat, SgNode* node) {
  node->Add(new SgPropInt(SG_PROP_NUM_NODES, stat->NumNodes()));
  node->Add(new SgPropInt(SG_PROP_NUM_LEAFS, stat->NumEvals()));
  node->Add(new SgPropMSec(SG_PROP_TIME_USED, stat->TimeUsed()));
  node->Add(new SgPropInt(SG_PROP_MAX_DEPTH, stat->DepthReached()));
}

void AddHandicap(int size, int row, int col, int* handicap,
                 SgVector<GoPoint>* stones) {
  DBG_ASSERT(2 <= *handicap);
  stones->PushBack(GoPointUtil::Pt(size + 1 - col, row));
  stones->PushBack(GoPointUtil::Pt(col, size + 1 - row));
  if (2 < *handicap)
    stones->PushBack(GoPointUtil::Pt(row, col));
  if (3 < *handicap)
    stones->PushBack(GoPointUtil::Pt(size + 1 - row, size + 1 - col));
  if (*handicap < 4)
    *handicap = 0;
  else
    *handicap -= 4;
}

}

GoGame::GoGame(int boardSize)
    : m_board(boardSize),
      m_root(new SgNode()),
      m_time(),
      m_numMovesToInsert(0) {
  GoInitCheck();
  Init(boardSize, GoRules());
}

GoGame::~GoGame() {
  m_root->DeleteTree();
#ifndef NDEBUG
  m_root = nullptr;
#endif
}

void GoGame::AddComment(const SgNode& node, const std::string& comment) {
  NonConstNodeRef(node).AddComment(comment);
}

void GoGame::AddMove(GoMove move, SgBlackWhite player,
                     const SgSearchStatistics* stat, bool makeMainVariation) {
  SgNode* node = m_current->LeftMostSon();
  while (node) {
    SgPropMove* prop = static_cast<SgPropMove*>(node->Get(SG_PROP_MOVE));
    if (prop && prop->IsValue(move) && prop->IsPlayer(player))
      break;
    node = node->RightBrother();
  }
  if (!node) {
    if (m_current->HasSon() && 0 < m_numMovesToInsert) {
      node = m_current->LeftMostSon()->NewFather();
      --m_numMovesToInsert;
    } else {
      node = m_current->NewRightMostSon();
      m_numMovesToInsert = 0;
    }
    node->AddMoveProp(move, player);
  }
  if (stat)
    AddStatisticsToNode(stat, node);
  m_time.PlayedMove(*node, player);
  if (makeMainVariation)
    node->PromoteNode();
  GoToNode(node);
}

const SgNode& GoGame::AddResignNode(SgBlackWhite player) {
  SgNode& node = *m_current->NewRightMostSon();
  std::ostringstream comment;
  comment << (player == SG_BLACK ? "Black" : "White") << " resigned";
  node.AddComment(comment.str());
  return node;
}

void GoGame::AppendChild(SgNode* child) {
  child->AppendTo(m_current);
}

bool GoGame::CanGoInDirection(SgNode::Direction dir) const {
  SgNode* node = m_current->NodeInDirection(dir);
  return node && node != m_current;
}

GoMove GoGame::CurrentMove() const {
  DBG_ASSERT(m_current);
  return m_current->NodeMove();
}

int GoGame::CurrentMoveNumber() const {
  return SgNodeUtil::GetMoveNumber(m_current);
}

bool GoGame::EndOfGame() const {
  return GoBoardUtil::EndOfGame(m_board);
}

std::string GoGame::GetGameInfoStringProp(SgPropID id) const {
  const SgNode* node = m_current->TopProp(id);
  if (node->HasProp(id)) {
    const SgPropText* prop = dynamic_cast<SgPropText*>(node->Get(id));
    return prop->Value();
  } else
    return "";
}

std::string GoGame::GetGameName() const {
  return GetGameInfoStringProp(SG_PROP_GAME_NAME);
}

std::string GoGame::GetPlayerName(SgBlackWhite player) const {
  SgPropID id = SgProp::PlayerProp(SG_PROP_PLAYER_BLACK, player);
  return GetGameInfoStringProp(id);
}

std::string GoGame::GetResult() const {
  return GetGameInfoStringProp(SG_PROP_RESULT);
}

void GoGame::GoInDirection(SgNode::Direction dir) {
  SgNode* node = m_current->NodeInDirection(dir);
  if (node != m_current)
    GoToNode(node);
}

void GoGame::GoToNode(const SgNode* dest) {
  m_updater.Update(dest, m_board);
  SgNodeUtil::UpdateTime(TimeRecord(), dest);
  m_current = NonConstNodePtr(dest);
  if (GoBoardUtil::RemainingChineseHandicap(m_board))
    m_board.SetToPlay(SG_BLACK);
  m_time.EnterNode(*m_current, m_board.ToPlay());
}

void GoGame::Init(int size, const GoRules& rules) {
  m_board.Init(size, rules);
  m_root->DeleteTree();
  m_root = new SgNode();
  SgPropInt* boardSizeProp = new SgPropInt(SG_PROP_SIZE, size);
  m_root->Add(boardSizeProp);
  GoKomi komi = rules.Komi();
  if (!komi.IsUnknown())
    m_root->SetRealProp(SG_PROP_KOMI, komi.ToFloat(), 1);
  InitHandicap(rules, m_root);
  GoToNode(m_root);
}

void GoGame::Reset() {
  int size = Board().Size();
  GoRules rules = Board().Rules();
  Init(size, rules);
  UpdateDate(SgTime::TodaysDate());
  SetTimeSettingsGlobal(m_timeSettings, m_time.Overhead());
}

void GoGame::Reset(GoRules& rules) {
  Init(Board().Size(), rules);
  UpdateDate(SgTime::TodaysDate());
  SetTimeSettingsGlobal(m_timeSettings, m_time.Overhead());
}

void GoGame::Init(SgNode* root) {
  m_root->DeleteTree();
  m_root = root;
  int size = GoNodeUtil::GetBoardSize(m_root);
  ForceInRange(GO_MIN_SIZE, &size, GO_MAX_SIZE);
  const GoRules& rules = m_board.Rules();
  m_board.Init(size, GoRules(rules.Handicap(), rules.Komi()));
  const int GAME_ID = 1;
  SgPropInt* gameId = new SgPropInt(SG_PROP_GAME, GAME_ID);
  m_root->Add(gameId);
  GoToNode(m_root);
}

SgNode* GoGame::NonConstNodePtr(const SgNode* node) const {
  DBG_ASSERT(node->Root() == m_root);
  return const_cast<SgNode*>(node);
}

SgNode& GoGame::NonConstNodeRef(const SgNode& node) const {
  DBG_ASSERT(node.Root() == m_root);
  return const_cast<SgNode&>(node);
}

void GoGame::PlaceHandicap(const SgVector<GoPoint>& stones) {
  DBG_ASSERT(GoBoardUtil::IsBoardEmpty(m_board));
  SgNode* node = m_current;
  if (node->HasSon())
    node = node->NewRightMostSon();
  SgPropAddStone* addBlack = new SgPropAddStone(SG_PROP_ADD_BLACK);
  for (SgVectorIterator<GoPoint> it(stones); it; ++it)
    addBlack->PushBack(*it);
  node->Add(addBlack);
  SgPropInt* handicap = new SgPropInt(SG_PROP_HANDICAP, stones.Length());
  node->Add(handicap);
  node->Add(new SgPropPlayer(SG_PROP_PLAYER, SG_WHITE));
  m_board.Rules().SetHandicap(stones.Length());
  GoToNode(node);
}

void GoGame::InitHandicap(const GoRules& rules, SgNode* root) {
  if (2 <= rules.Handicap()) {
    SgPropInt* handicap =
        new SgPropInt(SG_PROP_HANDICAP, rules.Handicap());
    root->Add(handicap);
    if (rules.JapaneseHandicap()) {
      if (9 <= m_board.Size()) {
        int h = rules.Handicap();
        int half = (m_board.Size() + 1) / 2;
        SgVector<GoPoint> stones;
        if ((4 < h) && (h % 2 != 0)) {
          stones.PushBack(GoPointUtil::Pt(half, half));
          --h;
        }
        if (13 <= m_board.Size()) {
          AddHandicap(m_board.Size(), 4, 4, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), half, 4, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), 3, 3, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), 7, 7, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), half, 3, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), half - (half - 4) / 2,
                        4, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), half + (half - 4) / 2,
                        4, &h, &stones);
        } else {
          AddHandicap(m_board.Size(), 3, 3, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), half, 3, &h, &stones);
          if (0 < h)
            AddHandicap(m_board.Size(), 4, 4, &h, &stones);
        }
        SgPropAddStone* addBlack =
            new SgPropAddStone(SG_PROP_ADD_BLACK, stones);
        root->Add(addBlack);
        SgPropPlayer* player =
            new SgPropPlayer(SG_PROP_PLAYER, SG_WHITE);
        root->Add(player);
      }
    } else {
      SgPropInt* chinese =
          new SgPropInt(SG_PROP_CHINESE, rules.Handicap());
      root->Add(chinese);
    }
  }
}

void GoGame::SetKomiGlobal(GoKomi komi) {
  SgNodeUtil::RemovePropInSubtree(*m_root, SG_PROP_KOMI);
  if (!komi.IsUnknown())
    m_root->SetRealProp(SG_PROP_KOMI, komi.ToFloat(), 1);
  m_board.Rules().SetKomi(komi);
}

void GoGame::SetRulesGlobal(const GoRules& rules) {
  m_board.Rules() = rules;
  SetKomiGlobal(rules.Komi());
}

void GoGame::SetTimeSettingsGlobal(const SgTimeSettings& timeSettings,
                                   double overhead) {
  m_timeSettings = timeSettings;
  SgNodeUtil::RemovePropInSubtree(*m_root, SG_PROP_TIME);
  SgNodeUtil::RemovePropInSubtree(*m_root, SG_PROP_OT_NU_MOVES);
  SgNodeUtil::RemovePropInSubtree(*m_root, SG_PROP_OT_PERIOD);
  m_time.SetOverhead(overhead);
  if (timeSettings.IsUnknown()) {
    return;
  }
  double mainTime = timeSettings.MainTime();
  m_root->Add(new SgPropTime(SG_PROP_TIME, mainTime));
  double overtime = timeSettings.Overtime();
  if (overtime > 0) {
    m_root->Add(new SgPropTime(SG_PROP_OT_PERIOD, overtime));
    m_root->SetIntProp(SG_PROP_OT_NU_MOVES, timeSettings.OvertimeMoves());
  }
  m_time.SetOTPeriod(overtime);
  m_time.SetOTNumMoves(timeSettings.OvertimeMoves());
  m_time.SetClock(*m_current, SG_BLACK, mainTime);
  m_time.SetClock(*m_current, SG_WHITE, mainTime);
  m_time.TurnClockOn(true);
}

void GoGame::SetToPlay(SgBlackWhite toPlay) {
  if (toPlay == m_board.ToPlay())
    return;
  m_board.SetToPlay(toPlay);
  m_current->Add(new SgPropPlayer(SG_PROP_PLAYER, toPlay));
  m_time.EnterNode(*m_current, toPlay);
}

void GoGame::SetupPosition(const SgBWArray<GoPointSet>& stones) {
  SgPropAddStone* addBlack = new SgPropAddStone(SG_PROP_ADD_BLACK);
  SgPropAddStone* addWhite = new SgPropAddStone(SG_PROP_ADD_WHITE);
  for (SgSetIterator it(stones[SG_BLACK]); it; ++it)
    if (m_board.GetColor(*it) != SG_BLACK)
      addBlack->PushBack(*it);
  for (SgSetIterator it(stones[SG_WHITE]); it; ++it)
    if (m_board.GetColor(*it) != SG_WHITE)
      addWhite->PushBack(*it);
  SgNode* node = m_current->NewRightMostSon();
  node->Add(addBlack);
  node->Add(addWhite);
  GoToNode(node);
}

void GoGame::UpdateDate(const std::string& date) {
  UpdateGameInfoStringProp(SG_PROP_DATE, date);
}

void GoGame::UpdateGameInfoStringProp(SgPropID id, const std::string& value) {
  SgNode* node = m_current->TopProp(id);
  node->SetStringProp(id, value);
}

void GoGame::UpdateGameName(const std::string& name) {
  UpdateGameInfoStringProp(SG_PROP_GAME_NAME, name);
}

void GoGame::UpdatePlayerName(SgBlackWhite color, const std::string& name) {
  SgPropID id = SgProp::PlayerProp(SG_PROP_PLAYER_BLACK, color);
  UpdateGameInfoStringProp(id, name);
}

void GoGame::UpdateResult(const std::string& result) {
  UpdateGameInfoStringProp(SG_PROP_RESULT, result);
}

bool GoGameUtil::GotoBeforeMove(GoGame* game, int moveNumber) {
  DBG_ASSERT(game->CurrentNode() == &game->Root());
  DBG_ASSERT(moveNumber == -1 || moveNumber > 0);
  if (moveNumber > 0) {
    while (game->CanGoInDirection(SgNode::NEXT)
        && !game->CurrentNode()->HasProp(SG_PROP_MOVE)
        && !game->CurrentNode()->LeftMostSon()->HasProp(SG_PROP_MOVE))
      game->GoInDirection(SgNode::NEXT);
    while (game->CurrentMoveNumber() < moveNumber - 1
        && game->CanGoInDirection(SgNode::NEXT))
      game->GoInDirection(SgNode::NEXT);
    if (game->CurrentMoveNumber() != moveNumber - 1)
      return false;
  } else {
    while (game->CanGoInDirection(SgNode::NEXT))
      game->GoInDirection(SgNode::NEXT);
  }
  return true;
}
