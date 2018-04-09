
#include "platform/SgSystem.h"
#include "SgGameReader.h"

#include <cstdio>
#include <iostream>
#include <map>
#include <vector>
#include "platform/SgDebug.h"
#include "platform/SgException.h"
#include "SgNode.h"

using namespace std;

namespace {
void PrintWarning(ostream &out, SgGameReader::Warnings &warnings, int index,
                  const char *text) {
  if (!warnings.test(index))
    return;
  out << text << '\n';
  warnings.reset(index);
}
}


SgGameReader::SgGameReader(istream &in, int defaultSize)
    : m_in(in),
      m_defaultSize(defaultSize),
      m_fileFormat(4) {}

bool SgGameReader::GetIntProp(const SgGameReader::RawProperties &properties,
                              const string &label, int &value) {
  RawProperties::const_iterator it = properties.find(label);
  if (it == properties.end() || it->second.size() == 0)
    return false;
  istringstream in(it->second[0]);
  in >> value;
  return !in.fail();
}
void SgGameReader::HandleProperties(SgNode *node,
                                    const RawProperties &properties,
                                    int &boardSize, SgPropPointFmt &fmt) {
  int value;
  if (GetIntProp(properties, "SZ", value)) {
    if (value < GO_MIN_SIZE || value > GO_MAX_SIZE)
      m_warnings.set(INVALID_BOARDSIZE);
    else
      boardSize = value;
  }
  if (GetIntProp(properties, "GM", value))
    fmt = SgPropUtil::GetPointFmt(value);
  for (RawProperties::const_iterator it = properties.begin();
       it != properties.end(); ++it) {
    const string &label = it->first;
    const vector<string> &values = it->second;
    if (values.size() == 0)
      m_warnings.set(PROPERTY_WITHOUT_VALUE);
    SgProp *prop;
    SgPropID id = SgProp::GetIDOfLabel(label);
    if (id != SG_PROP_NONE)
      prop = SgProp::CreateProperty(id);
    else
      prop =
          new SgPropUnknown(SG_PROP_UNKNOWN, label, vector<string>());
    if (prop->FromString(values, boardSize, fmt))
      node->Add(prop);
  }
}

void SgGameReader::PrintWarnings(ostream &out) const {
  Warnings warnings = m_warnings;
  PrintWarning(out, warnings, INVALID_BOARDSIZE, "Invalid board size");
  PrintWarning(out, warnings, PROPERTY_WITHOUT_VALUE,
               "Property withour value");
  DBG_ASSERT(warnings.none());
}

SgNode *SgGameReader::ReadGame(bool resetWarnings) {
  if (resetWarnings)
    m_warnings.reset();
  SgNode *root = 0;
  int c;
  while ((c = m_in.get()) != EOF) {
    while (c != '(' && c != EOF)
      c = m_in.get();
    if (c == EOF)
      break;
    root = ReadSubtree(0, m_defaultSize, SG_PROPPOINTFMT_GO);
    if (root)
      root = root->Root();
    if (root)
      break;
  }
  return root;
}

void SgGameReader::ReadGames(SgVectorOf<SgNode> *rootList) {
  m_warnings.reset();
  DBG_ASSERT(rootList);
  rootList->Clear();
  while (true) {
    SgNode *root = ReadGame(false);
    if (root)
      rootList->PushBack(root);
    else
      break;
  }
}

string SgGameReader::ReadLabel(int c) {
  string label;
  label += static_cast<char>(c);
  while ((c = m_in.get()) != EOF
      && (('A' <= c && c <= 'Z')
          || ('a' <= c && c <= 'z')
          || ('0' <= c && c <= '9')))
    label += static_cast<char>(c);
  if (c != EOF)
    m_in.unget();
  return label;
}

SgNode *SgGameReader::ReadSubtree(SgNode *node, int boardSize,
                                  SgPropPointFmt fmt) {
  RawProperties properties;
  int c;
  while ((c = m_in.get()) != EOF && c != ')') {
    if ('A' <= c && c <= 'Z') {
      string label = ReadLabel(c);
      m_in >> ws;
      string value;
      while (ReadValue(value))
        properties[label].push_back(value);
    } else if (c == ';') {
      if (node) {
        HandleProperties(node, properties, boardSize, fmt);
        properties.clear();
        node = node->NewRightMostSon();
      } else
        node = new SgNode();
    } else if (c == '(') {
      HandleProperties(node, properties, boardSize, fmt);
      properties.clear();
      ReadSubtree(node, boardSize, fmt);
    }
  }
  HandleProperties(node, properties, boardSize, fmt);
  return node;
}

bool SgGameReader::ReadValue(string &value) {
  m_in >> ws;
  value = "";
  int c;
  if ((c = m_in.get()) == EOF)
    return false;
  if (c != '[') {
    m_in.unget();
    return false;
  }
  bool inEscape = false;
  while ((c = m_in.get()) != EOF && (c != ']' || inEscape)) {
    if (c != '\n')
      value += static_cast<char>(c);
    if (inEscape)
      inEscape = false;
    else if (c == '\\')
      inEscape = true;
  }
  return true;
}
