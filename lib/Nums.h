
#ifndef NUMS_H
#define NUMS_H

#include "lib/SgRandom.h"

template<class T>
T SumOf(T weights[], int len) {
  T sum = 0.f;
  for (int i = 0; i < len; i++)
    sum += weights[i];
  return sum;
}

template<class T>
T MaxOf(T weights[], int len) {
  T max = weights[0];
  for (int i = 1; i < len; i++)
    if (max < weights[i])
      max = weights[i];
  return max;
}

template<class T>
void DivideAllByMax(T *weights, size_t len) {
  T max = MaxOf(weights, len);
  for (size_t i = 0; i < len; i++) {
    weights[i] /= max;
    if (std::isnan(weights[i]))
      weights[i] = 0.0;
  }
}

template<class T>
void Normalize(T weights[], int len) {
  T sum = SumOf<T>(weights, len);
  for (int i = 0; i < len; i++) {
    weights[i] /= sum;
    if (std::isnan(weights[i]))
      weights[i] = 0.0;
  }
}

template<class T>
void CumulativeDf(T weights[], int len) {
  T sum = sumOf(weights, len);
  for (int i = 1; i < len; i++) {
    weights[i] += weights[i - 1];
    weights[i - 1] /= sum;
  }
  weights[len - 1] = 1.0f;
}

template<class T>
T ControlW(T counts[], size_t len, T tau) {
  T exp = 1.0 / tau;
  for (size_t i = 0; i < len; i++) {
    counts[i] = pow(counts[i], exp);
    if (std::isnan(counts[i]))
      counts[i] = 0.0;
  }
  return SumOf<T>(counts, len);
}

template<class T>
void ControlAndNormalize(T *counts, int len, T tau) {
  DivideAllByMax(counts, len);
  T sum = ControlW<T>(counts, len, tau);
  for (int i = 0; i < len; i++) {
    counts[i] /= sum;
    if (std::isnan(counts[i]))
      counts[i] = 0.0;
  }
}

template<class T>
int CumulativeChoose(T weights[], size_t len, T sum, SgRandom &random) {
  T p = random.Float_01() * sum;
  for (size_t i = 0; i < len; i++) {
    p -= weights[i];
    if (p <= 0)
      return i;
  }
  return len - 1;
}

// assume sum(weights) = 1.0
template<class T>
int CumulativeChoose(T weights[], int len) {
  SgRandom rand;
  T p = rand.Float_01() * sumOf(weights, len);
  for (int i = 0; i < len; i++) {
    p -= weights[i];
    if (p <= 0)
      return i;
  }
  return len - 1;
}

#endif //NUMS_H
