

#include "platform/SgSystem.h"
#include "GoBoardSynchronizer.h"

#include "GoBoard.h"

GoBoardSynchronizer::GoBoardSynchronizer(const GoBoard& publisher)
    : m_publisher(publisher),
      m_subscriber(0),
      m_Rule(m_publisher.Rules()) {}

GoBoardSynchronizer::~GoBoardSynchronizer() {}

void GoBoardSynchronizer::SetSubscriber(GoBoard& subscriber) {
  DBG_ASSERT(!m_subscriber);
  m_subscriber = &subscriber;
}

int GoBoardSynchronizer::FindNuCommon() const {
  const int nuPublisher = m_publisher.MoveNumber();
  const int nuSubscriber = m_subscriber->MoveNumber();
  int i;
  for (i = 0; i < nuPublisher; ++i)
    if (i >= nuSubscriber || m_publisher.Move(i) != m_subscriber->Move(i))
      break;
  return i;
}

void GoBoardSynchronizer::ExecuteSubscriber(const GoPlayerMove& move) {
  PrePlay(move);
  m_subscriber->Play(move);
  OnPlay(move);
}

void GoBoardSynchronizer::OnBoardChange() {}

void GoBoardSynchronizer::PrePlay(const GoPlayerMove& move) {
  SuppressUnused(move);
}

void GoBoardSynchronizer::OnPlay(const GoPlayerMove& move) {
  SuppressUnused(move);
}

void GoBoardSynchronizer::PreUndo() {}

const GoRules& GoBoardSynchronizer::Rules() {
  return m_Rule;
}

void GoBoardSynchronizer::OnUndo() {}

void GoBoardSynchronizer::UpdateFromInit() {
  m_subscriber->Init(m_publisher.Size(), m_publisher.Setup());
  OnBoardChange();
  const int nuPublisher = m_publisher.MoveNumber();
  for (int i = 0; i < nuPublisher; ++i)
    ExecuteSubscriber(m_publisher.Move(i));
}

void GoBoardSynchronizer::UpdateIncremental() {
  const int nuCommon = FindNuCommon();
  const int nuSubscriber = m_subscriber->MoveNumber();
  const int nuUndo = nuSubscriber - nuCommon;
  for (int i = 0; i < nuUndo; ++i) {
    DBG_ASSERT(m_subscriber->MoveNumber() > 0);
    PreUndo();
    m_subscriber->Undo();
    OnUndo();
  }
  const int nuPublisher = m_publisher.MoveNumber();
  for (int i = nuCommon; i < nuPublisher; ++i)
    ExecuteSubscriber(m_publisher.Move(i));
}

void GoBoardSynchronizer::UpdateSubscriber() {
  if (m_subscriber == 0)
    return;
  m_subscriber->Rules() = m_publisher.Rules();
  if (m_publisher.Size() != m_subscriber->Size()
      || m_publisher.Setup() != m_subscriber->Setup())
    UpdateFromInit();
  else
    UpdateIncremental();
  UpdateWhoToPlay();
  DBG_ASSERT(m_publisher.GetHashCode() == m_subscriber->GetHashCode());
}

void GoBoardSynchronizer::UpdateWhoToPlay() {
  if (m_publisher.ToPlay() != m_subscriber->ToPlay())
    m_subscriber->SetToPlay(m_publisher.ToPlay());
}
