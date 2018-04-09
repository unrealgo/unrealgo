

#include "platform/SgSystem.h"
#include "GoSetupUtil.h"

#include <cstdio>
#include "GoBoard.h"
#include "GoSetup.h"

using GoPointUtil::Pt;
namespace {

bool IsBlackChar(int c) {
  return c == 'x' || c == 'X' || c == '@';
}

bool IsWhiteChar(int c) {
  return c == '0' || c == 'o' || c == 'O';
}

bool IsEmptyChar(int c) {
  return c == '.' || c == '+';
}

bool IsIgnoreChar(int c) {
  return c == ' ' || c == '\t';
}

bool ReadLine(std::streambuf& in, GoSetup& setup, int row, int& currLength) {
  int col = 0;
  for (int c = in.sbumpc(); c != EOF && c != '\n'; c = in.sbumpc()) {
    if (IsBlackChar(c) || IsWhiteChar(c) || IsEmptyChar(c)) {
      ++col;
      if (col > GO_MAX_SIZE)
        return false;
    } else if (!IsIgnoreChar(c))
      throw SgException("bad input data for ReadLine");

    GoPoint p = Pt(col, row);
    if (IsBlackChar(c)) {
      setup.m_stones[SG_BLACK].Include(p);
    } else if (IsWhiteChar(c)) {
      setup.m_stones[SG_WHITE].Include(p);
    }
  }
  currLength = col;
  return currLength > 0;
}

}

GoSetup GoSetupUtil::CreateSetupFromStream(std::streambuf& in, int& boardSize) {
  GoSetup setup;
  boardSize = 0;

  int currLength = 0;
  for (int row = 1; ReadLine(in, setup, row, currLength); ++row) {
    if (currLength != boardSize) {
      if (boardSize == 0)
        boardSize = currLength;
      else
        throw SgException("bad input data for CreateSetupFromStream");
    }
    if (row > boardSize)
      break;
  }
  GoPointSetUtil::Rotate(2, setup.m_stones[SG_BLACK], boardSize);
  GoPointSetUtil::Rotate(2, setup.m_stones[SG_WHITE], boardSize);
  return setup;
}

GoSetup GoSetupUtil::CreateSetupFromString(const std::string& in,
                                           int& boardSize) {
  std::stringbuf inBuf(in);
  return GoSetupUtil::CreateSetupFromStream(inBuf, boardSize);
}

GoSetup GoSetupUtil::CurrentPosSetup(const GoBoard& bd) {
  GoSetup setup;
  setup.m_player = bd.ToPlay();
  for (GoBoard::Iterator it2(bd); it2; ++it2) {
    GoPoint p = *it2;
    if (bd.Occupied(p))
      setup.m_stones[bd.GetColor(p)].Include(p);
  }
  return setup;
}