//----------------------------------------------------------------------------
/** @file SgRandom.h
    Random numbers. */
//----------------------------------------------------------------------------

#ifndef SG_RANDOM_H
#define SG_RANDOM_H

#include <algorithm>
#include <list>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/gamma_distribution.hpp>
#include <boost/random/variate_generator.hpp>

//----------------------------------------------------------------------------

/** Random number generator.
    Uses a Mersenne Twister, because this is faster than std::rand() and
    game playing programs usually need faster random numbers more than
    high quality ones. All random generators are internally registered to
    make it possible to change the random seed for all of them.

    SgRandom is thread-safe (w.r.t. different instances) after construction
    (the constructor is not thread-safe, because it uses a global variable
    for registration). */
class SgRandom {
 public:
  SgRandom();
  ~SgRandom();
  static void SetSeed(int seed);
  static int Seed();

  float Float(float range);
  float Float_01();
  unsigned int Int();

  int Int(int range);
  std::size_t Int(std::size_t range);

  int SmallInt(int range);
  std::size_t SmallInt(std::size_t range);
  int Range(int min, int max);
  unsigned int Max();

  unsigned int PercentageThreshold(int percentage);
  bool RandomEvent(unsigned int threshold);
  void generateDirichlet(double out_[], float alpha, int length);
  void generateDirichlet(double out_[], int length);

 private:
  struct GlobalData {
    boost::mt19937::result_type m_seed;
    std::list<SgRandom *> m_allGenerators;
    GlobalData();
  };
  void SetRandomSeed();
  static GlobalData &GetGlobalData();
  static void SetGlobalRandomSeed();

  boost::mt19937 m_generator;
  boost::gamma_distribution<> m_pdf;
  boost::variate_generator<boost::mt19937 &, boost::gamma_distribution<> > m_gamma_generator;
  /*	Random number generator for Float() and Float_01().
    Uses m_generator internally.
    See http://www.boost.org/doc/libs/1_39_0/libs/random/
      random-distributions.html#uniform_01
*/
  boost::uniform_01<boost::mt19937, float> m_floatGenerator;
};

inline float SgRandom::Float_01() {
  return m_floatGenerator();
}

inline float SgRandom::Float(float range) {
  float v = m_floatGenerator() * range;
  DBG_ASSERT(v <= range);
  // @todo: should be < range? Worried about rounding issues.
  return v;
}

inline unsigned int SgRandom::Int() {
  return m_generator();
}

inline int SgRandom::Int(int range) {
  DBG_ASSERT(range > 0);
  DBG_ASSERT(static_cast<unsigned int>(range) <= SgRandom::Max());
  int i = Int() % range;
  DBG_ASSERTRANGE(i, 0, range - 1);
  return i;
}

inline std::size_t SgRandom::Int(std::size_t range) {
  DBG_ASSERT(range <= SgRandom::Max());
  std::size_t i = Int() % range;
  DBG_ASSERT(i < range);
  return i;
}

inline unsigned int SgRandom::Max() {
  return m_generator.max();
}

inline unsigned int SgRandom::PercentageThreshold(int percentage) {
  return (m_generator.max() / 100) * percentage;
}

inline bool SgRandom::RandomEvent(unsigned int threshold) {
  return Int() <= threshold;
}

inline int SgRandom::Range(int min, int max) {
  return min + Int(max - min);
}

inline int SgRandom::SmallInt(int range) {
  DBG_ASSERT(range > 0);
  DBG_ASSERT(range <= (1 << 16));
  int i = ((Int() & 0xffff) * range) >> 16;
  DBG_ASSERTRANGE(i, 0, range - 1);
  return i;
}

inline std::size_t SgRandom::SmallInt(std::size_t range) {
  DBG_ASSERT(range <= (1 << 16));
  std::size_t i = ((Int() & 0xffff) * range) >> 16;
  DBG_ASSERT(i < range);
  return i;
}

#endif // SG_RANDOM_H
