
#ifndef GO_BOARDUPDATER_H
#define GO_BOARDUPDATER_H

#include <vector>

class GoBoard;
class SgNode;
class GoBoardUpdater {
 public:
  void Update(const SgNode* node, GoBoard& bd);

 private:

  std::vector<const SgNode*> m_nodes;
};

#endif
