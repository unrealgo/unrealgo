
#include "platform/SgSystem.h"
#include "MpiSynchronizer.h"

using namespace std;

MpiSynchronizer::~MpiSynchronizer() {}

NullMpiSynchronizer::NullMpiSynchronizer() {}

NullMpiSynchronizer::~NullMpiSynchronizer() {}

MpiSynchronizerHandle NullMpiSynchronizer::Create() {
  return MpiSynchronizerHandle(new NullMpiSynchronizer());
}

string NullMpiSynchronizer::ToNodeFilename(const string &filename) const {
  return filename;
}

bool NullMpiSynchronizer::IsRootProcess() const {
  return true;
}

void NullMpiSynchronizer::OnStartSearch(UctSearch &search) {
  SuppressUnused(search);
}

void NullMpiSynchronizer::OnEndSearch(UctSearch &search) {
  SuppressUnused(search);
}

void NullMpiSynchronizer::OnThreadStartSearch(UctSearch &search,
                                                UctThreadState &state) {
  SuppressUnused(search);
  SuppressUnused(state);
}

void NullMpiSynchronizer::OnThreadEndSearch(UctSearch &search,
                                              UctThreadState &state) {
  SuppressUnused(search);
  SuppressUnused(state);
}

void NullMpiSynchronizer::OnSearchIteration(UctSearch &search,
                                              UctValueType gameNumber,
                                              std::size_t threadId,
                                              const UctGameInfo &info) {
  SuppressUnused(search);
  SuppressUnused(gameNumber);
  SuppressUnused(threadId);
  SuppressUnused(info);
}

void NullMpiSynchronizer::OnStartPonder() {}

void NullMpiSynchronizer::OnEndPonder() {}

void NullMpiSynchronizer::WriteStatistics(ostream &out) const {
  SuppressUnused(out);
}

void NullMpiSynchronizer::SynchronizeUserAbort(bool &flag) {
  SuppressUnused(flag);
}

void NullMpiSynchronizer::SynchronizePassWins(bool &flag) {
  SuppressUnused(flag);
}

void NullMpiSynchronizer::SynchronizeEarlyPassPossible(bool &flag) {
  SuppressUnused(flag);
}

void NullMpiSynchronizer::SynchronizeMove(GoMove &move) {
  SuppressUnused(move);
}

void NullMpiSynchronizer::SynchronizeValue(UctValueType &value) {
  SuppressUnused(value);
}

void NullMpiSynchronizer::SynchronizeSearchStatus(UctValueType &value,
                                                    bool &earlyAbort,
                                                    UctValueType &rootMoveCount) {
  SuppressUnused(value);
  SuppressUnused(earlyAbort);
  SuppressUnused(rootMoveCount);
}

bool NullMpiSynchronizer::CheckAbort() {
  return false;
}
