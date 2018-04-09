
#ifndef SG_UCTTREE_H
#define SG_UCTTREE_H

#include <iostream>
#include <limits>
#include <stack>
#include <boost/shared_ptr.hpp>
#include <board/GoPoint.h>
#include "board/GoPoint.h"
#include "SgStatistics.h"
#include "SgStatisticsVlt.h"
#include "UctValue.h"
#include "SgScore.h"
#include "go/GoBoard.h"

class SgTimer;
struct UctMoveInfo {
  GoMove uct_move;
  UctValueType uct_value;
  UctValueType uct_w;
  UctValueType uct_Q;
  UctValueType uct_prior;
  UctValueType visit_count;
  UctValueType rave_value;
  UctValueType rave_cnt;
  float predictor_val;

  UctMoveInfo();
  explicit UctMoveInfo(GoMove move);

  UctMoveInfo(GoMove move, UctValueType value, UctValueType count,
                UctValueType raveValue, UctValueType raveCount);
  void Add(const UctValueType mean, const UctValueType count);
  void AddValue(const UctValueType value, const UctValueType count);
  void SetActionProb(const UctValueType prob);

};
std::ostream &operator<<(std::ostream &stream, const UctMoveInfo &info);

inline UctMoveInfo::UctMoveInfo()
    : uct_move(GO_NULLMOVE),
      uct_value(0),
      uct_w(0),
      uct_Q(0),
      uct_prior(0),
      visit_count(0),
      rave_value(0),
      rave_cnt(0),
      predictor_val(0.0) {}

inline UctMoveInfo::UctMoveInfo(GoMove move)
    : uct_move(move),
      uct_value(0),
      uct_w(0),
      uct_Q(0),
      uct_prior(0),
      visit_count(0),
      rave_value(0),
      rave_cnt(0),
      predictor_val(0.0) {}

inline UctMoveInfo::UctMoveInfo(GoMove move, UctValueType value, UctValueType count,
                                    UctValueType raveValue, UctValueType raveCount)
    : uct_move(move),
      uct_value(value),
      uct_w(0),
      uct_Q(0),
      uct_prior(0),
      visit_count(count),
      rave_value(raveValue),
      rave_cnt(raveCount),
      predictor_val(0.0) {}

inline void UctMoveInfo::Add(const UctValueType mean, const UctValueType count) {
  visit_count += count;
  UctValueType v1 = UctValueUtil::InverseValue(uct_value);
  UctValueType v2 = mean;
  v2 -= v1;
  v1 += (v2 * count) / visit_count;
  uct_value = UctValueUtil::InverseValue(v1);

  rave_cnt = visit_count;
  rave_value = v1;
}
inline void UctMoveInfo::AddValue(const UctValueType value, const UctValueType count) {
  visit_count += count;
  uct_w += value * count;
  uct_Q = uct_w / visit_count;
}

inline void UctMoveInfo::SetActionProb(const UctValueType prob) {
  uct_prior = prob;
}

typedef enum {
  PROVEN_NONE,
  PROVEN_WIN,
  PROVEN_LOSS
} UctProvenType;
std::ostream &operator<<(std::ostream &stream, const UctProvenType &type);

