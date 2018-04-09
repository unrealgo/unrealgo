
#ifndef GOBOARDHISTORY_H
#define GOBOARDHISTORY_H

#include <vector>
#include "GoBoard.h"

class GoBoardHistory {
 public:
  GoBoardHistory();
  void SetFromBoard(const GoBoard& bd);
  bool IsAlternatePlayFollowUpOf(const GoBoardHistory& other,
                                 std::vector<GoPoint>& sequence) const;
  bool SequenceToCurrent(const GoBoard& bd, std::vector<GoPoint>& sequence)
  const;

 private:
  int board_size;
  GoRules rules;
  GoSetup setup;
  GoArrayList<GoPlayerMove, GO_MAX_NUM_MOVES> moves;
  SgBlackWhite to_play;
};

inline GoBoardHistory::GoBoardHistory()
    : board_size(-1), to_play(SG_BLACK) {}

#endif
