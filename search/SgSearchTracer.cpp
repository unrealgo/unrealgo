
#include "platform/SgSystem.h"
#include "SgSearchTracer.h"

#include <iomanip>
#include <sstream>
#include "SgHashTable.h"
#include "SgSearchValue.h"

using namespace std;


SgSearchTracer::SgSearchTracer(SgNode *root)
    : m_traceNode(root) {}

SgSearchTracer::~SgSearchTracer() {}

void SgSearchTracer::AddMoveProp(SgNode *node, GoMove move,
                                 SgBlackWhite player) {
  node->AddMoveProp(move, player);
}

void SgSearchTracer::AddTraceNode(GoMove move, SgBlackWhite player) {
  if (m_traceNode != 0) {
    m_traceNode = m_traceNode->NewRightMostSon();
    AddMoveProp(m_traceNode, move, player);
  }
}

void SgSearchTracer::AppendTrace(SgNode *toNode) {
  if (m_traceNode != 0) {
    m_traceNode->Root()->AppendTo(toNode);
    m_traceNode = 0;
  }
}

void SgSearchTracer::InitTracing(const string &type) {
  DBG_ASSERT(!m_traceNode);
  m_traceNode = new SgNode();
  m_traceNode->Add(new SgPropText(SG_PROP_COMMENT, type));
}

void SgSearchTracer::StartOfDepth(int depth) {
  DBG_ASSERT(m_traceNode != 0);
  if (depth > 0 && m_traceNode->HasFather()) {
    m_traceNode = m_traceNode->Father();
  }
  m_traceNode = m_traceNode->NewRightMostSon();
  DBG_ASSERT(m_traceNode != 0);
  m_traceNode->SetIntProp(SG_PROP_MAX_DEPTH, depth);
  ostringstream stream;
  stream << "Iteration d = " << depth << ' ';
  m_traceNode->AddComment(stream.str());
}

void SgSearchTracer::TakeBackTraceNode() {
  if (m_traceNode != 0)
    m_traceNode = m_traceNode->Father();
}

void SgSearchTracer::TraceComment(const char *comment) const {
  if (m_traceNode != 0) {
    m_traceNode->AddComment(comment);
    m_traceNode->AddComment("\n");
  }
}

void SgSearchTracer::TraceValue(int value, SgBlackWhite toPlay) const {
  DBG_ASSERT(m_traceNode != 0);
  int v = (toPlay == SG_WHITE) ? -value : +value;
  m_traceNode->Add(new SgPropValue(SG_PROP_VALUE, v));
}

void SgSearchTracer::TraceValue(int value, SgBlackWhite toPlay,
                                const char *comment, bool isExact) const {
  TraceValue(value, toPlay);
  if (comment != 0)
    TraceComment(comment);
  if (isExact) {
    m_traceNode->Add(new SgPropMultiple(SG_PROP_CHECK, 1));
    TraceComment("exact");
  }
}