class UctNode {
 public:
  explicit UctNode(const UctMoveInfo &info, const UctNode *parent = 0);
  explicit UctNode(GoMove move, const UctNode *parent = 0);
  void AddGameResult(UctValueType eval);
  void AddGameResults(UctValueType eval, UctValueType count);
  void MergeResults(const UctNode &node);
  void RemoveGameResult(UctValueType eval);
  void RemoveGameResults(UctValueType eval, UctValueType count);
  UctValueType PosCount() const;
  UctValueType VisitCount() const;
  UctValueType MoveCount() const;
  const UctNode *FirstChild() const;
  bool HasChildren() const;
  UctValueType Mean() const;
  bool HasMean() const;
  size_t NumChildren() const;
  void SetFirstChild(const UctNode *child);
  void SetNumChildren(size_t nuChildren);
  void IncPosCount();
  void IncPosCount(UctValueType count);
  void DecPosCount();
  void DecPosCount(UctValueType count);
  void SetPosCount(UctValueType value);
  void InitializeValue(UctValueType value, UctValueType count);
  void CopyDataFrom(const UctNode &node, bool copyParent = true);
  void CopyNonPointerData(const UctNode &node);
  bool HasMove() const;
  GoMove Move() const;
  GoMove MoveNoCheck() const;
  void SetMove(GoMove move);
  UctValueType RaveCount() const;
  UctValueType RaveValue() const;
  bool HasRaveValue() const;
  void AddRaveValue(UctValueType value, UctValueType weight);
  void RemoveRaveValue(UctValueType value);
  void RemoveRaveValue(UctValueType value, UctValueType weight);
  void InitializeRaveValue(UctValueType value, UctValueType count);
  float PredictorValue() const;
  int VirtualLossCount() const;
  void AddVirtualLoss();
  void RemoveVirtualLoss();
  UctValueType KnowledgeCount() const;
  void SetKnowledgeCount(UctValueType count);
  bool IsProven() const;
  bool IsProvenWin() const;
  bool IsProvenLoss() const;
  UctProvenType ProvenType() const;
  void SetProvenType(UctProvenType type);
  float *Policy();
  void SetPolicy(float *newPolicy);
  void SetColor(SgBlackWhite color);
  SgBlackWhite GetColor() const;

#ifdef USE_DIRECT_VISITCOUNT
  void SetVisitCount(UctValueType count);
  UctValueType getTotalActionValue() const;
  UctValueType getMeanActionValue() const;
  void SetValue(UctValueType value);
  void AddValue(UctValueType value);
#endif

  UctValueType getPrior() const;
  void SetPrior(UctValueType prior);
  UctNode *Parent();
  const UctNode *Parent() const;
  void SetParent(UctNode *parent);

 private:
  UctStatisticsVolatile uct_stats;
#ifdef USE_DIRECT_VISITCOUNT
  volatile UctValueType m_visitCount;
  volatile UctValueType uct_w;
  volatile UctValueType uct_Q;
#endif

  float* policy;
  volatile UctValueType move_prior;
  UctNode* parent;
  const UctNode *volatile first_child;
  SgBlackWhite move_color;
  volatile size_t num_children;
  volatile GoMove move;
  volatile float predictor_val;
  UctStatisticsVolatile rave_value;
  volatile UctValueType pos_cnt;
  volatile UctValueType knowledge_cnt;
  volatile UctProvenType proven_type;
  volatile int v_loss_cnt;
};
std::ostream &operator<<(std::ostream &stream, const UctNode &node);

inline UctNode::UctNode(const UctMoveInfo &info, const UctNode *parent)
    : uct_stats(info.uct_value, info.visit_count),
      policy(nullptr),
#ifdef USE_DIRECT_VISITCOUNT
m_visitCount(info.visit_count),
uct_w(info.uct_w),
uct_Q(info.uct_Q),
#endif
      move_prior(info.uct_prior),
      parent(const_cast<UctNode *>(parent)),
      first_child(nullptr),
      move_color(SG_WHITE),
      num_children(0),
      move(info.uct_move),
      predictor_val(info.predictor_val),
      rave_value(info.rave_value, info.rave_cnt),
      pos_cnt(0),
      knowledge_cnt(0),
      proven_type(PROVEN_NONE),
      v_loss_cnt(0) {
}

inline UctNode::UctNode(GoMove move, const UctNode *parent)
    : uct_stats(move, 0),
      policy(0),
#ifdef USE_DIRECT_VISITCOUNT
m_visitCount(0),
uct_w(0),
uct_Q(0),
#endif
      move_prior(0),
      parent(const_cast<UctNode *>(parent)),
      first_child(nullptr),
      move_color(SG_WHITE),
      num_children(0),
      move(move),
      predictor_val(0),
      rave_value(0, 0),
      pos_cnt(0),

      knowledge_cnt(0),
      proven_type(PROVEN_NONE),
      v_loss_cnt(0) {
}

