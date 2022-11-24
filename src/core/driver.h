#ifndef DLB_DRIVER_H
#define DLB_DRIVER_H

#ifdef POWER_OF_D
#define DISABLE_MIGRATION
#endif

#include <chrono>
#include <climits>
#include <memory>
#include <numeric>
#include <numa.h>

#include <boost/log/trivial.hpp>

#include "../../concurrentqueue/concurrentqueue.h"
#include "../../eRPC/src/rpc.h"
#include "../../eRPC/src/util/numautils.h"

#include "../util/config.h"
#include "../util/helper.h"
#include "../util/ringMMIO.h"
#include "../util/distributions.h"

#include "context.h"
#include "defaults.h"
#include "endpointAddr.h"
#include "message.h"

// Signature of task generation function
using TaskGenerationFunc =
    std::function<void(char *data, size_t &len, uint8_t &type)>;

class Driver {
public:

  Driver(EndPointAddr addr, Config c, TaskGenerationFunc func);

  static void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void *);
  static void receiveResult(erpc::ReqHandle *req_handle, void *context);
  static void receiveReadyNotification(erpc::ReqHandle *req_handle, void *context);

  [[noreturn]] void startRunning();
  int getDriverID();

private:
  //  Task generation helper functions
  int sampleRecipient();

  // Stat function(s)
  void collectStats() const;

  // eRPC functions
  void createSenderReceiverRPC();
  void establishConnections();
  size_t getResource();

  // Driver info
  const int driverID;
  const Config config;
  const EndPointAddr address;
  size_t numServers;

  // Task generation
  TaskGenerationFunc taskGenerationFunc;

  // eRPC info
  erpc::Nexus nexus;
  erpc::Rpc<erpc::CTransport> *senderReceiverRPC;
  Context senderReceiverContext;
  int *sessionMap;
  EndPointAddr *serversEndPoints;
  uint64_t numSMResponses = 0;
  uint64_t numReadyNotification = 0;

  // Random number generator
  Exponential expRanGenerator;
  UniformInt uIntRanGenerator;

  // Stat
  uint64_t startRecording;
  uint64_t numSubmittedTasks = 0;
  uint64_t numSentTasks = 0;
  uint64_t numReceivedResponses = 0;
  uint64_t numOverMigratedTasks = 0;
  uint64_t numFailedTasks = 0;
  unsigned createdTaskCounter = 0;
  int maxMigrations = 0;
  RingMMIO<uint64_t> latencyHistory;


#ifdef FLUCTUATE_LOAD
  int fluctuationCounter = 0;
  std::vector<std::discrete_distribution<>> departureShares;
  std::mt19937 uRanGenerator;
#endif

};

#endif // DLB_DRIVER_H
