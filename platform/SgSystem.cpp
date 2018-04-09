
#include "SgSystem.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <boost/static_assert.hpp>
#include "boost/filesystem.hpp"

namespace SgDeterministic {
static bool g_deterministicMode = false;
void SetDeterministicMode(bool flag) {
  g_deterministicMode = flag;
}
bool IsDeterministicMode() {
  return g_deterministicMode;
}

} // namespace SgDeterministic


namespace {
/** Check that no old version of Boost libraries is used. */
#if defined(BOOST_FILESYSTEM_VERSION)
BOOST_STATIC_ASSERT(BOOST_FILESYSTEM_VERSION >= 3);
#endif

volatile bool g_force_abort = false;
std::list<AssertionHandlerInterface*> &AssertionHandlers() {
  static std::list<AssertionHandlerInterface *> s_assertionHandlers;
  return s_assertionHandlers;
}

} // namespace


AssertionHandlerInterface::AssertionHandlerInterface() {
  AssertionHandlers().push_back(this);
}

AssertionHandlerInterface::~AssertionHandlerInterface() {
  AssertionHandlers().remove(this);
}

//----------------------------------------------------------------------------

#ifndef NDEBUG

/** Set the shell variable SMARTGAME_ASSERT_CONTINUE to drop into the debugger
    instead of aborting the program whenever an DBG_ASSERT fails */
static bool s_assertContinue = (std::getenv("SMARTGAME_ASSERT_CONTINUE") != 0);

void SgHandleAssertion(const char *expr, const char *file, int line) {
  /** Set a breakpoint on the next line to drop into the debugger */
  std::cerr << "Assertion failed "
            << file << ':' << line << ": " << expr << '\n';
  for_each(AssertionHandlers().begin(), AssertionHandlers().end(),
           std::mem_fn(&AssertionHandlerInterface::Run));
  if (!s_assertContinue)
    abort();
}

#endif


void SetForceAbort(bool force_abort) {
  g_force_abort = force_abort;
}

bool ForceAbort() {
  return g_force_abort;
}
