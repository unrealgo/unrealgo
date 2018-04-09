

#ifndef GO_EYECOUNT_H
#define GO_EYECOUNT_H

#include <ostream>

enum GoEyeStatus {
  EYE_UNKNOWN,
  EYE_NONE,
  EYE_HALF,
  EYE_ONE,
  EYE_ONE_AND_HALF,
  EYE_TWO
};
class GoEyeCount {
 public:
  GoEyeCount()
      : m_minEyes(0),
        m_maxEyes(0),
        m_minPotEyes(0),
        m_maxPotEyes(0),
        m_isLocalSeki(false),
        m_maybeLocalSeki(false) {}

  GoEyeCount(int minEyes, int maxEyes, int minPotEyes, int maxPotEyes)
      : m_minEyes(minEyes),
        m_maxEyes(maxEyes),
        m_minPotEyes(minPotEyes),
        m_maxPotEyes(maxPotEyes),
        m_isLocalSeki(false),
        m_maybeLocalSeki(false) {}

  int MinEyes() const {
    return m_minEyes;
  }

  int MaxEyes() const {
    return m_maxEyes;
  }

  int MinPotEyes() const {
    return m_minPotEyes;
  }

  int MaxPotEyes() const {
    return m_maxPotEyes;
  }

  bool IsLocalSeki() const {
    return m_isLocalSeki;
  }

  bool MaybeLocalSeki() const {
    return m_maybeLocalSeki;
  }

  void Clear() {
    m_minEyes = m_maxEyes = m_minPotEyes = m_maxPotEyes = 0;
  }

  void SetUnknown() {
    m_minEyes = m_minPotEyes = 0;
    m_maxEyes = m_maxPotEyes = 2;
  }

  void SetMinEyes(int eyes) {
    m_minEyes = eyes;
  }

  void SetMaxEyes(int eyes) {
    m_maxEyes = eyes;
  }

  void SetExactEyes(int eyes) {
    m_minEyes = m_maxEyes = eyes;
  }

  void SetMinPotEyes(int eyes) {
    m_minPotEyes = eyes;
  }

  void SetMaxPotEyes(int eyes) {
    m_maxPotEyes = eyes;
  }

  void SetExactPotEyes(int eyes) {
    m_minPotEyes = m_maxPotEyes = eyes;
  }

  void SetEyes(int eyes, int potEyes) {
    SetExactEyes(eyes);
    SetExactPotEyes(potEyes);
  }

  void SetLocalSeki();

  void SetMaybeLocalSeki() {
    m_maybeLocalSeki = true;
  }

  void Normalize();
  void AddIndependent(const GoEyeCount& from);
  void NumericalAdd(const GoEyeCount& from);
  void AddPotential(const GoEyeCount& from);

 private:
  int m_minEyes;
  int m_maxEyes;
  int m_minPotEyes;
  int m_maxPotEyes;
  bool m_isLocalSeki;
  bool m_maybeLocalSeki;
};
std::ostream& operator<<(std::ostream& stream, const GoEyeCount& s);

#endif
