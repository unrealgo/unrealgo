
#include "platform/SgSystem.h"
#include "lib/SgRandom.h"

#include <ctime>
#include <functional>
#include "platform/SgDebug.h"
#include "Nums.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/random/gamma_distribution.hpp>
#include <boost/random/variate_generator.hpp>

SgRandom::GlobalData::GlobalData() {
  m_seed = 0;
}

SgRandom::SgRandom() : m_pdf(0.03), m_gamma_generator(m_generator, m_pdf), m_floatGenerator(m_generator) {
  SetRandomSeed();
  GetGlobalData().m_allGenerators.push_back(this);
}

SgRandom::~SgRandom() {
  GetGlobalData().m_allGenerators.remove(this);
}

SgRandom::GlobalData &SgRandom::GetGlobalData() {
  static GlobalData s_data;
  return s_data;
}

int SgRandom::Seed() {
  return GetGlobalData().m_seed;
}

void SgRandom::SetRandomSeed() {
  boost::mt19937::result_type seed = GetGlobalData().m_seed;
  if (seed == 0)
    return;
  m_generator.seed(seed);
}

void SgRandom::SetGlobalRandomSeed() {
  using boost::posix_time::microsec_clock;
  using boost::posix_time::ptime;
  using boost::posix_time::time_duration;
  using namespace boost::gregorian;

  const boost::posix_time::ptime currentTime =
      boost::posix_time::microsec_clock::universal_time();
  time_duration diff = currentTime - ptime(date(2000, Jan, 1));
  GetGlobalData().m_seed =
      static_cast<boost::mt19937::result_type>(
          diff.total_microseconds());
}

void SgRandom::SetSeed(int seed) {
  if (seed < 0) {
    GetGlobalData().m_seed = 0;
    return;
  }
  if (seed == 0)
    SetGlobalRandomSeed();
  else
    GetGlobalData().m_seed = (boost::mt19937::result_type) seed;

  // SgDebug() << "SgRandom::SetRandomSeed: " << GetGlobalData().m_seed << '\n';
  for_each(GetGlobalData().m_allGenerators.begin(),
           GetGlobalData().m_allGenerators.end(),
           std::mem_fn(&SgRandom::SetRandomSeed));
  srand(GetGlobalData().m_seed);
}

void SgRandom::generateDirichlet(double out_[], int length) {
  for (int i = 0; i < length; i++) {
    out_[i] = m_gamma_generator();
  }
  Normalize(out_, length);
}

void SgRandom::generateDirichlet(double out_[], float alpha, int length) {
  boost::gamma_distribution<> pdf(alpha);
  boost::variate_generator<boost::mt19937 &, boost::gamma_distribution<> > generator(m_generator, pdf);

  for (int i = 0; i < length; i++) {
    out_[i] = generator();
  }
  Normalize(out_, length);
}