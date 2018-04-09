
#include <string>
#include "SgUUID.h"
#include "uuid/uuid.h"

void SgUUID::generateUUID(std::string &out) {
  uuid_t uuid;
  char uuid_str[64];
  uuid_generate_random(uuid);
  uuid_unparse(uuid, uuid_str);
  out = uuid_str;
}