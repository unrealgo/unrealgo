#ifndef SG_SEARCHTRACER_H
#define SG_SEARCHTRACER_H

#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "SgNode.h"

class SgSearchTracer {
 public:
  explicit SgSearchTracer(SgNode *root);
  virtual ~SgSearchTracer();
  void AddTraceNode(GoMove move, SgBlackWhite player);
  SgNode *TraceNode() const;
  void TraceComment(const char *comment) const;
  void StartOfDepth(int depth);
  virtual void AddMoveProp(SgNode *node, GoMove move,
                           SgBlackWhite player);
  void TraceValue(int value, SgBlackWhite toPlay) const;
  void TraceValue(int value, SgBlackWhite toPlay,
                  const char *comment, bool isExact) const;
  void TakeBackTraceNode();
  virtual bool TraceIsOn() const;
  virtual void InitTracing(const std::string &type);
  void AppendTrace(SgNode *toNode);

 protected:
  SgNode *m_traceNode;

 private:
  SgSearchTracer(const SgSearchTracer &);
  SgSearchTracer &operator=(const SgSearchTracer &);
};

inline SgNode *SgSearchTracer::TraceNode() const {
  return m_traceNode;
}

inline bool SgSearchTracer::TraceIsOn() const {
  return true;
}

#endif
