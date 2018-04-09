
#ifndef SG_NODE_H
#define SG_NODE_H

#include <string>
#include "SgProp.h"
#include "board/GoPointSet.h"
#include "lib/SgVector.h"

class SgNode {
 public:
  enum Direction {
    PREVIOUS,
    NEXT,
    NEXT_RIGHTMOST,
    PREV_DEPTHFIRST,
    NEXT_DEPTHFIRST,
    PREV_TERMINAL,
    NEXT_TERMINAL,
    PREV_BRANCH,
    NEXT_BRANCH,
    LEFT_BROTHER,
    RIGHT_BROTHER,
    MAIN_BRANCH,
    START_OF_GAME,
    END_OF_GAME
  };
  SgNode();
  ~SgNode();
  SgNode *CopyTree() const;
  SgVector<GoPoint> VectorProp(SgPropID prop) const;

  bool HasFather() const {
    return (m_father != 0);
  }

  bool IsRoot() const {
    return (m_father == 0);
  }

  bool HasLeftBrother() const;

  bool HasRightBrother() const {
    return (m_brother != 0);
  }

  bool HasSon() const {
    return (m_son != 0);
  }

  bool HasBrother() const {
    return HasLeftBrother() || HasRightBrother();
  }
  bool IsOnMain() const;

  bool IsTerminal() const {
    return (m_son == 0);
  }

  bool IsBranchPoint() const {
    return (m_son && m_son->m_brother);
  }

  int NumSons() const;
  int NumLeftBrothers() const;
  SgNode *Root() const;

  SgNode *Father() const {
    return m_father;
  }

  SgNode *LeftBrother() const;

  SgNode *RightBrother() const {
    return m_brother;
  }

  SgNode *LeftMostSon() const {
    return m_son;
  }

  SgNode *RightMostSon() const;
  SgNode *NextDepthFirst() const;
  SgNode *PrevDepthFirst() const;
  SgNode *NodeInDirection(Direction dir) const;
  bool ContainsText(const std::string &findText);
  void PathToRoot(SgVectorOf<SgNode> *path) const;
  void ShortestPathTo(SgNode *node, int *numBack,
                      SgVectorOf<SgNode> *path) const;
  void PromoteNode();
  void PromotePath();
  void DeleteSubtree();
  void DeleteBranches();
  void DeleteTree();
  SgNode *NewFather();
  SgNode *NewRightBrother();
  SgNode *NewLeftMostSon();
  SgNode *NewRightMostSon();
  void AppendTo(SgNode *n);
  static SgNode *LinkTrees(const SgVectorOf<SgNode> &roots);

  SgPropList &Props() {
    return m_props;
  }

  const SgPropList &Props() const {
    return m_props;
  }

  void Add(const SgProp *prop) {
    DBG_ASSERT(!prop->Flag(SG_PROPCLASS_ROOT) || !HasFather()
                  || !Father()->HasFather());
    m_props.Add(prop);
  }

  SgProp *Get(SgPropID id) const {
    return m_props.Get(id);
  }
  bool HasProp(SgPropID id) const;
  bool HasNodeMove() const;
  SgBlackWhite NodePlayer() const;
  GoPoint NodeMove() const;
  SgNode *TopProp(SgPropID id) const;
  int GetIntProp(SgPropID id) const;
  bool GetIntProp(SgPropID id, int *value) const;
  double GetRealProp(SgPropID id) const;
  void SetIntProp(SgPropID id, int value);
  void SetRealProp(SgPropID id, double value, int precision = 0);
  void SetStringProp(SgPropID id, const std::string &value);
  bool GetStringProp(SgPropID id, std::string *value) const;
  void SetListProp(SgPropID id, const SgVector<GoPoint> &value);
  void SetListProp(SgPropID id, const GoPointSet &value);
  void AddComment(const std::string &comment);
  SgProp *CopyPropFrom(const SgNode &sourceNode, SgPropID id);
  void CopyAllPropsFrom(const SgNode &sourceNode);
  SgProp *AddMoveProp(GoMove move, SgBlackWhite player);
  SgBlackWhite Player() const;
  int CountNodes(bool fSetPropOnThisNode);
  static void CopySubtree(const SgNode *node, SgNode *copy);

#ifndef NDEBUG