inline void UctNode::AddGameResult(UctValueType eval) {
  uct_stats.Add(eval);
}

inline void UctNode::AddGameResults(UctValueType eval, UctValueType count) {
  uct_stats.Add(eval, count);
}

inline void UctNode::MergeResults(const UctNode &node) {
  if (node.uct_stats.IsDefined())
    uct_stats.Add(node.uct_stats.Mean(), node.uct_stats.Count());
  if (node.rave_value.IsDefined())
    rave_value.Add(node.rave_value.Mean(), node.rave_value.Count());
}

inline void UctNode::RemoveGameResult(UctValueType eval) {
  uct_stats.Remove(eval);
}

inline void UctNode::RemoveGameResults(UctValueType eval, UctValueType count) {
  uct_stats.Remove(eval, count);
}

inline void UctNode::AddRaveValue(UctValueType value, UctValueType weight) {
  rave_value.Add(value, weight);
}

inline void UctNode::RemoveRaveValue(UctValueType value) {
  rave_value.Remove(value);
}

inline void UctNode::RemoveRaveValue(UctValueType value, UctValueType weight) {
  rave_value.Remove(value, weight);
}

inline void UctNode::CopyDataFrom(const UctNode &node, bool copyParent) {
  if (copyParent)
    parent = node.parent;
  first_child = node.first_child;
  num_children = node.num_children;

  uct_stats = node.uct_stats;
  move = node.move;
  predictor_val = node.predictor_val;
  rave_value = node.rave_value;
  pos_cnt = node.pos_cnt;
#ifdef USE_DIRECT_VISITCOUNT
  m_visitCount = node.m_visitCount;
  uct_w = node.uct_w;
  uct_Q = node.uct_Q;
#endif
  knowledge_cnt = node.knowledge_cnt;
  proven_type = node.proven_type;
  v_loss_cnt = node.v_loss_cnt;
  move_color = node.move_color;
  move_prior = node.move_prior;
  policy = node.policy;
}

inline void UctNode::CopyNonPointerData(const UctNode &node) {
  uct_stats = node.uct_stats;
  move = node.move;
  predictor_val = node.predictor_val;
  rave_value = node.rave_value;
  pos_cnt = node.pos_cnt;
#ifdef USE_DIRECT_VISITCOUNT
  m_visitCount = node.m_visitCount;
  uct_w = node.uct_w;
  uct_Q = node.uct_Q;
#endif
  knowledge_cnt = node.knowledge_cnt;
  proven_type = node.proven_type;
  v_loss_cnt = node.v_loss_cnt;
  move_color = node.move_color;
  move_prior = node.move_prior;
}

inline const UctNode *UctNode::FirstChild() const {
  DBG_ASSERT(HasChildren());
  return first_child;
}

inline bool UctNode::HasChildren() const {

  bool retval = (num_children > 0);
  SgSynchronizeThreadMemory();
  return retval;
}

inline bool UctNode::HasMean() const {
  return uct_stats.IsDefined();
}

inline bool UctNode::HasRaveValue() const {
  return rave_value.IsDefined();
}

inline int UctNode::VirtualLossCount() const {
  return v_loss_cnt;
}

inline void UctNode::AddVirtualLoss() {
  v_loss_cnt++;
}

inline void UctNode::RemoveVirtualLoss() {
  v_loss_cnt--;
}

inline void UctNode::IncPosCount() {
  ++pos_cnt;
}

inline void UctNode::IncPosCount(UctValueType count) {
  pos_cnt += count;
}

inline void UctNode::DecPosCount() {
  UctValueType posCount = pos_cnt;
  if (posCount > 0) {
    pos_cnt = posCount - 1;
  }
}

inline void UctNode::DecPosCount(UctValueType count) {
  UctValueType posCount = pos_cnt;
  if (posCount >= count) {
    pos_cnt = posCount - count;
  }
}

inline bool UctNode::HasMove() const {
  return move != GO_NULLMOVE;
}

inline void UctNode::InitializeValue(UctValueType value, UctValueType count) {
  uct_stats.Initialize(value, count);
}

