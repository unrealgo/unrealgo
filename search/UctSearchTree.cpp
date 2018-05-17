
#include "platform/SgSystem.h"
#include "UctSearchTree.h"

#include <boost/format.hpp>
#include "platform/SgDebug.h"
#include "platform/SgTimer.h"
#include "../go/GoBoard.h"
#include "UctTreeUtil.h"

using boost::format;
using boost::shared_ptr;

UctNodeAllocator::~UctNodeAllocator() {
  if (node_start != nullptr) {
    Clear();
    std::free(node_start);
  }
}

bool UctNodeAllocator::Contains(const UctNode &node) const {
  return (&node >= node_start && &node < node_end);
}
void UctNodeAllocator::Swap(UctNodeAllocator &allocator) {
  std::swap(node_start, allocator.node_start);
  std::swap(node_end, allocator.node_end);
  std::swap(block_end, allocator.block_end);
  std::swap(hole_start, allocator.hole_start);
  std::swap(hole_end, allocator.hole_end);
  std::swap(max_accessed, allocator.max_accessed);
}

void UctNodeAllocator::SetMaxNodes(std::size_t maxNodes) {
  if (node_start != 0) {
    Clear();
    std::free(node_start);
  }
  void *ptr = std::malloc(maxNodes * sizeof(UctNode));
  if (ptr == 0)
    throw std::bad_alloc();
  node_start = static_cast<UctNode *>(ptr);
  node_end = node_start;
  max_accessed = node_start;
  hole_start = node_start;
  hole_end = node_start;
  block_end = node_start + maxNodes;
}

std::ostream &operator<<(std::ostream &stream, const UctMoveInfo &info) {
  stream << "move = " << GoWritePoint(info.uct_move)
         << "value = " << info.uct_value
         << "count = " << info.visit_count;
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const UctNode &node) {
  if (node.HasMove())
    stream << GoWritePoint(node.Move());
  else
    stream << "Root (no move)";
  stream << " mean = ";
  if (node.HasMean())
    stream << node.Mean();
  else
    stream << "undefined";
  stream << " pos-count = " << node.PosCount()
         << " move-count = " << node.MoveCount();
  stream << " Virtual-Loss-Count = " << node.VirtualLossCount()
         << ' ' << node.ProvenType()
         << '\n';
  return stream;
}


std::ostream &operator<<(std::ostream &stream, const UctProvenType &type) {
  static const char *s_string[3] =
      {
          "proven none",
          "proven win",
          "proven loss"
      };
  DBG_ASSERT(type >= PROVEN_NONE);
  DBG_ASSERT(type <= PROVEN_LOSS);
  stream << s_string[type];
  return stream;
}

UctSearchTree:: UctSearchTree()
    : max_nodes_allowed(0),
      tree_root(GO_NULLMOVE) {}

void UctSearchTree::ApplyFilter(const UctSearchTree &tree, std::size_t allocatorId,
                            const UctNode &parent,
                            const std::vector<GoMove> &rootFilter) {
  if (rootFilter.empty())
    return;

  DBG_ASSERT(Contains(parent));
  DBG_ASSERT(Allocator(allocatorId).HasCapacity(parent.NumChildren()));
  if (!parent.HasChildren())
    return;

  UctNodeAllocator &allocator = Allocator(allocatorId);
  const UctNode *firstChild = allocator.Finish();
  size_t newNuChildren = 0;
  for (UctChildNodeIterator it(*this, parent); it; ++it) {
    GoMove move = (*it).Move();
    if (find(rootFilter.begin(), rootFilter.end(), move) == rootFilter.end()) {
      UctNode *child = allocator.CreateOneAtEnd(move, &parent);
      UctTreeUtil::MoveNode(tree, const_cast<UctNode &>(*it), *child);
      ++newNuChildren;
    }
  }

  auto &nonConstNode = const_cast<UctNode &>(parent);
  SgSynchronizeThreadMemory();
  nonConstNode.SetFirstChild(firstChild);
  SgSynchronizeThreadMemory();
  nonConstNode.SetNumChildren(newNuChildren);
}

void UctSearchTree::CheckConsistency() const {
  for (UctTreeIterator it(*this); it; ++it)
    if (!Contains(*it))
      ThrowConsistencyError(str(format("! Contains(%1%)") % &(*it)));
}

size_t UctSearchTree::GetMaxMemoryUsed(size_t allocatorID) {
  return Allocator(allocatorID).GetMaxMemoryUsed();
}

void UctSearchTree::Clear() {
  for (size_t i = 0; i < NuAllocators(); ++i) {
    Allocator(i).Clear();
  }
  tree_root = UctNode(GO_NULLMOVE);
}

bool UctSearchTree::Contains(const UctNode &node) const {
  if (&node == &tree_root)
    return true;
  for (size_t i = 0; i < NuAllocators(); ++i)
    if (Allocator(i).Contains(node))
      return true;
  return false;
}

