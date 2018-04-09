

#include "platform/SgSystem.h"
#include "GoBoardUpdater.h"

#include "GoBoard.h"
#include "GoSetupUtil.h"
#include "SgNode.h"
#include "board/SgUtil.h"

namespace {

SgEmptyBlackWhite GetPlayer(const SgNode* node) {
  if (!node->HasProp(SG_PROP_PLAYER))
    return SG_EMPTY;
  auto* prop = dynamic_cast<SgPropPlayer*>(node->Get(SG_PROP_PLAYER));
  return prop->Value();
}

}

void GoBoardUpdater::Update(const SgNode* start, GoBoard& bd) {
  DBG_ASSERT(start != nullptr);
  m_nodes.clear();
  while (start != 0) {
    m_nodes.push_back(start);
    start = start->Father();
  }
  const SgNode* root = m_nodes[m_nodes.size() - 1];
  int size = GO_DEFAULT_SIZE;
  auto* boardSize = dynamic_cast<SgPropInt*>(root->Get(SG_PROP_SIZE));
  if (boardSize) {
    size = boardSize->Value();
    DBG_ASSERT(SgUtil::InRange(size, GO_MIN_SIZE, GO_MAX_SIZE));
  }
  bd.Init(size);

  for (auto it = m_nodes.rbegin();
       it != m_nodes.rend(); ++it) {
    const SgNode* node = *it;
    SgEmptyBlackWhite player = GetPlayer(node);
    if (node->HasProp(SG_PROP_ADD_EMPTY)
        || node->HasProp(SG_PROP_ADD_BLACK)
        || node->HasProp(SG_PROP_ADD_WHITE)) {
      GoSetup setup = GoSetupUtil::CurrentPosSetup(bd);
      if (player != SG_EMPTY)
        setup.m_player = player;
      if (node->HasProp(SG_PROP_ADD_BLACK)) {
        auto* prop =
            dynamic_cast<SgPropAddStone*>(node->Get(SG_PROP_ADD_BLACK));
        const SgVector<GoPoint>& addBlack = prop->Value();
        for (SgVectorIterator<GoPoint> it2(addBlack); it2; ++it2) {
          GoPoint p = *it2;
          setup.m_stones[SG_WHITE].Exclude(p);
          if (!setup.m_stones[SG_BLACK].Contains(p))
            setup.AddBlack(p);
        }
      }
      if (node->HasProp(SG_PROP_ADD_WHITE)) {
        auto* prop =
            dynamic_cast<SgPropAddStone*>(node->Get(SG_PROP_ADD_WHITE));
        const SgVector<GoPoint>& addWhite = prop->Value();
        for (SgVectorIterator<GoPoint> it2(addWhite); it2; ++it2) {
          GoPoint p = *it2;
          setup.m_stones[SG_BLACK].Exclude(p);
          if (!setup.m_stones[SG_WHITE].Contains(p))
            setup.AddWhite(p);
        }
      }
      if (node->HasProp(SG_PROP_ADD_EMPTY)) {
        auto* prop =
            dynamic_cast<SgPropAddStone*>(node->Get(SG_PROP_ADD_EMPTY));
        const SgVector<GoPoint>& addEmpty = prop->Value();
        for (SgVectorIterator<GoPoint> it2(addEmpty); it2; ++it2) {
          GoPoint p = *it2;
          setup.m_stones[SG_BLACK].Exclude(p);
          setup.m_stones[SG_WHITE].Exclude(p);
        }
      }
      bd.Init(bd.Size(), setup);
    } else if (player != SG_EMPTY)
      bd.SetToPlay(player);
    if (node->HasProp(SG_PROP_MOVE)) {
      auto* prop =
          dynamic_cast<SgPropMove*>(node->Get(SG_PROP_MOVE));
      GoPoint p = prop->Value();
      if (p == GO_PASS || !bd.Occupied(p))
        bd.Play(p, prop->Player());
    }
  }
}