inline void UctNode::InitializeRaveValue(UctValueType value, UctValueType count) {
  rave_value.Initialize(value, count);
}

inline UctValueType UctNode::Mean() const {
  return uct_stats.Mean();
}

inline GoMove UctNode::Move() const {
  DBG_ASSERT(HasMove());
  return move;
}

inline GoMove UctNode::MoveNoCheck() const {
  return move;
}

inline void UctNode::SetMove(GoMove move) {
  move = move;
}

inline UctValueType UctNode::MoveCount() const {
  return uct_stats.Count();
}

inline size_t UctNode::NumChildren() const {
  return num_children;
}

inline UctValueType UctNode::PosCount() const {
  return pos_cnt;
}

inline UctValueType UctNode::VisitCount() const {
#ifdef USE_DIRECT_VISITCOUNT
  return m_visitCount;
#else
  return uct_stats.Count();
#endif
}

inline float UctNode::PredictorValue() const {
  return predictor_val;
}

inline UctValueType UctNode::RaveCount() const {
  return rave_value.Count();
}

inline UctValueType UctNode::RaveValue() const {
  return rave_value.Mean();
}

inline void UctNode::SetFirstChild(const UctNode *child) {
  first_child = child;
}

inline void UctNode::SetNumChildren(size_t nuChildren) {
  // DBG_ASSERT(nuChildren >= 0);
  num_children = nuChildren;
}

inline void UctNode::SetPosCount(UctValueType value) {
  pos_cnt = value;
}

inline UctValueType UctNode::KnowledgeCount() const {
  return knowledge_cnt;
}

inline void UctNode::SetKnowledgeCount(UctValueType count) {
  knowledge_cnt = count;
}

inline bool UctNode::IsProven() const {
  return proven_type != PROVEN_NONE;
}

inline bool UctNode::IsProvenWin() const {
  return proven_type == PROVEN_WIN;
}

inline bool UctNode::IsProvenLoss() const {
  return proven_type == PROVEN_LOSS;
}

inline UctProvenType UctNode::ProvenType() const {
  return proven_type;
}

inline void UctNode::SetProvenType(volatile UctProvenType type) {
  proven_type = type;
}

inline float *UctNode::Policy() {
  return policy;
}
inline void UctNode::SetPolicy(float *newPolicy) {
  delete policy;
  policy = newPolicy;
}

inline void UctNode::SetColor(SgBlackWhite color) {
  move_color = color;
}

inline SgBlackWhite UctNode::GetColor() const {
  return move_color;
}

#ifdef USE_DIRECT_VISITCOUNT
inline void UctNode::SetVisitCount(UctValueType count)
{
    m_visitCount = count;
}

inline UctValueType UctNode::getTotalActionValue() const {
    return uct_w;
}
inline UctValueType UctNode::getMeanActionValue() const {
    return uct_Q;
}

inline void UctNode::SetValue(UctValueType value) {
  uct_w = value;
}

inline void UctNode::AddValue(UctValueType value)
{
  m_visitCount += 1;
  uct_w += value;
  uct_Q = uct_w / m_visitCount;
}
#endif

inline UctValueType UctNode::getPrior() const {
  return move_prior;
}

inline void UctNode::SetPrior(UctValueType prior) {
  move_prior = prior;
}

inline UctNode *UctNode::Parent() {
  return parent;
}

inline const UctNode *UctNode::Parent() const {
  return parent;
}

inline void UctNode::SetParent(UctNode *parent) {
  this->parent = parent;
}
class UctNodeAllocator {
 public:
  UctNodeAllocator();
  ~UctNodeAllocator();
  void Clear();
  bool RangeRemoveExcept(UctNode *start, int size, UctNode *retain);
  bool HasCapacity(std::size_t n) const;
  std::size_t NuNodes() const;
  std::size_t MaxNodes() const;
  void SetMaxNodes(std::size_t maxNodes);
  bool Contains(const UctNode &node) const;
  const UctNode *Start() const;
  UctNode *Finish();
  const UctNode *Finish() const;
  UctNode *CreateOne(GoMove move, const UctNode *parent = 0);
  UctNode *CreateOneAtEnd(GoMove move, const UctNode *parent = 0);
  UctNode *Create(const std::vector<UctMoveInfo> &moves, const UctNode *parent, UctValueType &count_);
  UctNode *CreateN(std::size_t n, const UctNode *parent = 0);
  void Swap(UctNodeAllocator &allocator);
  uint64_t GetMaxMemoryUsed();