void UctSearchTree::CopyPruneLowCount(UctSearchTree &target, UctValueType minCount,
                                  bool warnTruncate, double maxTime) const {
  size_t allocatorId = 0;
  SgTimer timer;
  bool abort = false;
  CopySubtree(target, target.tree_root, tree_root, minCount, allocatorId,
              warnTruncate, abort, timer, maxTime, false);
  SgSynchronizeThreadMemory();
}


UctProvenType UctSearchTree::CopySubtree(UctSearchTree &targetTree, UctNode &targetNode,
                                       const UctNode &srcNode, UctValueType minCount,
                                       std::size_t &currentAllocatorId,
                                       bool warnTruncate, bool &abort, SgTimer &timer,
                                       double maxTime, bool alwaysKeepProven) const {
  DBG_ASSERT(Contains(srcNode));
  DBG_ASSERT(targetTree.Contains(targetNode));
  targetNode.CopyNonPointerData(srcNode);

  if (!srcNode.HasChildren())
    return (UctProvenType) srcNode.ProvenType();

  if (srcNode.MoveCount() < minCount
      && (!srcNode.IsProven() || !alwaysKeepProven)
      ) {
    targetNode.SetProvenType(PROVEN_NONE);
    return PROVEN_NONE;
  }

  UctNodeAllocator &targetAllocator = targetTree.Allocator(currentAllocatorId);
  size_t nuChildren = srcNode.NumChildren();
  if (!abort) {
    if (!targetAllocator.HasCapacity((size_t) nuChildren)) {
      if (warnTruncate)
        SgDebug() <<
                  "UctSearchTree::CopySubtree: Truncated (allocator capacity)\n";
      abort = true;
    }
    if (timer.IsTimeOut(maxTime, 10000)) {
      if (warnTruncate)
        SgDebug() << "UctSearchTree::CopySubtree: Truncated (max time)\n";
      abort = true;
    }
    if (ForceAbort()) {
      if (warnTruncate)
        SgDebug() << "UctSearchTree::CopySubtree: Truncated (aborted)\n";
      abort = true;
    }
  }
  if (abort) {
    targetNode.SetProvenType(PROVEN_NONE);
    return PROVEN_NONE;
  }
  targetNode.SetNumChildren(nuChildren);
  UctNode *firstTargetChild = targetAllocator.CreateN(nuChildren, &targetNode);
  targetNode.SetFirstChild(firstTargetChild);
  UctProvenType childProvenType;
  UctProvenType parentProvenType = PROVEN_LOSS;
  UctNode *targetChild = firstTargetChild;
  for (UctChildNodeIterator it(*this, srcNode); it; ++it, ++targetChild) {
    const UctNode &srcChild = *it;
    ++currentAllocatorId;
    if (currentAllocatorId >= targetTree.NuAllocators())
      currentAllocatorId = 0;
    childProvenType = CopySubtree(targetTree, *targetChild, srcChild,
                                  minCount, currentAllocatorId,
                                  warnTruncate, abort, timer,
                                  maxTime, alwaysKeepProven);
    if (childProvenType == PROVEN_LOSS)
      parentProvenType = PROVEN_WIN;
    else if (parentProvenType != PROVEN_WIN
        && childProvenType == PROVEN_NONE)
      parentProvenType = PROVEN_NONE;
  }
  targetNode.SetProvenType(parentProvenType);
  return parentProvenType;
}

void UctSearchTree::Prune(std::size_t allocatorId, const UctNode &parent, UctNode *toKeep) {
  DBG_ASSERT(Contains(parent));
  DBG_ASSERT(Contains(*toKeep));
  auto &nonConstParent = const_cast<UctNode &>(parent);
  UctNodeAllocator &allocator = Allocator(allocatorId);
  auto *firstChild = const_cast<UctNode *>(parent.FirstChild());
  allocator.RangeRemoveExcept(firstChild, parent.NumChildren(), toKeep);
  nonConstParent.SetNumChildren(1);

  if (toKeep != firstChild) {
    for (UctChildNodeIterator it(*this, *firstChild); it; ++it) {
      const_cast<UctNode &>(*it).SetParent(firstChild);
    }
  }
}

void UctSearchTree::CreateAllocators(std::size_t num_ths) {
  Clear();
  node_allocators.clear();
  for (size_t i = 0; i < num_ths; ++i) {
    boost::shared_ptr<UctNodeAllocator> allocator(new UctNodeAllocator());
    node_allocators.push_back(allocator);
  }
}

