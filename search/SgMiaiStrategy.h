
#ifndef SG_MIAISTRATEGY_H
#define SG_MIAISTRATEGY_H

#include <utility>
#include "board/GoBlackWhite.h"
#include "board/GoBWArray.h"
#include "lib/SgHash.h"
#include "board/GoPointSet.h"
#include "SgStrategy.h"
#include "lib/SgVector.h"

typedef std::pair<GoPoint, GoPoint> SgMiaiPair;


namespace SgMiaiPairUtil {
GoPoint Other(const SgMiaiPair &pair, GoPoint p);
}

class SgMiaiStrategy
    : public SgStrategy {
 public:

  SgMiaiStrategy(SgBlackWhite player)
      : SgStrategy(player),
        m_failed(false) {}


  void AddPair(const SgMiaiPair &miaiPair);


  void SetStrategy(const SgVector<SgMiaiPair> &miaiStrategies) {
    //DBG_ASSERT(m_miaiStrategies.IsEmpty());
    m_miaiStrategies = miaiStrategies;
  }


  const SgVector<SgMiaiPair> &MiaiStrategies() const {
    return m_miaiStrategies;
  }


  bool HasOverlappingMiaiPairs() const;

  GoPointSet Dependency() const;

  SgStrategyStatus Status() const;

  const SgVector<GoPoint> &OpenThreats() const;

  GoPoint OpenThreatMove() const;

  void ExecuteMove(GoPoint p, SgBlackWhite player);

  void UndoMove();

  void Clear();

  void Write(std::ostream &stream) const;

 private:

  void StrategyFailed();

  SgVector<SgMiaiPair> m_miaiStrategies;

  SgVector<GoPoint> m_openThreats;

  bool m_failed;
};

#endif // SG_MIAISTRATEGY_H
