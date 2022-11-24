#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
#ifndef DLB_SERVER_H
#define DLB_SERVER_H

#ifdef POWER_OF_D
#define DISABLE_MIGRATION
#endif

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <list>
#include <memory>
#include <numa.h>
#include <queue>
#include <random>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <cassert>

#include <boost/log/trivial.hpp>

#include "../../concurrentqueue/concurrentqueue.h"
#include "../../eRPC/src/rpc.h"
#include "../../eRPC/src/util/numautils.h"

#include "../RL/AC_AVX2/AC.h"
#include "../util/config.h"
#include "../util/distributions.h"
#include "../util/helper.h"
#include "../util/ringMMIO.h"

#include "context.h"
#include "defaults.h"
#include "endpointAddr.h"
#include "message.h"

using TaskExecutionFunc = std::function<void(
    const char *data, size_t len, char *res, size_t &resLen, int taskType)>;


struct Traits : public moodycamel::ConcurrentQueueDefaultTraits
{
	// Use a slightly larger default block size
	static const size_t BLOCK_SIZE = QUEUE_BLOCK_SIZE;

	// Reuse blocks once allocated.
	static const bool RECYCLE_ALLOCATED_BLOCKS = true;
};

class Server {
public:
  Server(const EndPointAddr& address, const Config& c, TaskExecutionFunc *taskExecutionFuncs);

  // get functions
  int inline getServerID() const { return serverID; }

  // Initialization functions
  void addLocalNeighbour(int ID, Server *neighbour);

  // eRPC RPC calls
  static void gatewaySMHandler(int, erpc::SmEventType, erpc::SmErrType, void *);
  static void heartbeatMgrSMHandler(int, erpc::SmEventType, erpc::SmErrType, void *);
  static void enqueueTask(erpc::ReqHandle *req_handle, void *context);
  static void updateLoad(erpc::ReqHandle *req_handle, void *context);

  // Thread functions
  void startRunning();
  [[noreturn]] void startGateway();
  [[noreturn]] void startHeartbeatMgr();
  [[noreturn]] void startWorker(int workerID);

  void setTaskExecutionFuncs(TaskExecutionFunc *);
  int getNumWorkers() {return numWorkers;};

private:
  void spawnThreads();
  void joinThreads();

  // Gateway thread
  void createGatewayRPC();
  void establishGatewayConnections();
  void checkInbox();
  void drainOutbox();
  void handleTaskEnqueue(Message *message);
  void processOutboxPackets(size_t numberOfMessages);

  // Policy functions
  void initPolicies();
  int consultMigrationPolicy(uint8_t policyIndex);

  // Heart-beater
  void createHeartbeatMgrRPC();
  void putBackHeartbeatMgrResource(size_t id);
  void establishHeartbeatMgrConnections();
  void broadcastMessageToNeighbours(Message &message, int rpcType);
  void broadcastLoadToNeighbours();

  // State and reward functions
  float normalizeLoad(float load) const;
  void calculateServerLoad();
  void printStats();

  // Reward
  float globalReward(float *state, uint64_t numOfMigratedTasks,
                     uint64_t numOfStolenTasks, uint64_t numOfFailedTasks,
                     uint64_t numOfFailedStealingAttempts);

  // Server info
  const int serverID;
  const EndPointAddr address;
  const int numWorkers;
  const int numNeighbours;
  int numLocalNeighbours = 0;
  const int numPeers;
  const Config config;

  // Queues
  const int maxOnFlyMessages;
  const int perWorkerTaskQueueSize;
  const int taskQueueCapacity;
  const int outboxCapacity;
  const int outboxDrainMaxBulkSize;
  const int inboxCapacity;
  const int inboxDrainMaxBulkSize;
  moodycamel::ConcurrentQueue<Message, Traits> *taskQueue;
  moodycamel::ConcurrentQueue<Packet> *outbox;
  moodycamel::ConcurrentQueue<Message> *inbox;
  moodycamel::ProducerToken *taskQueueProducerToken;
  moodycamel::ConsumerToken *outboxConsumerToken;
  moodycamel::ConsumerToken *inboxConsumerToken;
  size_t *outboxContextBulkIndices;
  Packet *outboxBulkBuffer;
  Message *inboxBulkBuffer;
  float taskQueueSize = 0.0f;
  const float taskQueueSizeExpAvgCoef;

