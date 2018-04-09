

#ifndef GO_BOARDSYNCHRONIZER_H
#define GO_BOARDSYNCHRONIZER_H

#include "GoPlayerMove.h"

class GoBoard;
class GoRules;
class GoBoardSynchronizer {
 public:

  explicit GoBoardSynchronizer(const GoBoard& publisher);
  virtual ~GoBoardSynchronizer();
  void SetSubscriber(GoBoard& subscriber);
  void UpdateSubscriber();
  const GoRules& Rules();

 protected:

  virtual void OnBoardChange();
  virtual void PrePlay(const GoPlayerMove& move);
  virtual void OnPlay(const GoPlayerMove& move);
  virtual void PreUndo();
  virtual void OnUndo();

 private:
  const GoBoard& m_publisher;
  GoBoard* m_subscriber;
  const GoRules& m_Rule;
  GoBoardSynchronizer(const GoBoardSynchronizer&);
  GoBoardSynchronizer& operator=(const GoBoardSynchronizer&);
  void ExecuteSubscriber(const GoPlayerMove& move);
  int FindNuCommon() const;
  void UpdateFromInit();
  void UpdateIncremental();
  void UpdateWhoToPlay();
};

#endif

