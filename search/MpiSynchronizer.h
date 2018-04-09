
#ifndef SG_MPISYNCHRONIZER_H
#define SG_MPISYNCHRONIZER_H

#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include "board/GoPoint.h"
#include "UctValue.h"

class UctSearch;
class UctThreadState;
struct UctGameInfo;

class MpiSynchronizer {
 public:
  virtual ~MpiSynchronizer();
  virtual std::string ToNodeFilename(const std::string &filename) const = 0;
  virtual bool IsRootProcess() const = 0;
  virtual void OnStartSearch(UctSearch &search) = 0;
  virtual void OnEndSearch(UctSearch &search) = 0;
  virtual void OnThreadStartSearch(UctSearch &search,
                                   UctThreadState &state) = 0;
  virtual void OnThreadEndSearch(UctSearch &search,
                                 UctThreadState &state) = 0;
  virtual void OnSearchIteration(UctSearch &search,
                                 UctValueType gameNumber,
                                 std::size_t threadId,
                                 const UctGameInfo &info) = 0;
  virtual void OnStartPonder() = 0;
  virtual void OnEndPonder() = 0;
  virtual void WriteStatistics(std::ostream &out) const = 0;
  virtual void SynchronizeUserAbort(bool &flag) = 0;
  virtual void SynchronizePassWins(bool &flag) = 0;
  virtual void SynchronizeEarlyPassPossible(bool &flag) = 0;
  virtual void SynchronizeMove(GoMove &move) = 0;
  virtual void SynchronizeValue(UctValueType &value) = 0;
  virtual void SynchronizeSearchStatus(UctValueType &value, bool &earlyAbort,
                                       UctValueType &rootMoveCount) = 0;
  virtual bool CheckAbort() = 0;
};
typedef boost::shared_ptr<MpiSynchronizer> MpiSynchronizerHandle;


class NullMpiSynchronizer : public MpiSynchronizer {
 public:
  NullMpiSynchronizer();
  virtual ~NullMpiSynchronizer();
  static MpiSynchronizerHandle Create();
  virtual std::string ToNodeFilename(const std::string &filename) const;
  virtual bool IsRootProcess() const;
  virtual void OnStartSearch(UctSearch &search);
  virtual void OnEndSearch(UctSearch &search);
  virtual void OnThreadStartSearch(UctSearch &search,
                                   UctThreadState &state);
  virtual void OnThreadEndSearch(UctSearch &search,
                                 UctThreadState &state);
  virtual void OnSearchIteration(UctSearch &search, UctValueType gameNumber,
                                 std::size_t threadId, const UctGameInfo &info);
  virtual void OnStartPonder();
  virtual void OnEndPonder();
  virtual void WriteStatistics(std::ostream &out) const;
  virtual void SynchronizeUserAbort(bool &flag);
  virtual void SynchronizePassWins(bool &flag);
  virtual void SynchronizeEarlyPassPossible(bool &flag);
  virtual void SynchronizeMove(GoMove &move);
  virtual void SynchronizeValue(UctValueType &value);
  virtual void SynchronizeSearchStatus(UctValueType &value, bool &earlyAbort,
                                       UctValueType &rootMoveCount);
  virtual bool CheckAbort();

};

#include "UctSearch.h"

#endif
