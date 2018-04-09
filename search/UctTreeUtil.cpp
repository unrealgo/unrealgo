
#include "platform/SgSystem.h"
#include "UctTreeUtil.h"

#include <iomanip>
#include "UctSearch.h"
#include "board/SgWrite.h"

using namespace std;

UctTreeStatistics::UctTreeStatistics() {
  Clear();
}

void UctTreeStatistics::Clear() {
  m_nuNodes = 0;
  memset(m_moveCounts, 0, sizeof(m_moveCounts));
  m_biasRave.Clear();
}

void UctTreeStatistics::Compute(const UctSearchTree &tree) {
  Clear();
  for (UctTreeIterator it(tree); it; ++it) {
    const UctNode &node = *it;
    ++m_nuNodes;
    UctValueType count = node.MoveCount();
    if (count < UctValueType(UctTreeStatistics::MAX_MOVECOUNT))
      ++m_moveCounts[static_cast<size_t>(count)];
    if (!node.HasChildren())
      continue;
    for (UctChildNodeIterator childIt(tree, node); childIt; ++childIt) {
      const UctNode &child = *childIt;
      if (child.HasRaveValue() && child.HasMean()) {
        UctValueType childValue =
            UctSearch::InverseEstimate(child.Mean());
        UctValueType biasRave = child.RaveValue() - childValue;
        m_biasRave.Add(biasRave);
      }
    }
  }
}

void UctTreeStatistics::Write(ostream &out) const {
  out << SgWriteLabel("NuNodes") << m_nuNodes << '\n';
  for (size_t i = 0; i < MAX_MOVECOUNT; ++i) {
    ostringstream label;
    label << "MoveCount[" << i << ']';
    size_t percent = m_moveCounts[i] * 100 / m_nuNodes;
    out << SgWriteLabel(label.str()) << setw(2) << right << percent
        << "%\n";
  }
  out << SgWriteLabel("BiasRave");
  m_biasRave.Write(out);
  out << '\n';
}

std::ostream &operator<<(ostream &out, const UctTreeStatistics &stat) {
  stat.Write(out);
  return out;
}

const UctNode *
UctTreeUtil::FindMatchingNode(const UctSearchTree &tree,
                                const std::vector<GoMove> &sequence) {
  const UctNode *node = &tree.Root();
  for (auto it = sequence.begin(); it != sequence.end(); ++it) {
    const GoMove mv = *it;
    node = UctTreeUtil::FindChildWithMove(tree, *node, mv);
    if (node == 0)
      return 0;
  }
  return node;
}

void UctTreeUtil::ExtractSubtree(const UctSearchTree &tree, UctSearchTree &target,
                                   const std::vector<GoMove> &sequence,
                                   bool warnTruncate, double maxTime,
                                   UctValueType minCount) {
  target.Clear();
  const UctNode *node = FindMatchingNode(tree, sequence);
  if (node)
    tree.ExtractSubtree(target, *node, warnTruncate, maxTime, minCount);
}

const UctNode *UctTreeUtil::FindChildWithMove(const UctSearchTree &tree,
                                                  const UctNode &node,
                                                  GoMove move) {
  if (!node.HasChildren())
    return nullptr;
  for (UctChildNodeIterator it(tree, node); it; ++it) {
    const UctNode &child = *it;
    if (child.Move() == move)
      return &child;
  }
  return nullptr;
}

bool UctTreeUtil::CheckTreeConsistency(const UctSearchTree &tree, const UctNode &parent) {
  if (parent.MoveCount() != parent.VisitCount())
    return false;
  if (!parent.HasChildren())
    return true;

  for (UctChildNodeIterator it(tree, parent); it; ++it) {
    const UctNode &child = *it;
    if (child.Parent() != &parent || !CheckTreeConsistency(tree, child))
      return false;
  }
  return true;
}

void UctTreeUtil::MoveNode(const UctSearchTree &tree, UctNode &srcNode, UctNode &dstNode) {
  DBG_ASSERT(!dstNode.HasChildren());
  dstNode.CopyDataFrom(srcNode, true);
  size_t childNuChildren = srcNode.NumChildren();
  dstNode.SetNumChildren(childNuChildren);
  if (childNuChildren > 0)
    dstNode.SetFirstChild(srcNode.FirstChild());

  if (srcNode.HasChildren()) {
    for (UctChildNodeIterator it(tree, srcNode); it; ++it) {
      auto &child = const_cast<UctNode &>(*it);
      child.SetParent(&dstNode);
    }
  }
}