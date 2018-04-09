
#include "GoBoardHistory.h"

void GoBoardHistory::SetFromBoard(const GoBoard& bd) {
  board_size = bd.Size();
  rules = bd.Rules();
  setup = bd.Setup();
  moves.Clear();
  for (int i = 0; i < bd.MoveNumber(); ++i)
    moves.PushBack(bd.Move(i));
  to_play = bd.ToPlay();
}

bool GoBoardHistory::IsAlternatePlayFollowUpOf(const GoBoardHistory& other,
                          std::vector<GoPoint>& sequence) const {
  if (board_size != other.board_size
      || rules != other.rules
      || setup != other.setup
      || moves.Length() < other.moves.Length())
    return false;
  for (int i = 0; i < other.moves.Length(); ++i)
    if (moves[i] != other.moves[i])
      return false;
  sequence.clear();
  SgBlackWhite toPlay = other.to_play;
  for (int i = other.moves.Length(); i < moves.Length(); ++i) {
    GoPlayerMove m = moves[i];
    if (m.Color() != toPlay)
      return false;
    sequence.push_back(m.Point());
    toPlay = SgOppBW(toPlay);
  }
  if (toPlay != this->to_play)
    return false;
  return true;
}

bool
GoBoardHistory::SequenceToCurrent(const GoBoard& bd,
                                  std::vector<GoPoint>& sequence) const {
  GoBoardHistory currentPosition;
  currentPosition.SetFromBoard(bd);
  return currentPosition.IsAlternatePlayFollowUpOf(*this, sequence);
}
