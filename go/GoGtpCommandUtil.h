

#ifndef GO_GTPCOMMANDUTIL_H
#define GO_GTPCOMMANDUTIL_H

#include <cstddef>
#include "GoBoard.h"
#include "GtpEngine.h"
#include "board/GoBlackWhite.h"
#include "board/GoBoardColor.h"
#include "SgGtpUtil.h"
#include "board/GoPoint.h"
#include "board/GoPointArray.h"
#include "lib/SgVector.h"

namespace GoGtpCommandUtil {

SgEmptyBlackWhite EmptyBlackWhiteArg(const GtpCommand& cmd,
                                     std::size_t number);
SgBlackWhite BlackWhiteArg(const GtpCommand& cmd, std::size_t number);
GoPoint EmptyPointArg(const GtpCommand& cmd, std::size_t number,
                      const GoBoard& board);
SgVector<GoPoint> GetHandicapStones(int size, int n);
GoMove MoveArg(const GtpCommand& cmd, std::size_t number,
               const GoBoard& board);
void ParseMultiStoneArgument(GtpCommand& cmd,
                             const GoBoard& board,
                             SgBlackWhite& toPlay,
                             SgBlackWhite& defender,
                             SgVector<GoPoint>& crucial);
GoPoint PointArg(const GtpCommand& cmd, const GoBoard& board);
GoPoint PointArg(const GtpCommand& cmd, std::size_t number,
                 const GoBoard& board);
SgVector<GoPoint> PointListArg(const GtpCommand& cmd, std::size_t number,
                               const GoBoard& board);
SgVector<GoPoint> PointListArg(const GtpCommand& cmd,
                               const GoBoard& board);
template<int SIZE>
void RespondColorGradientData(GtpCommand& cmd,
                              const GoArray<float, SIZE>& data,
                              float minValue,
                              float maxValue,
                              const GoBoard& board);
void RespondNumberArray(GtpCommand& cmd,
                        const GoPointArray<int>& array,
                        int scale,
                        const GoBoard& board);
std::string SortResponseAnalyzeCommands(const std::string& response);
GoPoint StoneArg(const GtpCommand& cmd, std::size_t number,
                 const GoBoard& board);
}

inline SgVector<GoPoint> GoGtpCommandUtil::PointListArg(const GtpCommand& cmd,
                                                        const GoBoard& board) {
  return PointListArg(cmd, 0, board);
}

template<int SIZE>
void GoGtpCommandUtil::RespondColorGradientData(GtpCommand& cmd,
                                                const GoArray<float, SIZE>& data,
                                                float minValue,
                                                float maxValue,
                                                const GoBoard& board) {
  SgColorGradient gr(SgRGB(0, 255, 0), minValue, SgRGB(0, 0, 255), maxValue);
  GoPointArray<std::string> array("\"\"");
  for (GoBoard::Iterator it(board); it; ++it)
    array[*it] = gr.ColorOf(data[*it]).ToString();
  cmd << SgWritePointArray<std::string>(array, board.Size());
}

#endif

