

#ifndef GO_SEARCH_H
#define GO_SEARCH_H

#include "SgABSearch.h"

class GoBoard;
class GoSearch : public SgABSearch {
 public:
  GoSearch(GoBoard& board, SgSearchHashTable* hash);
  GoBoard& Board();
  const GoBoard& Board() const;
  bool CheckDepthLimitReached() const;
  bool EndOfGame() const;
  bool Execute(GoMove move, int* delta, int depth);
  SgHashCode GetHashCode() const;
  SgBlackWhite GetToPlay() const;
  void SetToPlay(SgBlackWhite toPlay);
  std::string MoveString(GoMove move) const;
  void TakeBack();

 private:
  GoBoard& m_board;
};

inline GoBoard& GoSearch::Board() {
  return m_board;
}

inline const GoBoard& GoSearch::Board() const {
  return m_board;
}

#endif