  UctNodeAllocator &operator=(const UctNodeAllocator &tree) = delete;

 private:
  UctNode *node_start;
  UctNode *node_end;
  UctNode *hole_start;
  UctNode *hole_end;
  UctNode *block_end;
  UctNode *max_accessed;
};

inline UctNodeAllocator::UctNodeAllocator() {
  node_start = nullptr;
  node_end = nullptr;
  block_end = nullptr;
  hole_start = nullptr;
  hole_end = nullptr;
  max_accessed = nullptr;
}

inline void UctNodeAllocator::Clear() {
  if (node_start != nullptr) {
    for (UctNode *it = node_start; it != node_end; ++it)
      it->~UctNode();
  }
  node_end = node_start;
  hole_start = node_start;
  hole_end = node_start;
  max_accessed = node_end;
}

inline bool UctNodeAllocator::RangeRemoveExcept(UctNode *start, int size, UctNode *retain) {
  if (start != 0) {
    UctNode *end = start + size;
    if (start != retain) {
      start->CopyDataFrom(*retain);
      start->SetFirstChild(retain->FirstChild());
      start->SetNumChildren(retain->NumChildren());
      start->SetParent(retain->Parent());
    }

    if (end == node_end)
      node_end = start + 1;
    else if (hole_end - hole_start < size - 1) {
      hole_start = start + 1;
      hole_end = end;
    }
  }
  return true;
}

inline UctNode *UctNodeAllocator::CreateOne(GoMove move, const UctNode *parent) {
  DBG_ASSERT(HasCapacity(1));
  UctNode *childStart = node_end;
  if (hole_end - hole_start >= 1) {
    childStart = hole_start;
    hole_start += 1;
  } else {
    node_end++;
    if (max_accessed < node_end)
      max_accessed = node_end;
  }

  new(childStart) UctNode(move, parent);
  if (parent)
    childStart->SetColor(SgOppBW(parent->GetColor()));
  return childStart;
}

inline UctNode *UctNodeAllocator::CreateOneAtEnd(GoMove move, const UctNode *parent) {
  DBG_ASSERT(HasCapacity(1));
  UctNode *childStart = node_end;
  node_end++;
  if (max_accessed < node_end)
    max_accessed = node_end;
  new(childStart) UctNode(move, parent);
  if (parent)
    childStart->SetColor(SgOppBW(parent->GetColor()));
  return childStart;
}

inline UctNode *
UctNodeAllocator::Create(const std::vector<UctMoveInfo> &moves, const UctNode *parent, UctValueType &count_) {
  DBG_ASSERT(HasCapacity(moves.size()));

  UctNode *childStart;
  if (std::size_t(hole_end - hole_start) >= moves.size())
  {
    childStart = hole_start;
    hole_start += moves.size();
  } else {
    childStart = node_end;
    node_end += moves.size();
    if (max_accessed < node_end)
      max_accessed = node_end;
  }

  count_ = 0;
  SgBlackWhite color = SgOppBW(parent->GetColor());
  for (auto it = moves.begin(); it != moves.end(); ++it, ++childStart) {
    new(childStart) UctNode(*it, parent);
    childStart->SetColor(color);
    count_ += it->visit_count;
  }
  return childStart - moves.size();
}

inline UctNode *UctNodeAllocator::CreateN(std::size_t n, const UctNode *parent) {
  DBG_ASSERT(HasCapacity(n));
  UctNode *childStart = node_end;
  if ((std::size_t) (hole_end - hole_start) >= n) {
    childStart = hole_start;
    hole_start += n;
  } else {
    node_end += n;
    if (max_accessed < node_end)
      max_accessed = node_end;
  }
  UctNode *child = childStart;
  SgBlackWhite color = parent ? SgOppBW(parent->GetColor()) : SG_WHITE;
  for (size_t i = 0; i < n; ++i, ++child) {
    new(child) UctNode(GO_NULLMOVE, parent);
    child->SetColor(color);
  }

  return childStart;
}