  static void GetStatistics(int *numAlloc, int *numUsed);
#endif

  static void MemCheck();

  static std::string TreeIndex(const SgNode *node);

 private:
  SgNode *m_son;
  SgNode *m_father;
  SgNode *m_brother;
  SgPropList m_props;
  bool m_marked;
  void LinkWithBrother(SgNode *node);

  void Mark() {
    m_marked = true;
  }

  void Unmark() {
    m_marked = false;
  }

  bool IsMarked() const {
    return m_marked;
  }

  SgNode(const SgNode &);
  SgNode &operator=(const SgNode &);

#ifndef NDEBUG
  static int s_allocated;
  static int s_freed;
#endif
};

class SgSonNodeIterator {
 public:
  explicit SgSonNodeIterator(SgNode *node)
      : m_nextNode(node->LeftMostSon()) {}

  void operator++() {
    m_nextNode = m_nextNode->RightBrother();
  }

  SgNode *operator*() const {
    DBG_ASSERT(m_nextNode);
    return m_nextNode;
  }

  explicit operator bool() const {
    return m_nextNode != 0;
  }

  operator int() const = delete;
  SgSonNodeIterator &operator=(const SgSonNodeIterator &) = delete;

 private:
  SgNode *m_nextNode;
  SgSonNodeIterator(const SgSonNodeIterator &);
};

class SgSonNodeConstIterator {
 public:
  explicit SgSonNodeConstIterator(const SgNode *node)
      : m_nextNode(node->LeftMostSon()) {}

  void operator++() {
    m_nextNode = m_nextNode->RightBrother();
  }

  const SgNode *operator*() const {
    DBG_ASSERT(m_nextNode);
    return m_nextNode;
  }

  explicit operator bool() const {
    return m_nextNode != 0;
  }

 private:
  SgNode *m_nextNode;
  operator int() const = delete;
  SgSonNodeConstIterator(const SgSonNodeConstIterator &);
  SgSonNodeConstIterator &operator=(const SgSonNodeConstIterator &);
};

class SgNodeIterator {
 public:
  explicit SgNodeIterator(SgNode *rootOfSubtree, bool postOrder = false);

  void Abort() {
    m_nextNode = 0;
  }

  void operator++() {
    DBG_ASSERT(m_nextNode);
    Next();
  }

  SgNode *operator*() const {
    DBG_ASSERT(m_nextNode);
    return m_nextNode;
  };

  operator bool() const {
    return m_nextNode != 0;
  }

 private:
  bool Next();
  bool m_postOrder;
  SgNode *const m_rootOfSubtree;
  SgNode *m_nextNode;
  operator int() const;
  SgNodeIterator(const SgNodeIterator &);
  SgNodeIterator &operator=(const SgNodeIterator &);
};

class SgNodeConstIterator {
 public:
  SgNodeConstIterator(const SgNode *rootOfSubtree, bool postOrder = false);
  bool Next();

  void Abort() {
    m_nextNode = 0;
  }

  void operator++() {
    DBG_ASSERT(m_nextNode);
    Next();
  }

  const SgNode *operator*() const {
    DBG_ASSERT(m_nextNode);
    return m_nextNode;
  };

  operator bool() const {
    return m_nextNode != 0;
  }

 private:
  bool m_postOrder;
  const SgNode *const m_rootOfSubtree;
  const SgNode *m_nextNode;
  operator int() const;
  SgNodeConstIterator(const SgNodeConstIterator &);
  SgNodeConstIterator &operator=(const SgNodeConstIterator &);
};

#endif