const UctNode *UctSearchTree::CreateChildIfNotExist(std::size_t allocatorId, const UctNode *parent, GoMove move) {
  const UctNode *target = 0;
  if (parent->HasChildren()) {
    for (UctChildNodeIterator it(*this, *parent); it; ++it) {
      const UctNode &child = *it;
      if (child.Move() == move) {
        target = it();
        break;
      }
    }
  }

  if (!target)
  {
    size_t newNumChildRen = parent->NumChildren() + 1;
    UctNodeAllocator &allocator = Allocator(allocatorId);
    DBG_ASSERT(allocator.HasCapacity(newNumChildRen));
    if (!parent->HasChildren()) {
      target = allocator.CreateOne(move, parent);
      const_cast<UctNode *>(parent)->SetFirstChild(target);
    } else {
      UctNode *newChildren = allocator.CreateN(newNumChildRen, parent);
      int i = 0;
      for (UctChildNodeIterator it(*this, *parent); it; ++it) {
        const UctNode &child = *it;
        newChildren[i].CopyDataFrom(child);
        ++i;
      }
      newChildren[newNumChildRen - 1].SetMove(move);
      const_cast<UctNode *>(parent)->SetFirstChild(newChildren);
    }
    const_cast<UctNode *>(parent)->SetNumChildren(newNumChildRen);
  }

  return target;
}

void UctSearchTree::DumpDebugInfo(std::ostream &out) const {
  out << "Root " << &tree_root << '\n';
  for (size_t i = 0; i < NuAllocators(); ++i)
    out << "Allocator " << i
        << " size=" << Allocator(i).NuNodes()
        << " start=" << Allocator(i).Start()
        << " finish=" << Allocator(i).Finish() << '\n';
}
void UctSearchTree::ExtractSubtree(UctSearchTree &target, const UctNode &node,
                               bool warnTruncate, double maxTime,
                               UctValueType minCount) const {
  DBG_ASSERT(Contains(node));
  DBG_ASSERT(&target != this);
  DBG_ASSERT(target.MaxNodes() == MaxNodes());
  target.Clear();
  size_t allocatorId = 0;
  SgTimer timer;
  bool abort = false;
  CopySubtree(target, target.tree_root, node, minCount, allocatorId, warnTruncate,
              abort, timer, maxTime,  true);
  SgSynchronizeThreadMemory();
}

std::size_t UctSearchTree::NuNodes() const {
  size_t nuNodes = 1;
  for (size_t i = 0; i < NuAllocators(); ++i)
    nuNodes += Allocator(i).NuNodes();
  return nuNodes;
}

void UctSearchTree::SetMaxNodes(std::size_t maxNodes) {
  Clear();
  size_t nuAllocators = NuAllocators();
  if (nuAllocators == 0) {
    SgDebug() << "UctSearchTree::SetMaxNodes: no allocators registered\n";
    DBG_ASSERT(false);
    return;
  }
  max_nodes_allowed = maxNodes;
  size_t maxNodesPerAlloc = maxNodes / nuAllocators;
  for (size_t i = 0; i < NuAllocators(); ++i)
    Allocator(i).SetMaxNodes(maxNodesPerAlloc);
}
void UctSearchTree::Swap(UctSearchTree &tree) {
  DBG_ASSERT(MaxNodes() == tree.MaxNodes());
  DBG_ASSERT(NuAllocators() == tree.NuAllocators());
  if (tree_root.HasChildren()) {
    for (UctChildNodeIterator it(*this, tree_root); it; ++it) {
      auto &child = const_cast<UctNode &>(*it);
      child.SetParent(&tree.tree_root);
    }
  }
  if (tree.Root().HasChildren()) {
    for (UctChildNodeIterator it(tree, tree.Root()); it; ++it) {
      auto &child = const_cast<UctNode &>(*it);
      child.SetParent(&tree_root);
    }
  }
  std::swap(tree_root, tree.tree_root);
  for (size_t i = 0; i < NuAllocators(); ++i)
    Allocator(i).Swap(tree.Allocator(i));
}

void UctSearchTree::ThrowConsistencyError(const std::string &message) const {
  DumpDebugInfo(SgDebug());
  throw SgException("UctSearchTree::ThrowConsistencyError: " + message);
}


UctTreeIterator::UctTreeIterator(const UctSearchTree &tree)
    : m_tree(tree),
      m_current(&tree.Root()) {}

const UctNode &UctTreeIterator::operator*() const {
  return *m_current;
}

void UctTreeIterator::operator++() {
  if (m_current->HasChildren()) {
    auto *it = new UctChildNodeIterator(m_tree, *m_current);
    m_stack.push(shared_ptr<UctChildNodeIterator>(it));
    m_current = &(**it);
    return;
  }
  while (!m_stack.empty()) {
    UctChildNodeIterator &it = *m_stack.top();
    DBG_ASSERT(it);
    ++it;
    if (it) {
      m_current = &(*it);
      return;
    } else {
      m_stack.pop();
      m_current = nullptr;
    }
  }
  m_current = nullptr;
}

UctTreeIterator::operator bool() const {
  return (m_current != nullptr);
}