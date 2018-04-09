
#ifndef SG_NODEUTIL_H
#define SG_NODEUTIL_H

#include "SgProp.h"

class SgNode;
class SgTimeRecord;

namespace SgNodeUtil {
int GetMoveNumber(const SgNode *node);
void RemovePropInSubtree(SgNode &root, SgPropID id);
void UpdateTime(SgTimeRecord &time, const SgNode *node);
}

#endif
