
#ifndef GO_BOARDRESTORER_H
#define GO_BOARDRESTORER_H

#include "GoBoard.h"

class GoBoardRestorer {
 public:
  explicit GoBoardRestorer(GoBoard& bd);
  ~GoBoardRestorer();

 private:
  typedef GoArrayList<GoPlayerMove, GO_MAX_NUM_MOVES> MoveList;
  GoBoard& m_bd;
  int m_size;
  GoRules m_rules;
  MoveList m_moves;
};

#endif
