

#ifndef GO_GAME_H
#define GO_GAME_H

#include <string>
#include "GoBoard.h"
#include "GoBoardUpdater.h"
#include "GoBoardUtil.h"
#include "SgTimeSettings.h"
#include "SgNode.h"
#include "board/GoPoint.h"
#include "SgTimeRecord.h"

class SgSearchStatistics;
class GoGame {
 public:

  explicit GoGame(int boardSize = GO_DEFAULT_SIZE);
  ~GoGame();
  void Init(SgNode* root);
  void Init(int size, const GoRules& rules);
  void Reset();
  void Reset(GoRules& rules);
  const GoBoard& Board() const;
  const SgNode& Root() const;
  const SgTimeSettings& TimeSettings() const;
  void PlaceHandicap(const SgVector<GoPoint>& stones);
  void SetupPosition(const SgBWArray<GoPointSet>& stones);
  void AddMove(GoMove move, SgBlackWhite player,
               const SgSearchStatistics* stat = 0,
               bool makeMainVariation = true);
  void AddComment(const std::string& comment);
  void AddComment(const SgNode& node, const std::string& comment);
  const SgNode& AddResignNode(SgBlackWhite player);
  void AppendChild(SgNode* child);
  void GoToNode(const SgNode* dest);
  void GoInDirection(SgNode::Direction dir);
  bool CanGoInDirection(SgNode::Direction dir) const;
  void SetToPlay(SgBlackWhite toPlay);
  bool EndOfGame() const;
  SgTimeRecord& TimeRecord();
  const SgTimeRecord& Time() const;
  const SgNode* CurrentNode() const;
  GoMove CurrentMove() const;
  int CurrentMoveNumber() const;
  void SetKomiGlobal(GoKomi komi);
  void SetTimeSettingsGlobal(const SgTimeSettings& timeSettings,
                             double overhead = 0);
  std::string GetPlayerName(SgBlackWhite player) const;
  void UpdatePlayerName(SgBlackWhite player, const std::string& name);
  void UpdateDate(const std::string& date);
  std::string GetResult() const;
  void UpdateResult(const std::string& result);
  std::string GetGameName() const;
  void UpdateGameName(const std::string& name);
  void SetRulesGlobal(const GoRules& rules);

 private:
  GoBoard m_board;
  SgNode* m_root;
  SgNode* m_current;
  GoBoardUpdater m_updater;
  SgTimeSettings m_timeSettings;
  SgTimeRecord m_time;
  int m_numMovesToInsert;
  GoGame(const GoGame&);
  GoGame& operator=(const GoGame&);
  std::string GetGameInfoStringProp(SgPropID id) const;
  void InitHandicap(const GoRules& rules, SgNode* root);
  SgNode* NonConstNodePtr(const SgNode* node) const;
  SgNode& NonConstNodeRef(const SgNode& node) const;
  void UpdateGameInfoStringProp(SgPropID id, const std::string& value);
};

inline void GoGame::AddComment(const std::string& comment) {
  m_current->AddComment(comment);
}

inline const GoBoard& GoGame::Board() const {
  return m_board;
}

inline const SgNode& GoGame::Root() const {
  return *m_root;
}

inline SgTimeRecord& GoGame::TimeRecord() {
  return m_time;
}

inline const SgTimeRecord& GoGame::Time() const {
  return m_time;
}

inline const SgTimeSettings& GoGame::TimeSettings() const {
  return m_timeSettings;
}

inline const SgNode* GoGame::CurrentNode() const {
  return m_current;
}

namespace GoGameUtil {

bool GotoBeforeMove(GoGame* game, int moveNumber);
}

#endif