inline uint64_t UctNodeAllocator::GetMaxMemoryUsed() {
  return (char *) max_accessed - (char *) node_start;
}

inline UctNode *UctNodeAllocator::Finish() {
  return node_end;
}

inline const UctNode *UctNodeAllocator::Finish() const {
  return node_end;
}

inline bool UctNodeAllocator::HasCapacity(std::size_t n) const {
  return (std::size_t(hole_end - hole_start) >= n) || (node_end + n <= block_end);
}

inline std::size_t UctNodeAllocator::MaxNodes() const {
  return block_end - node_start;
}

inline std::size_t UctNodeAllocator::NuNodes() const {
  return node_end - node_start;
}

inline const UctNode *UctNodeAllocator::Start() const {
  return node_start;
}
class UctSearchTree {
 public:
  friend class UctChildNodeIterator;
  UctSearchTree();
  void CreateAllocators(std::size_t num_ths);
  void AddGameResult(const UctNode &node, const UctNode *father,
                     UctValueType eval);
  void AddGameResults(const UctNode &node, const UctNode *father,
                      UctValueType eval, UctValueType count);
  void RemoveGameResult(const UctNode &node, const UctNode *father,
                        UctValueType eval);
  void RemoveGameResults(const UctNode &node, const UctNode *father,
                         UctValueType eval, UctValueType count);

  void AddVirtualLoss(const UctNode &node);
  void RemoveVirtualLoss(const UctNode &node);

  void SetProvenType(const UctNode &node, UctProvenType type);
  size_t GetMaxMemoryUsed(size_t allocatorID);

  void Clear();
  std::size_t MaxNodes() const;
  void SetMaxNodes(std::size_t maxNodes);
  void Swap(UctSearchTree &tree);

  bool HasCapacity(std::size_t allocatorId, std::size_t n) const;
  void CreateChildren(std::size_t allocatorId, const UctNode &node,
                      const std::vector<UctMoveInfo> &moves);
  const UctNode *CreateChildIfNotExist(std::size_t allocatorId, const UctNode *parent, GoMove move);

  void Prune(std::size_t allocatorId, const UctNode &node, UctNode *exception);
  void Expand(std::size_t allocatorId, const UctNode &leafNode, const std::vector<UctMoveInfo> &moves);

  void MergeChildren(std::size_t allocatorId, const UctNode &node,
                     const std::vector<UctMoveInfo> &moves,
                     bool deleteChildTrees);

  void ExtractSubtree(UctSearchTree &target, const UctNode &node,
                      bool warnTruncate,
                      double maxTime = std::numeric_limits<double>::max(),
                      UctValueType minCount = 0) const;

  void CopyPruneLowCount(UctSearchTree &target, UctValueType minCount,
                         bool warnTruncate,
                         double maxTime = std::numeric_limits<double>::max()) const;
  const UctNode &Root() const;
  UctNode &Root();
  std::size_t NuAllocators() const;
  std::size_t NuNodes() const;
  std::size_t NuNodes(std::size_t allocatorId) const;

  void AddRaveValue(const UctNode &node, UctValueType value, UctValueType weight);
  void RemoveRaveValue(const UctNode &node, UctValueType value, UctValueType weight);
  void InitializeValue(const UctNode &node, UctValueType value,
                       UctValueType count);
  void SetPosCount(const UctNode &node, UctValueType posCount);
  void InitializeRaveValue(const UctNode &node, UctValueType value, UctValueType count);
  void ApplyFilter(const UctSearchTree &tree, std::size_t allocatorId, const UctNode &parent,
                   const std::vector<GoMove> &rootFilter);

  void CheckConsistency() const;
  bool Contains(const UctNode &node) const;
  void DumpDebugInfo(std::ostream &out) const;

