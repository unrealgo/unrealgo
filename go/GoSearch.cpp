

#include "platform/SgSystem.h"
#include "GoSearch.h"

#include <sstream>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "SgNode.h"
#include "board/GoPoint.h"
#include "board/SgWrite.h"

GoSearch::GoSearch(GoBoard& board, SgSearchHashTable* hash)
    : SgABSearch(hash),
      m_board(board) {
  SetOpponentBest(true);
}

bool GoSearch::CheckDepthLimitReached() const {
  return false;
}

bool GoSearch::EndOfGame() const {
  return PrevMove() == GO_PASS && PrevMove2() == GO_PASS;
}

bool GoSearch::Execute(GoMove move, int* delta, int depth) {
  SuppressUnused(delta);
  SuppressUnused(depth);
  GoBoard& bd = Board();
  SgBlackWhite toPlay = bd.ToPlay();
  if (!GoBoardUtil::PlayIfLegal(bd, move, toPlay))
    return false;
  return true;
}

SgHashCode GoSearch::GetHashCode() const {
  return Board().GetHashCodeInclToPlay();
}

SgBlackWhite GoSearch::GetToPlay() const {
  return Board().ToPlay();
}

std::string GoSearch::MoveString(GoMove move) const {
  std::ostringstream buffer;
  buffer << GoWritePoint(move);
  return buffer.str();
}

void GoSearch::SetToPlay(SgBlackWhite toPlay) {
  Board().SetToPlay(toPlay);
}

void GoSearch::TakeBack() {
  Board().Undo();
}

