
#ifndef SG_TIME_H
#define SG_TIME_H

#include <string>

enum SgTimeMode {
  SG_TIME_CPU,
  SG_TIME_REAL,
  SG_TIME_NONE
};


namespace SgTime {
// Format time as MM:SS
std::string Format(double time, bool minsAndSecs = true);

double Get();

double Get(SgTimeMode mode);
SgTimeMode DefaultMode();
void SetDefaultMode(SgTimeMode mode);

/** Get today's date in a format compatible with the DT property
    of the SGF standard. */
std::string TodaysDate();
std::string Time2String();
}

#endif // SG_TIME_H