  UctSearchTree &operator=(const UctSearchTree &tree) = delete;

 private:
  std::size_t max_nodes_allowed;
  UctNode tree_root;
  std::vector<boost::shared_ptr<UctNodeAllocator> > node_allocators;

  UctNodeAllocator &Allocator(std::size_t i);
  const UctNodeAllocator &Allocator(std::size_t i) const;
  UctProvenType CopySubtree(UctSearchTree &target, UctNode &targetNode,
                              const UctNode &node, UctValueType minCount,
                              std::size_t &currentAllocatorId, bool warnTruncate,
                              bool &abort, SgTimer &timer, double maxTime,
                              bool alwaysKeepProven) const;
  void ThrowConsistencyError(const std::string &message) const;
};

inline void UctSearchTree::AddGameResult(const UctNode &node,
                                     const UctNode *father, UctValueType eval) {
  DBG_ASSERT(Contains(node));
  if (father != nullptr)
    const_cast<UctNode *>(father)->IncPosCount();
  const_cast<UctNode &>(node).AddGameResult(eval);
}

inline void UctSearchTree::AddGameResults(const UctNode &node,
                                      const UctNode *father, UctValueType eval,
                                      UctValueType count) {
  DBG_ASSERT(Contains(node));
  if (father != 0)
    const_cast<UctNode *>(father)->IncPosCount(count);
  const_cast<UctNode &>(node).AddGameResults(eval, count);
}

inline void UctSearchTree::CreateChildren(std::size_t allocatorId,
                                      const UctNode &node,
                                      const std::vector<UctMoveInfo> &moves) {
  DBG_ASSERT(Contains(node));
  auto &nonConstNode = const_cast<UctNode &>(node);
  DBG_ASSERT(moves.size() <= std::size_t(std::numeric_limits<int>::max()));
  size_t nuChildren = moves.size();
  DBG_ASSERT(nuChildren > 0);
  UctNodeAllocator &allocator = Allocator(allocatorId);
  DBG_ASSERT(allocator.HasCapacity(nuChildren));

  DBG_ASSERT(NuAllocators() > 1 || !node.HasChildren());

  UctValueType parentCount = 0;
  const UctNode *firstChild = allocator.Create(moves, &node, parentCount);
  nonConstNode.SetPosCount(parentCount);
  SgSynchronizeThreadMemory();
  nonConstNode.SetFirstChild(firstChild);
  SgSynchronizeThreadMemory();
  nonConstNode.SetNumChildren(nuChildren);
}
inline void
UctSearchTree::Expand(std::size_t allocatorId, const UctNode &leafNode, const std::vector<UctMoveInfo> &moves) {
  DBG_ASSERT(Contains(leafNode));
  auto &nonConstLeaf = const_cast<UctNode &>(leafNode);
  DBG_ASSERT(moves.size() <= std::size_t(std::numeric_limits<int>::max()));
  size_t nuChildren = moves.size();
  DBG_ASSERT(nuChildren > 0);
  UctNodeAllocator &allocator = Allocator(allocatorId);
  DBG_ASSERT(allocator.HasCapacity(nuChildren));
  DBG_ASSERT(NuAllocators() > 1 || !leafNode.HasChildren());
  UctValueType count = 0;
  const UctNode *firstChild = allocator.Create(moves, &leafNode, count);
  SgSynchronizeThreadMemory();
  nonConstLeaf.SetFirstChild(firstChild);
  SgSynchronizeThreadMemory();
  nonConstLeaf.SetNumChildren(nuChildren);
}

inline void UctSearchTree::RemoveGameResult(const UctNode &node,
                                        const UctNode *father, UctValueType eval) {
  DBG_ASSERT(Contains(node));
  if (father != nullptr)
    const_cast<UctNode *>(father)->DecPosCount();
  const_cast<UctNode &>(node).RemoveGameResult(eval);
}

