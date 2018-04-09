#ifndef SG_STRATEGY_H
#define SG_STRATEGY_H

#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "lib/SgHash.h"
#include "board/GoPointSet.h"

enum SgStrategyStatus {
  SGSTRATEGY_ACHIEVED,
  SGSTRATEGY_THREATENED,
  SGSTRATEGY_UNKNOWN,
  SGSTRATEGY_FAILED,
  _SGSTRATEGY_COUNT
};
std::ostream &operator<<(std::ostream &stream, SgStrategyStatus s);


class SgStrategy {
 public:
  SgStrategy(SgBlackWhite player);

  virtual ~SgStrategy() {}

  SgBlackWhite Player() const {
    return m_player;
  }

  virtual void Clear();
  virtual GoPointSet Dependency() const = 0;
  virtual SgStrategyStatus Status() const = 0;
  virtual void ExecuteMove(GoMove p, SgBlackWhite player) = 0;
  virtual void UndoMove() = 0;
  virtual void Write(std::ostream &stream) const;

 private:
  SgBlackWhite m_player;
  SgHashCode m_code;
};
std::ostream &operator<<(std::ostream &stream, const SgStrategy &s);

#endif
