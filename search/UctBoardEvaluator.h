
#ifndef SG_UCTEVALUATOR_H
#define SG_UCTEVALUATOR_H

#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "platform/SgTimer.h"
#include "UctSearchTree.h"
#include "UctValue.h"
#include "lib/SgRandom.h"

#include "funcapproximator/DlTFNetworkEvaluator.h"

class UctBoardEvaluator {
 public:
  UctBoardEvaluator();
  explicit UctBoardEvaluator(const std::string &graphPath, const std::string &checkpoint);
  void LoadGraph(const std::string &graphPath);
  void UpdateCheckPoint(const std::string &checkpoint);
  void EvaluateState(char feature[][NUM_MAPS][GO_MAX_SIZE][GO_MAX_SIZE],
                     UctValueType actions_[][GO_MAX_MOVES], UctValueType value_[], int numBatches = MAX_BATCHES);
  bool GraphLoaded();

 private:
  tensorflow::DlTFNetworkEvaluator m_evaluator;
};

#endif