inline void UctSearchTree::RemoveGameResults(const UctNode &node,
                                         const UctNode *father, UctValueType eval,
                                         UctValueType count) {
  DBG_ASSERT(Contains(node));
  if (father != nullptr)
    const_cast<UctNode *>(father)->DecPosCount(count);
  const_cast<UctNode &>(node).RemoveGameResults(eval, count);
}

inline void UctSearchTree::AddVirtualLoss(const UctNode &node) {
  const_cast<UctNode &>(node).AddVirtualLoss();
}

inline void UctSearchTree::RemoveVirtualLoss(const UctNode &node) {
  const_cast<UctNode &>(node).RemoveVirtualLoss();
}

inline void UctSearchTree::AddRaveValue(const UctNode &node, UctValueType value,
                                    UctValueType weight) {
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).AddRaveValue(value, weight);
}

inline void UctSearchTree::RemoveRaveValue(const UctNode &node, UctValueType value,
                                       UctValueType weight) {
  SuppressUnused(weight);
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).RemoveRaveValue(value, weight);
}

inline UctNodeAllocator &UctSearchTree::Allocator(std::size_t i) {
  DBG_ASSERT(i < node_allocators.size());
  return *node_allocators[i];
}

inline const UctNodeAllocator &UctSearchTree::Allocator(std::size_t i) const {
  DBG_ASSERT(i < node_allocators.size());
  return *node_allocators[i];
}

inline bool UctSearchTree::HasCapacity(std::size_t allocatorId,
                                   std::size_t n) const {
  return Allocator(allocatorId).HasCapacity(n);
}

inline void UctSearchTree::InitializeValue(const UctNode &node,
                                       UctValueType value, UctValueType count) {
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).InitializeValue(value, count);
}

inline void UctSearchTree::InitializeRaveValue(const UctNode &node,
                                           UctValueType value, UctValueType count) {
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).InitializeRaveValue(value, count);
}

inline std::size_t UctSearchTree::MaxNodes() const {
  return max_nodes_allowed;
}

inline std::size_t UctSearchTree::NuAllocators() const {
  return node_allocators.size();
}

inline std::size_t UctSearchTree::NuNodes(std::size_t allocatorId) const {
  return Allocator(allocatorId).NuNodes();
}

inline const UctNode &UctSearchTree::Root() const {
  return tree_root;
}

inline UctNode &UctSearchTree::Root() {
  return tree_root;
}

inline void UctSearchTree::SetPosCount(const UctNode &node,
                                   UctValueType posCount) {
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).SetPosCount(posCount);
}

inline void UctSearchTree::SetProvenType(const UctNode &node,
                                     UctProvenType type) {
  DBG_ASSERT(Contains(node));
  const_cast<UctNode &>(node).SetProvenType(type);
}


class UctChildNodeIterator {
 public:
  UctChildNodeIterator(const UctSearchTree &tree, const UctNode &node);
  const UctNode &operator*() const;
  const UctNode *operator()() const;
  void operator++();
  explicit operator bool() const;

  operator int() const = delete;

 private:
  const UctNode *m_current;
  const UctNode *m_last;
};

inline UctChildNodeIterator::UctChildNodeIterator(const UctSearchTree &tree,
                                              const UctNode &node) {
  SG_DEBUG_ONLY(tree);
  DBG_ASSERT(tree.Contains(node));
  DBG_ASSERT(node.HasChildren());
  m_current = node.FirstChild();
  m_last = m_current + node.NumChildren();
}

inline const UctNode &UctChildNodeIterator::operator*() const {
  return *m_current;
}

inline const UctNode *UctChildNodeIterator::operator()() const {
  return m_current;
}

inline void UctChildNodeIterator::operator++() {
  ++m_current;
}

inline UctChildNodeIterator::operator bool() const {
  return (m_current < m_last);
}

class UctTreeIterator {
 public:
  explicit UctTreeIterator(const UctSearchTree &tree);
  const UctNode &operator*() const;
  void operator++();
  explicit operator bool() const;

  operator int() const = delete;

 private:
  const UctSearchTree &m_tree;
  const UctNode *m_current;
  std::stack<boost::shared_ptr<UctChildNodeIterator> > m_stack;
};

#endif