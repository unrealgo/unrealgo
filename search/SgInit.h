
#ifndef SG_INIT_H
#define SG_INIT_H

void SgInitImpl(bool isDebugBuild);

void SgFini();

// initialization of SmartGo module.
inline void SgInit() {
  // This function must be inline, it needs to use the setting of NDEBUG
  // of the user code including this header
#ifndef NDEBUG
  SgInitImpl(true);
#else
  SgInitImpl(false);
#endif
}

void SgInitCheck();

#endif // SG_INIT_H

