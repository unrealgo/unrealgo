
#include <funcapproximator/DlConfig.h>
#include "funcapproximator/DlTFNetworkEvaluator.h"
#include "UctBoardEvaluator.h"

UctBoardEvaluator::UctBoardEvaluator() : m_evaluator("", DF_HWC) {
  m_evaluator.SetNetworkInput(DlConfig::GetInstance().get_network_input());
  std::vector<std::string> outputs;
  DlConfig::GetInstance().get_network_outputs(outputs);
  m_evaluator.SetNetworkOutput(outputs);
}

UctBoardEvaluator::UctBoardEvaluator(const std::string &graphPath, const std::string &checkpoint) : m_evaluator(graphPath) {
  if (!checkpoint.empty())
    m_evaluator.UpdateCheckPoint(checkpoint);
}

void UctBoardEvaluator::LoadGraph(const std::string &graphPath) {
  m_evaluator.LoadGraph(graphPath);
}

void UctBoardEvaluator::UpdateCheckPoint(const std::string &checkpoint) {
  m_evaluator.UpdateCheckPoint(checkpoint);
}

void UctBoardEvaluator::EvaluateState(char feature[][NUM_MAPS][GO_MAX_SIZE][GO_MAX_SIZE],
                                   UctValueType actions_[][GO_MAX_MOVES], UctValueType value_[], int numBatches) {
  m_evaluator.Evaluate(feature, actions_, value_, numBatches);
}

bool UctBoardEvaluator::GraphLoaded() {
  return m_evaluator.MetaGraphLoaded();
}