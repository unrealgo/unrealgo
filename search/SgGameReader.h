//----------------------------------------------------------------------------

#ifndef SG_GAMEREADER_H
#define SG_GAMEREADER_H

#include <bitset>
#include <map>
#include <vector>
#include "SgProp.h"
#include "lib/SgVector.h"

class SgNode;
class SgGameReader {
 public:
  enum WarningFlags {
    INVALID_BOARDSIZE,
    PROPERTY_WITHOUT_VALUE,
    NU_WARNING_FLAGS
  };
  typedef std::bitset<NU_WARNING_FLAGS> Warnings;
  SgGameReader(std::istream& in, int defaultSize = 19);
  Warnings GetWarnings() const;
  void PrintWarnings(std::ostream& out) const;
  SgNode* ReadGame();
  void ReadGames(SgVectorOf<SgNode>* rootList);

 private:
  typedef std::map<std::string, std::vector<std::string> > RawProperties;
  std::istream& m_in;
  const int m_defaultSize;
  int m_fileFormat;
  Warnings m_warnings;
  SgGameReader(const SgGameReader&);
  SgGameReader& operator=(const SgGameReader&);
  static bool GetIntProp(const RawProperties& properties,
                         const std::string& label, int& value);
  void HandleProperties(SgNode* node, const RawProperties& properties,
                        int& boardSize, SgPropPointFmt& fmt);
  SgNode* ReadGame(bool resetWarnings);
  std::string ReadLabel(int c);
  SgNode* ReadSubtree(SgNode* node, int boardSize, SgPropPointFmt fmt);
  bool ReadValue(std::string& value);
};

inline SgGameReader::Warnings SgGameReader::GetWarnings() const {
  return m_warnings;
}

inline SgNode* SgGameReader::ReadGame() {
  return ReadGame(true);
}

#endif
