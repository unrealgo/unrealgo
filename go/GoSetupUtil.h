

#ifndef GO_SETUPUTIL_H
#define GO_SETUPUTIL_H

#include <streambuf>
#include "GoBoard.h"
#include "GoSetup.h"
namespace GoSetupUtil {

GoSetup CreateSetupFromStream(std::streambuf& in, int& boardSize);
GoSetup CreateSetupFromString(const std::string& in, int& boardSize);
GoSetup CurrentPosSetup(const GoBoard& bd);

}

#endif

