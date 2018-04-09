
#include "SgInit.h"

#include <iostream>
#include "platform/SgException.h"
#include "SgMemCheck.h"
#include "SgProp.h"

using namespace std;

namespace {
  bool isLibInitialized = false;
}

void SgFini() {
  SgProp::Fini();
  SgMemCheck();
  isLibInitialized = false;
}
void SgInitImpl(bool isDebugBuild) {
#ifndef NDEBUG
  if (!isDebugBuild) {
    cerr <<
         "Incompatible library: search was compiled "
                 "without NDEBUG, but main program with\n";
    abort();
  }
#else
  if (isDebugBuild) {
    cerr << "Incompatible library: search was compiled "
        "with NDEBUG, but main program without\n";
    abort();
  }
#endif

  SgProp::Init();
  isLibInitialized = true;
}

void SgInitCheck() {
  if (!isLibInitialized)
    throw SgException("SgInit not called");
}

