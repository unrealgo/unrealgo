
#ifndef UNREALGO_BOARDCONFIG_H
#define UNREALGO_BOARDCONFIG_H

#include <boost/static_assert.hpp>

const int GO_MIN_SIZE = 2;

#ifndef GO_DEFINE_MAX_SIZE
static const int GO_MAX_SIZE = 19;
#else
BOOST_STATIC_ASSERT(GO_DEFINE_MAX_SIZE >= GO_MIN_SIZE);
BOOST_STATIC_ASSERT(GO_DEFINE_MAX_SIZE <= 25);
static const int GO_MAX_SIZE = GO_DEFINE_MAX_SIZE;
#endif

typedef int GoPoint;
typedef GoPoint GoMove;

const GoMove GO_NULLMOVE = -1;
const GoMove GO_COUPONMOVE = -2;
const GoMove GO_COUPONMOVE_VIRTUAL = -3;
const GoMove UCT_RESIGN = -4;

const GoPoint GO_ENDPOINT = 0;
const int GO_MINPOINT = 0;
const int GO_MAXPOINT = GO_MAX_SIZE * GO_MAX_SIZE + 3 * (GO_MAX_SIZE + 1);
const int GO_MAX_ONBOARD = GO_MAX_SIZE * GO_MAX_SIZE;
const int GO_MAX_MOVES = GO_MAX_ONBOARD + 1;
const int GO_WEST_EAST = 1;
const int GO_NORTH_SOUTH = GO_MAX_SIZE + 1;
const GoPoint GO_NULLPOINT = GO_NULLMOVE;
const GoMove GO_PASS = GO_MAXPOINT + 1;


#endif //UNREALGO_BOARDCONFIG_H
