

#ifndef GO_MOTIVE_H
#define GO_MOTIVE_H

#include <bitset>
#include <iosfwd>

enum GoMotive {

  GO_MOT_NONE,

  GO_MOT_RANDOM,

  GO_MOT_CAPTURE,

  GO_MOT_ESCAPE,

  GO_MOT_CONNECT,

  GO_MOT_CUT,

  GO_MOT_TO_DIVIDE,

  GO_MOT_URGENT,

  GO_MOT_EXPAND,

  GO_MOT_STABILIZE,

  GO_MOT_REDUCE,

  GO_MOT_DEFEND,

  GO_MOT_INVADE,

  GO_MOT_SENTE,

  GO_MOT_FORCED,

  GO_MOT_ONEYE,

  GO_MOT_TWOEYE,

  GO_MOT_YOSE,

  GO_MOT_ATTACK,

  GO_MOT_SAVE,

  GO_MOT_DOUBLEATARI,

  GO_MOT_ATARI,

  GO_MOT_KOTHREAT,

  _GO_NU_MOTIVE
};
typedef std::bitset<_GO_NU_MOTIVE> GoMotives;
std::ostream& operator<<(std::ostream& out, GoMotive motive);
enum GoModifier {
  GO_MOD_NORMAL,

  GO_MOD_VERY_BIG,

  GO_MOD_BIG,

  GO_MOD_SMALL,

  GO_MOD_VERY_SMALL,

  _GO_NU_MODIFIER
};
std::ostream& operator<<(std::ostream& out, GoModifier modifier);

#endif
