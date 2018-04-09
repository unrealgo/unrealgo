
#ifndef SG_GAMEWRITER_H
#define SG_GAMEWRITER_H

#include <iosfwd>
#include "SgProp.h"

class SgNode;

class SgGameWriter {
 public:
  explicit SgGameWriter(std::ostream &out);
  void WriteGame(const SgNode &root, bool allProps, int fileFormat,
                 int gameNumber, int defaultSize);

 private:
  std::ostream &m_out;
  int m_fileFormat;
  int m_numPropsOnLine;
  void ConvertFormat(SgNode &root);
  void HandleProps(const SgNode *node, int &boardSize) const;

  bool ShouldWriteProperty(const SgProp &prop);
  void StartNewLine();
  void WriteNode(const SgNode &node, bool allProps, int boardSize,
                 SgPropPointFmt fmt);
  void WriteSubtree(const SgNode &node, bool allProps, int boardSize,
                    SgPropPointFmt fmt);

  SgGameWriter(const SgGameWriter &) = delete;
  SgGameWriter &operator=(const SgGameWriter &) = delete;
};

#endif
