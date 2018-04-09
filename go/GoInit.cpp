

#include "platform/SgSystem.h"
#include "GoInit.h"

#include "GoRegion.h"
#include "platform/SgException.h"
#include "SgInit.h"
#include "SgProp.h"

namespace {
bool s_initialized = false;

void RegisterGoProps() {
  SgProp* moveProp = new SgPropMove(0);
  SG_PROP_MOVE_BLACK =
      SgProp::Register(moveProp, "B",
                       SG_PROPCLASS_MOVE + SG_PROPCLASS_BLACK);
  SG_PROP_MOVE_WHITE
      = SgProp::Register(moveProp, "W",
                         SG_PROPCLASS_MOVE + SG_PROPCLASS_WHITE);
}
}

void GoFinish() {
  GoRegion::Finish();
  s_initialized = false;
}

void GoInit() {
  SgInitCheck();
  RegisterGoProps();
  s_initialized = true;
}

void GoInitCheck() {
  if (!s_initialized)
    throw SgException("GoInit not called");
}

