
#ifndef SG_UCTTREEUTIL_H
#define SG_UCTTREEUTIL_H

#include <cstddef>
#include <iosfwd>
#include "UctValue.h"
#include "SgStatistics.h"

class UctNode;
class UctSearchTree;

class UctTreeStatistics {
 public:
  static const std::size_t MAX_MOVECOUNT = 5;
  std::size_t m_nuNodes;
  std::size_t m_moveCounts[MAX_MOVECOUNT];
  SgStatisticsExt<UctValueType, std::size_t> m_biasRave;
  UctTreeStatistics();
  void Clear();
  void Compute(const UctSearchTree &tree);
  void Write(std::ostream &out) const;
};

std::ostream &operator<<(std::ostream &out, const UctTreeStatistics &stat);


namespace UctTreeUtil {
void ExtractSubtree(const UctSearchTree &tree, UctSearchTree &target,
                    const std::vector<GoMove> &sequence,
                    bool warnTruncate,
                    double maxTime = std::numeric_limits<double>::max(),
                    UctValueType minCount = 0);
const UctNode *FindChildWithMove(const UctSearchTree &tree,
                                   const UctNode &node, GoMove move);
const UctNode *FindMatchingNode(const UctSearchTree &tree,
                                  const std::vector<GoMove> &sequence);
bool CheckTreeConsistency(const UctSearchTree &tree, const UctNode &parent);
void MoveNode(const UctSearchTree &tree, UctNode &srcNode, UctNode &dstNode);
}

#endif
