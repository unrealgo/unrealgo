

#ifndef GO_MOVEEXECUTOR_H
#define GO_MOVEEXECUTOR_H

#include "GoBoardUtil.h"

class GoMoveExecutor {
 public:
  GoMoveExecutor(GoBoard& board, GoPoint move)
      : m_board(board) {
    m_isLegal = GoBoardUtil::PlayIfLegal(m_board, move);
  }

  GoMoveExecutor(GoBoard& board, GoPoint move, SgBlackWhite player)
      : m_board(board) {
    m_isLegal = GoBoardUtil::PlayIfLegal(m_board, move, player);
  }

  ~GoMoveExecutor() {
    if (m_isLegal)
      m_board.Undo();
  }

  bool IsLegal() const {
    return m_isLegal;
  }

  void UndoMove() {
    DBG_ASSERT(m_isLegal);
    m_board.Undo();
    m_isLegal = false;
  }

 private:
  GoBoard& m_board;
  bool m_isLegal;
  GoMoveExecutor(const GoMoveExecutor&);
  GoMoveExecutor& operator=(const GoMoveExecutor&);
};

#endif

