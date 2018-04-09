
#include "platform/SgSystem.h"
#include "SgGameWriter.h"

#include <iostream>
#include "platform/SgDebug.h"
#include "platform/SgException.h"
#include "SgNode.h"

using namespace std;
using SgPropUtil::GetPointFmt;

SgGameWriter::SgGameWriter(ostream &out)
    : m_out(out),
      m_fileFormat(4),
      m_numPropsOnLine(0) {}

void SgGameWriter::WriteGame(const SgNode &root, bool allProps,
                             int fileFormat, int gameNumber, int defaultSize) {
  if (!m_out)
    throw SgException("SgGameWriter: write error");
  if (fileFormat != 0)
    m_fileFormat = fileFormat;
  else if (root.HasProp(SG_PROP_FORMAT))
    m_fileFormat = root.GetIntProp(SG_PROP_FORMAT);
  SgPropPointFmt fmt = GetPointFmt(gameNumber);
  WriteSubtree(root, allProps, defaultSize, fmt);
  m_out.put('\n');
}

void SgGameWriter::WriteSubtree(const SgNode &nodeRef, bool allProps,
                                int boardSize, SgPropPointFmt fmt) {
  StartNewLine();
  m_out.put('(');
  const SgNode *node = &nodeRef;
  do {
    HandleProps(node, boardSize);
    m_out.put(';');
    WriteNode(*node, allProps, boardSize, fmt);
    node = node->LeftMostSon();
  } while (node && !node->HasRightBrother());

  while (node) {
    HandleProps(node, boardSize);
    WriteSubtree(*node, allProps, boardSize, fmt);
    node = node->RightBrother();
  }
  m_out.put(')');
}

void SgGameWriter::HandleProps(const SgNode *node, int &boardSize) const {
  int value;
  bool hasSizeProp = node->GetIntProp(SG_PROP_SIZE, &value);
  if (hasSizeProp) {
    if (value >= GO_MIN_SIZE && value <= GO_MAX_SIZE)
      boardSize = value;
    else
      SgWarning() << "Invalid size " << value;
  }
}

void SgGameWriter::WriteNode(const SgNode &node, bool allProps, int boardSize,
                             SgPropPointFmt fmt) {
  for (SgPropListIterator it(node.Props()); it; ++it) {
    SgProp *prop = *it;
    vector<string> values;
    if ((allProps || ShouldWriteProperty(*prop))
        && prop->ToString(values, boardSize, fmt, m_fileFormat)) {
      if (prop->Flag(SG_PROPCLASS_NEWLINE))
        StartNewLine();
      m_out << prop->Label();
      for (vector<string>::const_iterator it2 = values.begin();
           it2 != values.end(); ++it2)
        m_out << '[' << (*it2) << ']';
      if (++m_numPropsOnLine >= 10)
        StartNewLine();
    }
  }

  if (node.HasProp(SG_PROP_GAME))
    StartNewLine();
}

void SgGameWriter::StartNewLine() {
  if (m_numPropsOnLine > 0) {
    m_numPropsOnLine = 0;
    m_out.put('\n');
  }
}

bool SgGameWriter::ShouldWriteProperty(const SgProp &prop) {
  if (m_fileFormat == 3 && prop.Flag(SG_PROPCLASS_NOTCLEAN))
    return false;

  if (prop.Label() == "")
    return false;

  if (prop.ID() == SG_PROP_TIME_BLACK || prop.ID() == SG_PROP_TIME_WHITE)
    return false;

  return true;
}
