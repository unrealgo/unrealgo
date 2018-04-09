
#ifndef SG_ADDITIVEKNOWLEDGE_H
#define SG_ADDITIVEKNOWLEDGE_H

#include "UctValue.h"

class SgAdditiveKnowledge {
 public:
  SgAdditiveKnowledge();

  UctValueType PredictorDecay() const;

  UctValueType PredictorWeight(UctValueType posCount) const;

  void SetPredictorDecay(UctValueType value);

 private:

  UctValueType m_predictorDecay;
};

inline SgAdditiveKnowledge::SgAdditiveKnowledge()
    : m_predictorDecay(5.0f) {}

inline UctValueType SgAdditiveKnowledge::PredictorDecay() const {
  return m_predictorDecay;
}

inline void SgAdditiveKnowledge::SetPredictorDecay(UctValueType value) {
  m_predictorDecay = value;
}

inline UctValueType SgAdditiveKnowledge::PredictorWeight(UctValueType posCount)
const {
  return sqrt(m_predictorDecay / (posCount + m_predictorDecay));
}

#endif
