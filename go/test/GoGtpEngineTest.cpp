

#include "platform/SgSystem.h"

#include "GoGtpEngine.h"
#include "GoNodeUtil.h"
#include <boost/test/auto_unit_test.hpp>

namespace {

class TestPlayer
    : public GoPlayer {
 public:
  TestPlayer(const GoBoard& bd);
  GoPoint GenMove(const SgTimeRecord& time, SgBlackWhite toPlay);
};

TestPlayer::TestPlayer(const GoBoard& bd)
    : GoPlayer(bd) {}

GoPoint TestPlayer::GenMove(const SgTimeRecord& time, SgBlackWhite toPlay) {
  SuppressUnused(time);
  SuppressUnused(toPlay);
  return GO_NULLMOVE;
}

void Execute(GoGtpEngine& engine, const std::string& cmd) {
  std::ofstream nullStream;
  engine.ExecuteCommand(cmd, nullStream);
}

BOOST_AUTO_TEST_CASE(GoGtpEngineTest_CmdClearBoard_KomiInGameAfterClearBoard) {
  GoGtpEngine engine;
  Execute(engine, "komi 1");
  Execute(engine, "clear_board");
  const SgNode& root = engine.Game().Root();
  BOOST_CHECK_EQUAL(GoKomi(1), GoNodeUtil::GetKomi(&root));
}

BOOST_AUTO_TEST_CASE(GoGtpEngineTest_CmdKomi) {
  GoGtpEngine engine;
  Execute(engine, "komi 1");
  BOOST_CHECK_EQUAL(GoKomi(1), GoNodeUtil::GetKomi(&engine.Game().Root()));
  Execute(engine, "play b a1");
  Execute(engine, "komi 2");
  BOOST_CHECK_EQUAL(GoKomi(2), GoNodeUtil::GetKomi(&engine.Game().Root()));
  Execute(engine, "clear_board");
  BOOST_CHECK_EQUAL(GoKomi(2), GoNodeUtil::GetKomi(&engine.Game().Root()));
}

BOOST_AUTO_TEST_CASE(GoGtpEngineTest_CmdBoardsize_PlayerBoard) {
  GoGtpEngine engine;
  int size = engine.Board().Size();
  engine.SetPlayer(new TestPlayer(engine.Board()));
  BOOST_CHECK_EQUAL(size, engine.Player().Board().Size());
  int newSize = size;
  DBG_ASSERT(newSize >= GO_MIN_SIZE);
  std::ostringstream cmd;
  cmd << "set_size " << newSize;
  Execute(engine, cmd.str());
  BOOST_CHECK_EQUAL(newSize, engine.Player().Board().Size());
}

}