  // eRPC info
  erpc::Nexus nexus;
  erpc::Rpc<erpc::CTransport> *gatewayRPC{};
  erpc::Rpc<erpc::CTransport> *heartbeatMgrRPC{};

  int *gatewaySessionMap;
  Context gatewayContext;
  uint64_t gatewaySMResponses = 0;

  int *heartbeatMgrSessionMap;
  Context heartbeatMgrContext;
  uint64_t heartbeatMgrSMResponses = 0;

  // Threads info
  std::thread gatewayThread;
  std::thread heartbeatMgrThread;
  std::thread *workerThreads;
  TaskExecutionFunc *taskExecutionFuncs;

  // Neighbours
  Server **localNeighbours;
  EndPointAddr *peersEndPoints;

  // Random number generator
  Uniform uRanGenerator;
  UniformInt uIntRanGenerator;
  UniformInt uIntRanGenerator2;

  // Stats
  float *systemState;
  RingMMIO<float> stateHistory;

  // Admission policy guard
  std::atomic<uint8_t> policyAccessGuard{};

  float serverLoad = 0.0;
  const float normalizationFactor;
  float arrivalRate = 0;

  const int numTaskTypes;

  uint64_t *receivedTasksPerType;
  uint64_t *admittedTasksPerType;
  uint64_t *executedTasksPerType;
  uint64_t *migratedTasksPerType;
  uint64_t *execTimePerTypeNs;
  uint64_t execTimeNs = 0;


  uint64_t receivedTasks = 0;
  uint64_t admittedTasks = 0;
  uint64_t executedTasks = 0;
  uint64_t migratedTasks = 0; // including stolen tasks
  uint64_t stolenTasks = 0;
  uint64_t failedOnFullQueue = 0;
  uint64_t maxServerQueueSize = 0;

  // reward calculation
  const float migrationCost;
  const float stealingCost;
  const float failingCost;
  const float failedStealingCost;
  float imbalanceExpAvg = 0;
  float previousImbalance = 0;
  const float imbalanceExpAvgCoef;
  float rewardExpAvg = 0.0f;
  const float rewardExpAvgCoef;
  RingMMIO<float> rewardHistory;

#ifdef DLB

public:

  static void consensus(erpc::ReqHandle *req_handle, void *context);
  static void dequeueTask(erpc::ReqHandle *req_handle, void *context);

private:

  void handleTaskDequeue(Message *message);

  // AC functions
  void updateACParameters();
  void updateMigrationProbs(uint8_t nextPolicyIndex);
  void updateStealingProbs(uint8_t nextPolicyIndex);

  // Stealing policy functions
  int consultStealingPolicy(uint8_t policyIndex);
  void makeStealingDecision();

  void normalizeSystemState();

  void broadcastParametersToNeighbours();

  // Agent
  AC *agent;
  Memory * agentMemory;

  // Policy info
  double **stealingProbs{};
  double **migrationProbs{};
  const int consensusPeriod;
  int learningStep = 0;

  // Policy and actor features function pointers
  void (*actorFeatures)(float *input, float *output, size_t n, int index);
  void (*criticFeatures)(float *input, float *output, size_t n);

  // Policy and actor feature sizes
  int actorFeatureSize;
  int criticFeatureSize;
  int criticStateSize;
  int actorStateSize;

  // Stats
  uint64_t receivedStealingReqs = 0;

  // make stealing decisions
  bool steal = true;

#elif defined(JBT)

  const int threshold;
  int **clusterQueueSizes;

#elif defined(JSQ) || defined(POWD)

  int **clusterQueueSizes;

#endif

#ifdef CHANGE_SPEED

  const bool changeSpeed;

#endif

};

#endif // DLB_SERVER_H
