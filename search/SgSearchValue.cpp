#include "platform/SgSystem.h"
#include "SgSearchValue.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <math.h>

using namespace std;

string SgSearchValue::ToString(int unitPerPoint) const {
  if (m_value == 0)
    return "0";
  ostringstream stream;
  stream << (m_value > 0 ? "B+" : "W+");
  if (IsEstimate()) {
    if (unitPerPoint == 1)
      stream << (abs(m_value) / unitPerPoint);
    else
      stream << setprecision(1)
             << (float(abs(m_value)) / float(unitPerPoint));
  } else {
    if (KoLevel() != 0)
      stream << "(ko)";
    if (Depth() != 0) {
      stream << " (" << Depth() << " moves)";
    }
  }
  return stream.str();
}

bool SgSearchValue::FromString(const string &s) {
  SuppressUnused(s);
  DBG_ASSERT(false);
  return false;
}

int SgSearchValue::KoLevel() const {
  if (IsEstimate())
    return 0;
  else {
    int level = (abs(m_value) - 1) / SgABSearch::MAX_DEPTH;
    return (MAX_LEVEL - 1) - level;
  }
}
