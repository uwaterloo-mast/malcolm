#include "server.h"

void criticSecondOrderFeatures(float *input, float *output, size_t n) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    for (int j = i; j < n; j++) {
      output[k++] = input[i] * input[j];
    }
  }
  output[k] = 1.0f;
}

inline void actorSecondOrderFeatures(float *input, float *output, size_t n, int index) {

  int k = 0;

  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    for (int j = i; j < n; j++) {
      output[k++] = input[i] * input[j];
      output[k++] = input[index] * input[i] * input[j];
    }
  }
  output[k] = 1.0f;
}

void criticThirdOrderFeatures(float *input, float *output, size_t n) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    for (int j = i; j < n; j++) {
      output[k++] = input[i] * input[j];
      for (int l = j; l < n; l++) {
        output[k++] = input[i] * input[j] * input[l];
      }
    }
  }
  output[k] = 1.0f;
}

inline void actorThirdOrderFeatures(float *input, float *output, size_t n, int index) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    for (int j = i; j < n; j++) {
      output[k++] = input[i] * input[j];
      for (int l = j; l < n; l++) {
        output[k++] = input[i] * input[j] * input[l];
        output[k++] = input[index] * input[i] * input[j] * input[l];
      }
    }
  }
  output[k] = 1.0f;
}

void criticCustomizedOrderFeatures(float *input, float *output, size_t n) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    output[k++] = powf(input[i], 2);
    output[k++] = powf(input[i], 3);
    output[k++] = powf(input[i], 4);
    for (int j = i + 1; j < n; j++) {
      output[k++] = input[i] * input[j];
    }
  }
  output[k] = 1.0f;
}

inline void actorCustomizedOrderFeatures(float *input, float *output, size_t n, int index) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    output[k++] = powf(input[i], 2);
    output[k++] = powf(input[i], 3);
    output[k++] = powf(input[i], 4);
    for (int j = i + 1; j < n; j++) {
      output[k++] = input[i] * input[j];
      output[k++] = input[index] * input[i] * input[j];
    }
  }
  output[k] = 1.0f;
}

inline void criticExpFeatures(float *input, float *output, size_t n) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    output[k++] = powf(input[i], 2);
    output[k++] = powf(input[i], 3);
    output[k++] = (exp(input[i]) - 1) / 4;
    for (int j = i + 1; j < n; j++) {
      output[k++] = input[i] * input[j];
      output[k++] = (exp(input[i] - input[j]) - 1) / 4;
    }
  }
  output[k] = 1.0f;
}

inline void actorExpFeatures(float *input, float *output, size_t n, int index) {
  int k = 0;
  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    output[k++] = powf(input[i], 2);
    output[k++] = powf(input[i], 3);
    output[k++] = (exp(input[i]) - 1) / 4;
    for (int j = i + 1; j < n; j++) {
      output[k++] = input[i] * input[j];
      output[k++] = (exp(input[i] - input[j]) - 1) / 4;
    }
  }
  output[k] = 1.0f;
}

inline void criticSortedFeatures(float *input, float *output, size_t n) {

  int temp[n];

  std::iota(temp, temp + n, 0);

  std::stable_sort(temp, temp + n,
                   [&input](size_t i1, size_t i2) {return input[i1] > input[i2];});

  for (int i = 0; i < n; i++)
    output[temp[i]] = i / (float) (1000 * n);

  int k = n;

  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
    for (int j = i; j < n; j++) {
      output[k++] = input[i] * input[j];
    }
  }

  output[k] = 1.0f;
}

inline void actorSortedFeatures(float *input, float *output, size_t n, int index) {

  int temp[n];

  std::iota(temp, temp + n, 0);

  std::stable_sort(temp, temp + n,
                   [&input](size_t i1, size_t i2) {return input[i1] > input[i2];});

  float normalizer = 0.5 * log(n);
  for (int i = 0; i < n; i++)
    output[temp[i]] = (log(i + 1) - normalizer) / normalizer;

  int k = n;

  for (int i = 0; i < n; i++) {
    output[k++] = input[i];
//    for (int j = i; j < n; j++) {
//      output[k++] = input[i] * input[j];
//      output[k++] = input[index] * input[i] * input[j];
//    }
  }

  output[k] = 1.0f;
}

void Server::gatewaySMHandler(int session_num, erpc::SmEventType, erpc::SmErrType, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  instance->gatewaySMResponses++;
  int sessionIndex = instance->numPeers;

  for (int i = 0; i < instance->numPeers; i++) {
    if (i == instance->getServerID())
      continue;

    if (instance->gatewaySessionMap[i] == session_num) {
      sessionIndex = i;
      BOOST_LOG_TRIVIAL(info)
          << "Server ID: " << instance->getServerID()
          << ": Got gateway response from peer=" << i
          << " with session_num=" << session_num << "!" << std::endl;
    }
  }

  assert(sessionIndex < instance->numPeers);
}

Server::Server(const EndPointAddr &address, const Config &config, TaskExecutionFunc *taskExecutionFuncs)
    : serverID(config.get_integer(address.getURI() + ".id", 0)),
      address(address),
      numWorkers(config.get_integer(address.getURI() + ".numWorkers", 1)),
      numNeighbours(config.get_integer(address.getURI() + ".numNeighbours", 0)),
      numPeers(numNeighbours + 1 + (int)config.get_vector(address.getURI() + ".drivers").size()),
      config(config),
      maxOnFlyMessages(config.get_integer("maxOnFlyMsgs", 128)),
      perWorkerTaskQueueSize(config.get_integer("perWorkerTaskQueueSize", 4)),
      taskQueueCapacity(numWorkers * perWorkerTaskQueueSize),
      outboxCapacity(taskQueueCapacity * 1024),
      outboxDrainMaxBulkSize(maxOnFlyMessages),
      inboxCapacity(taskQueueCapacity * 256),
      inboxDrainMaxBulkSize(taskQueueCapacity),
      taskQueueSizeExpAvgCoef(1 - (1 / config.get_float("taskQueueSizeAvgNumSamples", 20))),
      nexus(address.getURI(),
            config.get_integer(address.getURI() + ".numaNode", 0),
            config.get_integer(address.getURI() + ".numNexusThreads", 0)),
      gatewayContext(maxOnFlyMessages),
      heartbeatMgrContext(maxOnFlyMessages),
      taskExecutionFuncs(taskExecutionFuncs),
      uRanGenerator(getCurrentTime()),
      uIntRanGenerator(getCurrentTime(), numNeighbours + 1),
      uIntRanGenerator2(getCurrentTime(), numNeighbours),
      numTaskTypes(config.get_integer("numTaskTypes", 1)),
      normalizationFactor(config.get_float("normalizationFactor", 0.001)),
      migrationCost(config.get_float("migrationCost", 0.0001)),
      stealingCost(config.get_float("stealingInitiationCost", 0.0001)),
      failingCost(config.get_float("failingCost", 100)),
      failedStealingCost(config.get_float("failedStealingCost", 0.1)),
      imbalanceExpAvgCoef(1 - (1 / config.get_float("earlyStoppingNumStepsCondition", 20))),
      rewardExpAvgCoef(1 - (1 / config.get_float("meanRewardMovingAvgNumSamples", 20))),
      rewardHistory(std::string("./") + std::to_string(serverID) + "_rewardHistory.bin",
          config.get_integer("numHistorySamples", 50000 * 60)),

#ifdef CHANGE_SPEED

    changeSpeed(serverID < 4),

#endif

#ifdef DLB

      consensusPeriod(config.get_integer("batchSize", 8) * config.get_integer("consensusPeriod", 16)),
      steal((config.get("stealing", "on") == "on" ? true : false)),

#elif defined(JBT)

      threshold(config.get_integer("JBTThreshold", 30)),

#endif

      stateHistory(std::string("./") + std::to_string(serverID) + "_stateHistory.bin",
          config.get_integer("numHistorySamples", 50000 * 60)) {

  auto neededTaskQueueCapacity = (int) ((ceil(taskQueueCapacity / (double) QUEUE_BLOCK_SIZE) + 1) * QUEUE_BLOCK_SIZE);
  taskQueue = new(std::align_val_t(64)) moodycamel::ConcurrentQueue<Message, Traits>(neededTaskQueueCapacity);
  outbox = new(std::align_val_t(64)) moodycamel::ConcurrentQueue<Packet>(outboxCapacity);
  inbox = new(std::align_val_t(64)) moodycamel::ConcurrentQueue<Message>(inboxCapacity);
  taskQueueProducerToken = new(std::align_val_t(64)) moodycamel::ProducerToken(*taskQueue);
  outboxConsumerToken = new(std::align_val_t(64)) moodycamel::ConsumerToken(*outbox);
  inboxConsumerToken = new(std::align_val_t(64)) moodycamel::ConsumerToken(*inbox);
  outboxContextBulkIndices = new(std::align_val_t(64)) size_t[outboxDrainMaxBulkSize];
  outboxBulkBuffer = new(std::align_val_t(64)) Packet[outboxDrainMaxBulkSize];
  inboxBulkBuffer = new(std::align_val_t(64)) Message[inboxDrainMaxBulkSize];

  nexus.register_req_func(ENQUEUE_TASK_ID, &Server::enqueueTask);
  nexus.register_req_func(UPDATE_LOAD_ID, &Server::updateLoad);

  heartbeatMgrSessionMap = new(std::align_val_t(64)) int[numNeighbours + 1];
  gatewaySessionMap = new(std::align_val_t(64)) int[numPeers];
  memset(gatewaySessionMap, -1, numPeers * sizeof(int));

  // workers initializations:
  workerThreads = new(std::align_val_t(64)) std::thread[numWorkers];

  localNeighbours = new(std::align_val_t(64)) Server *[numNeighbours + 1];
  for (auto i = 0; i < numNeighbours + 1; i++)
    localNeighbours[i] = nullptr;
  peersEndPoints = new(std::align_val_t(64)) EndPointAddr[numPeers];

  receivedTasksPerType = new(std::align_val_t(64)) uint64_t[numTaskTypes];
  memset(receivedTasksPerType, 0, numTaskTypes * sizeof(uint64_t));

  admittedTasksPerType = new(std::align_val_t(64)) uint64_t[numTaskTypes];
  memset(admittedTasksPerType, 0, numTaskTypes * sizeof(uint64_t));

  executedTasksPerType = new(std::align_val_t(64)) uint64_t[numTaskTypes];
  memset(executedTasksPerType, 0, numTaskTypes * sizeof(uint64_t));

  migratedTasksPerType = new(std::align_val_t(64)) uint64_t[numTaskTypes];
  memset(migratedTasksPerType, 0, numTaskTypes * sizeof(uint64_t));

  execTimePerTypeNs = new(std::align_val_t(64)) uint64_t[numTaskTypes];
  memset(execTimePerTypeNs, 0, numTaskTypes * sizeof(uint64_t));

  systemState = new(std::align_val_t(64)) float[numNeighbours + 1];
  memset(systemState, 0, (numNeighbours + 1) * sizeof(float));

#ifdef DLB

  nexus.register_req_func(CONSENSUS_ID, &Server::consensus);
  nexus.register_req_func(DEQUEUE_TASK_ID, &Server::dequeueTask);

  int numServers = numNeighbours + 1;
  criticStateSize = numServers;
  actorStateSize = numServers;

  if (config.get("criticOrder", "second") == "second") {
    criticFeatureSize = (criticStateSize + 1) * (criticStateSize + 2) / 2;
    criticFeatures = &criticSecondOrderFeatures;
  } else if (config.get("criticOrder") == "third") {
    criticFeatureSize = (criticStateSize + 1) * (criticStateSize + 2)  * (criticStateSize + 3) / 6;
    criticFeatures = &criticThirdOrderFeatures;
  } else if (config.get("criticOrder") == "exp") {
    criticFeatureSize = (criticStateSize) * (criticStateSize + 3) + 1;
    criticFeatures = &criticExpFeatures;
  } else if (config.get("criticOrder") == "sort"){
    criticFeatureSize = (criticStateSize + 1) * (criticStateSize + 2) / 2 + criticStateSize;
    criticFeatures = &criticSortedFeatures;
  } else {
    criticFeatureSize = (criticStateSize) * (criticStateSize + 7) / 2 + 1;
    criticFeatures = &criticCustomizedOrderFeatures;
  }

  if (config.get("actorOrder", "second") == "second") {
    actorFeatureSize = (actorStateSize + 1) * (actorStateSize + 2) / 2 +
        actorStateSize * (actorStateSize + 1) / 2;
    actorFeatures = &actorSecondOrderFeatures;
  } else if (config.get("actorOrder") == "third") {
    actorFeatureSize = (actorStateSize + 1) * (actorStateSize + 2)  * (actorStateSize + 3) / 6 +
        actorStateSize * (actorStateSize + 1)  * (actorStateSize + 2) / 6;
    actorFeatures = &actorThirdOrderFeatures;
  } else if (config.get("actorOrder") == "exp") {
    actorFeatureSize = (actorStateSize) * (actorStateSize + 3) + 1;
    actorFeatures = &actorExpFeatures;
  } else if (config.get("actorOrder") == "sort") {
//    actorFeatureSize = (actorStateSize + 1) * (actorStateSize + 2) / 2 +
//        actorStateSize * (actorStateSize + 1) / 2 + actorStateSize;
    actorFeatureSize = 2 * actorStateSize + 1;
    actorFeatures = &actorSortedFeatures;
  } else {
    actorFeatureSize = (actorStateSize) * (actorStateSize + 3) + 1;
    actorFeatures = &actorCustomizedOrderFeatures;
  }

  agent = new (std::align_val_t(64)) AC(
    numServers,
    config.get_float("criticLR", 0.0002),
    config.get_float("mActorLR", 0.0002),
    config.get_float("sActorLR", 0.0002),
    config.get_float("avgRewardStepSize", 0.0002),
    config.get_integer("updatePeriod", 40),
    config.get_integer("batchSize", 8),
    criticFeatureSize,
    actorFeatureSize,
    config.get_float("expBeta", 1),
    config.get_float("criticTDLambda", 0),
    config.get_float("actorsTDLambda", 0),
    config.get_float("betaOne", 0.9),
    config.get_float("betaTwo", 0.999),
    config.get_float("weightDecay", 0.0001),
    (config.get("LRScheduler", "constant") == "cosine" ? COSINEANNEALING : CONSTANT),
    config.get_float("minEta", 0.0001),
    config.get_float("maxEta", 0.001),
    config.get_integer("warmStartPeriod", 100),
    config.get_integer("lrWarmStartMult", 2),
    (config.get("optimization", "adamW") == "adamW" ? true : false));

  agentMemory = agent->getSharedMemory();

#endif

  initPolicies();
}

void Server::addLocalNeighbour(int id, Server *neighbour) {
  localNeighbours[id] = neighbour;
  numLocalNeighbours++;
}

void Server::enqueueTask(erpc::ReqHandle *req_handle, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  auto message_buf = req_handle->get_req_msgbuf()->buf_;
  auto *message = reinterpret_cast<Message *>(message_buf);
  assert(message->type == TASK);
  instance->handleTaskEnqueue(message);
  auto &resp = req_handle->pre_resp_msgbuf_;
  instance->gatewayRPC->resize_msg_buffer(&resp, 1);
  instance->gatewayRPC->enqueue_response(req_handle, &resp);
}

void Server::handleTaskEnqueue(Message *message) {
  auto taskInfo = reinterpret_cast<TaskInfo *>(message->data);
  assert(taskInfo->taskType < numTaskTypes);

  receivedTasks++;
  receivedTasksPerType[taskInfo->taskType]++;

  int destination = serverID;

  uint8_t policyIndex = policyAccessGuard.load(std::memory_order_acquire);
  if (taskInfo->numOfMigrations == 0)
    destination = consultMigrationPolicy(policyIndex);

  if (destination == serverID) {
    if (taskQueue->try_enqueue(*taskQueueProducerToken, *message)) {
      admittedTasks++;
      admittedTasksPerType[taskInfo->taskType]++;
    } else {
      // migratedOnFullQueue++;
      // destination = consultStaticMigrationPolicy(policyIndex);
      failedOnFullQueue++;
      Packet packet{};
      packet.message.type = RESULT;
      packet.rpcType = RECEIVE_RESULT_ID;
      packet.destination = taskInfo->driverID;
      auto resultInfo = reinterpret_cast<ResultInfo *>(packet.message.data);
      resultInfo->creationTime = taskInfo->creationTime;
      resultInfo->taskID = taskInfo->taskID;
      resultInfo->taskType = taskInfo->taskType;
      resultInfo->serverID = serverID;
      resultInfo->numOfMigrations = taskInfo->numOfMigrations;
      resultInfo->executionTimeNs = 0;
      resultInfo->failed = true;
      packet.message.dataSize = sizeof(ResultInfo);
      outbox->enqueue(packet);

#ifdef DLB

      agentMemory->incrementServerFailedTasks(policyIndex);

#endif

    }
    return;
  }

  assert(destination != serverID);

  message->senderID = serverID;
  taskInfo->numOfMigrations = taskInfo->numOfMigrations + 1;

  if (localNeighbours[destination] != nullptr) {
    localNeighbours[destination]->inbox->enqueue(*message);
    migratedTasks++;
    migratedTasksPerType[taskInfo->taskType]++;
  } else {
    Packet packet{};
    packet.rpcType = ENQUEUE_TASK_ID;
    packet.destination = destination;
    memcpy(&packet.message, message, MESSAGE_HEADER_SIZE + message->dataSize);
    outbox->enqueue(packet);
  }

#ifdef DLB

  agentMemory->incrementServerMigratedTasks(policyIndex);

#endif

}

void Server::checkInbox() {
  auto n = inbox->try_dequeue_bulk(*inboxConsumerToken, inboxBulkBuffer,inboxDrainMaxBulkSize);
  for (size_t i = 0; i < n; i++) {
    if (inboxBulkBuffer[i].type == TASK)
      handleTaskEnqueue(&inboxBulkBuffer[i]);

#ifdef DLB

    else if (inboxBulkBuffer[i].type == STEAL)
      handleTaskDequeue(&inboxBulkBuffer[i]);

#endif

  }
}

void Server::startRunning() {
  spawnThreads();
  joinThreads();
}

void Server::spawnThreads() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Creating server threads!" << std::endl;

  gatewayThread = std::thread(&Server::startGateway, this);

  for (auto i = 0; i < numWorkers; i++)
    workerThreads[i] = std::thread(&Server::startWorker, this, i);
}

void Server::joinThreads() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Waiting for server threads to finish!" << std::endl;

  gatewayThread.join();

  for (int i = 0; i < numWorkers; i++)
    workerThreads[i].join();

  heartbeatMgrThread.join();
}

// Receiver thread
 [[noreturn]] void Server::startGateway() {
  int firstCPUID = config.get_integer(address.getURI() + ".coreAffinityInit", 0);
  assert(numa_node_of_cpu(firstCPUID) == config.get_integer(address.getURI() + ".numaNode", 0));
  setCoreAffinity(firstCPUID);

  createGatewayRPC();

  establishGatewayConnections();

  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << ": Starting gateway!" << std::endl;

  heartbeatMgrThread = std::thread(&Server::startHeartbeatMgr, this);

  while (true) {
    gatewayRPC->run_event_loop_once();
    checkInbox();
    drainOutbox();
    taskQueueSize = taskQueueSizeExpAvgCoef * taskQueueSize + (1 - taskQueueSizeExpAvgCoef) * taskQueue->size_approx();
  }
}

void Server::createGatewayRPC() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Creating gateway RPC!" << std::endl;
  int physicalPortNumber = config.get_integer(address.getURI() + ".receiverPhysicalPortNumber", (address.getPortNumber() + 1) % 2);
  gatewayRPC = new erpc::Rpc<erpc::CTransport>(&nexus, static_cast<void *>(this), RECEIVER_RPC_ID, gatewaySMHandler, physicalPortNumber);
  gatewayRPC->retry_connect_on_invalid_rpc_id_ = true;
  for (size_t i = 0; i < gatewayContext.maxOnFlyMsgs; i++) {
    gatewayContext.reqBufs[i] = gatewayRPC->alloc_msg_buffer_or_die(sizeof(Message));
    gatewayContext.respBufs[i] = gatewayRPC->alloc_msg_buffer_or_die(sizeof(Message));
  }
}

void Server::establishGatewayConnections() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Establishing gateway connection to remote neighbours!" << std::endl;

  std::vector<std::string> neighboursURIs = config.get_vector(address.getURI() + ".neighbours");
  std::vector<std::string> driversURIs = config.get_vector(address.getURI() + ".drivers");
  std::vector<std::string> peersURIs = neighboursURIs;

  peersURIs.insert(peersURIs.end(), driversURIs.begin(), driversURIs.end());

  for (auto &peer : peersURIs) {
    auto id = config.get_integer(peer + ".id");
    if (id >= numNeighbours + 1 || localNeighbours[id] == nullptr) {
      int sessionNum = gatewayRPC->create_session(peer, RECEIVER_RPC_ID);
      assert(sessionNum >= 0);
      gatewaySessionMap[id] = sessionNum;
      std::cout << "Peer ID: " << id << " is connected with session num=" << sessionNum << std::endl;
    }
    peersEndPoints[id] = peer;
    std::cout << "Peer ID: " << id << " is " << peer << std::endl;
  }

  while (gatewaySMResponses != (numPeers - numLocalNeighbours))
    gatewayRPC->run_event_loop_once();

  Packet packet{};
  packet.message.senderID = serverID;
  packet.message.type = READY;
  packet.message.dataSize = 0;
  packet.rpcType = NOTIFY_READY_ID;

  for (const auto& driverURI : driversURIs) {
    packet.destination = config.get_integer(driverURI + ".id");
    outbox->enqueue(packet);
  }
}

inline void Server::drainOutbox() {
  auto availableSize = gatewayContext.availableSlotSize();
  if (availableSize == 0)
    return;

  auto n = outbox->try_dequeue_bulk(*outboxConsumerToken, outboxBulkBuffer, availableSize);
  if (n == 0)
    return;

  gatewayContext.getList(outboxContextBulkIndices, n);
  processOutboxPackets(n);
}

void Server::processOutboxPackets(size_t numberOfMessages) {

  for (int i = 0; i < numberOfMessages; i++) {
    auto contextIndex = outboxContextBulkIndices[i];

    assert(gatewaySessionMap[outboxBulkBuffer[i].destination] != -1);

    outboxBulkBuffer[i].message.senderID = serverID;
    auto rpcType = outboxBulkBuffer[i].rpcType;

    if (rpcType == RECEIVE_RESULT_ID) {
      executedTasks++;
      auto resultInfo = reinterpret_cast<ResultInfo *>(outboxBulkBuffer[i].message.data);
      if (!resultInfo->failed) {
        auto taskType = resultInfo->taskType;
        executedTasksPerType[taskType]++;
        // exeTime = 0.99 * exeTime + 0.01 * newExeTime
        execTimePerTypeNs[taskType] -= execTimePerTypeNs[taskType] >> 6;
        execTimePerTypeNs[taskType] += resultInfo->executionTimeNs >> 6;
        execTimeNs -= execTimeNs >> 6;
        execTimeNs += resultInfo->executionTimeNs >> 6;

#ifdef DLB

        if (steal && taskQueueSize < 1)
          makeStealingDecision();

#endif

      }

    } else if (rpcType == ENQUEUE_TASK_ID) {
      auto taskInfo = reinterpret_cast<TaskInfo *>(outboxBulkBuffer[i].message.data);
      migratedTasks++;
      migratedTasksPerType[taskInfo->taskType]++;
    }

    gatewayRPC->resize_msg_buffer(&gatewayContext.reqBufs[contextIndex],
                                  MESSAGE_HEADER_SIZE +
                                      outboxBulkBuffer[i].message.dataSize);

    std::memcpy(gatewayContext.reqBufs[contextIndex].buf_,
                reinterpret_cast<const uint8_t *>(&outboxBulkBuffer[i].message),
                MESSAGE_HEADER_SIZE + outboxBulkBuffer[i].message.dataSize);

    gatewayRPC->enqueue_request(gatewaySessionMap[outboxBulkBuffer[i].destination],
        outboxBulkBuffer[i].rpcType, &gatewayContext.reqBufs[contextIndex],
        &gatewayContext.respBufs[contextIndex],
        [](void *context, void *tag) {
          auto server = static_cast<Server *>(context);
          server->gatewayContext.putBack(reinterpret_cast<size_t>(tag));
        },
        reinterpret_cast<void *>(contextIndex));
  }
}

void Server::initPolicies() {

  int initialIndex = 0;

#ifdef DLB

  stealingProbs = new(std::align_val_t(64)) double *[2];
  migrationProbs = new(std::align_val_t(64)) double *[2];
  for (int i = 0; i < 2; ++i) {
    migrationProbs[i] = new(std::align_val_t(64)) double[numNeighbours + 1];
    stealingProbs[i] = new(std::align_val_t(64)) double[numNeighbours + 1];
  }

  initialIndex = agentMemory->getNewIndex();
  // agentMemory->setState(systemState, initialIndex, numNeighbours + 1);
  actorFeatures(systemState, agentMemory->getState(initialIndex), actorStateSize, serverID);
  agent->updateProbabilities(agentMemory->getState(initialIndex),
                             agentMemory->getMigrationProbs(initialIndex),
                             agentMemory->getStealingProbs(initialIndex),
                             agentMemory->getMigrationLinearOutput(initialIndex),
                             agentMemory->getStealingLinearOutput(initialIndex),
                             agentMemory->alignedStateSize,
                             numNeighbours + 1);

  updateMigrationProbs(initialIndex);
  updateStealingProbs(initialIndex);

#elif defined(JSQ) || defined(JBT) || defined(POWD)

  clusterQueueSizes = new (std::align_val_t(64)) int *[2];
  for (int i = 0; i < 2; i++) {
    clusterQueueSizes[i] = new (std::align_val_t(64)) int [numNeighbours + 1];
    memset(clusterQueueSizes[i], 0.0f, (numNeighbours + 1) * sizeof(int));
  }

#endif

  policyAccessGuard.store(initialIndex, std::memory_order_release);

}

inline int Server::consultMigrationPolicy(const uint8_t policyIndex) {
  auto index = policyIndex % 2;
  int destination = serverID;

#ifdef DLB

  auto next = uRanGenerator.getNext().second;
  destination = (int) (std::upper_bound(migrationProbs[index], migrationProbs[index] + numNeighbours, next) - migrationProbs[index]);
  agentMemory->incrementMigrationsAction(destination, policyIndex);

#elif defined(JSQ)

  clusterQueueSizes[index][serverID] = taskQueue->size_approx();
  destination = std::min_element(clusterQueueSizes[index], clusterQueueSizes[index] + numNeighbours + 1) - clusterQueueSizes[index];
  clusterQueueSizes[index][destination]++;

#elif defined(JBT)

  clusterQueueSizes[index][serverID] = taskQueue->size_approx();
  destination = (int)uIntRanGenerator.getNext().second;
  for (int i = 0; i < numNeighbours + 1; i++) {
    if (clusterQueueSizes[index][i] < threshold) {
      destination = i;
      break;
    }
  }
  clusterQueueSizes[index][destination]++;

#elif defined(POWD)

  clusterQueueSizes[index][serverID] = taskQueue->size_approx();
  int randServer[2];
  randServer[0] = (int)uIntRanGenerator.getNext().second;
  auto randomNum = (int)uIntRanGenerator2.getNext().second;
  randServer[1] = (randomNum >= randServer[0]) ? (randomNum + 1) : randomNum;
  auto selectedServer = (clusterQueueSizes[index][randServer[0]] >=
      clusterQueueSizes[index][randServer[1]]) ? randServer[1] : randServer[0];
  destination = selectedServer;
  clusterQueueSizes[index][destination]++;

#endif

  assert(destination <= numNeighbours);
  return destination;
}

// Worker Thread
[[noreturn]] void Server::startWorker(int workerID) {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Entering worker ID: " << workerID << std::endl;
  int firstCPUID = config.get_integer(address.getURI() + ".coreAffinityInit", 0);
  int threadAssignedCore = firstCPUID + 2 + workerID;
  assert(numa_node_of_cpu(threadAssignedCore) == config.get_integer(address.getURI() + ".numaNode", 0));
  setCoreAffinity(threadAssignedCore);

  auto startTime = getCurrentTime();

  while (true) {
    Message message{};

    while (!taskQueue->try_dequeue_from_producer(*taskQueueProducerToken, message))
      ;

    auto taskInfo = reinterpret_cast<TaskInfo *>(message.data);
    auto taskType = taskInfo->taskType;
    taskInfo->data = (char *)(message.data + sizeof(TaskInfo));
    auto driverId = taskInfo->driverID;
    if (driverId < numNeighbours + 1 || driverId >= numPeers) {
      std::cout << "Invalid driverID: " << driverId << std::endl;
    }

    Packet packet{};
    packet.message.type = RESULT;
    packet.rpcType = RECEIVE_RESULT_ID;
    packet.destination = driverId;

    auto resultInfo = reinterpret_cast<ResultInfo *>(packet.message.data);
    resultInfo->data = (char *)(packet.message.data + sizeof(ResultInfo));
    resultInfo->creationTime = taskInfo->creationTime;
    resultInfo->taskID = taskInfo->taskID;
    resultInfo->taskType = taskType;
    resultInfo->serverID = serverID;
    resultInfo->numOfMigrations = taskInfo->numOfMigrations;

#ifdef CHANGE_SPEED

    if (changeSpeed && (getCurrentTime() - startTime) > 3 * TEN_SECONDS_IN_NANOSECONDS) {
      double executionTime;
      memcpy(&executionTime, taskInfo->data, sizeof(executionTime));
      executionTime *=  1.5;
      memcpy(taskInfo->data, &executionTime, sizeof(executionTime));
    }

#endif

    auto start = getCurrentTime();
    taskExecutionFuncs[0](taskInfo->data, taskInfo->dataSize, resultInfo->data,
                          resultInfo->dataSize, taskType);
    auto finish = getCurrentTime();
    resultInfo->executionTimeNs = finish - start;

    packet.message.dataSize = resultInfo->dataSize + sizeof(ResultInfo);
    outbox->enqueue(packet);
  }
}

void Server::setTaskExecutionFuncs(TaskExecutionFunc *taskExecutionFunc) {
  taskExecutionFuncs = taskExecutionFunc;
}


// Heart beater Thread!
[[noreturn]] void Server::startHeartbeatMgr() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Starting heartbeatMgr thread!" << std::endl;
  int firstCPUID = config.get_integer(address.getURI() + ".coreAffinityInit", 0);
  int threadAssignedCore = firstCPUID + 1;
  assert(numa_node_of_cpu(threadAssignedCore) == config.get_integer(address.getURI() + ".numaNode", 0));

  setCoreAffinity(threadAssignedCore);

  createHeartbeatMgrRPC();

  establishHeartbeatMgrConnections();

  // Wait for a task to arrive!
  while (receivedTasks == 0)
    ;

  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Started heartbeatMgr!" << std::endl;

  int heartBeatPeriod = config.get_integer("heartBeatPeriod", 100);
  uint64_t periodInNanosecond = heartBeatPeriod * MICROSECOND_TO_NANOSECOND;
  int periodIndex = 0;
  auto printingIndex = TEN_SECOND_IN_US / heartBeatPeriod;
  auto start = getCurrentTime();
  uint64_t previousMigrations = 0;
  uint64_t previousFailures = 0;

  while (true) {
    periodIndex++;
    calculateServerLoad();
    stateHistory += (serverLoad * perWorkerTaskQueueSize);
    systemState[serverID] = Server::normalizeLoad(serverLoad);

    broadcastLoadToNeighbours();

    while (getCurrentTime() - start < periodIndex * periodInNanosecond) {
      heartbeatMgrRPC->run_event_loop_once();
    }

#ifdef DLB

    // normalizeSystemState();
    updateACParameters();

#elif defined(JSQ) || defined(JBT) || defined(POWD)

    int nextIndex = policyAccessGuard.load(std::memory_order_acquire) + 1;
    policyAccessGuard.store(nextIndex, std::memory_order_release);

    auto reward = Server::globalReward(systemState,
                                       migratedTasks - previousMigrations, 0,
                                       failedOnFullQueue - previousFailures, 0);
    previousMigrations = migratedTasks;
    previousFailures = failedOnFullQueue;

    rewardExpAvg = rewardExpAvgCoef * rewardExpAvg + (1.0f - rewardExpAvgCoef) * reward;
    rewardHistory += reward;

#endif

    if (periodIndex % printingIndex == 0) {
      arrivalRate = (float) receivedTasks / 10.0f;
      printStats();
      start = getCurrentTime();
      periodIndex = 0;
    }
  }
}

void Server::heartbeatMgrSMHandler(int session_num, erpc::SmEventType,
                                    erpc::SmErrType, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  instance->heartbeatMgrSMResponses++;
  int sessionIndex = instance->numNeighbours + 1;
  for (int i = 0; i < instance->numNeighbours + 1; i++) {
    if (i == instance->getServerID())
      continue;
    if (instance->heartbeatMgrSessionMap[i] == session_num) {
      sessionIndex = i;
      BOOST_LOG_TRIVIAL(info)
          << "Server ID: " << instance->getServerID()
          << ": Got heartbeatMgr response from peer=" << i
          << " with session_num=" << session_num << "!" << std::endl;
    }
  }
  assert(sessionIndex < instance->numNeighbours + 1);
}

void Server::updateLoad(erpc::ReqHandle *req_handle, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  auto message_buf = req_handle->get_req_msgbuf()->buf_;
  auto *message = reinterpret_cast<Message *>(message_buf);
  assert(message->type == LOAD);
  auto *loadInfo = reinterpret_cast<LoadInfo *>(message->data);
  auto latestGuard = instance->policyAccessGuard.load(std::memory_order_acquire);
  // instance->systemState[loadInfo->serverID] = loadInfo->load;
  instance->systemState[loadInfo->serverID] = instance->normalizeLoad(loadInfo->load);

#ifdef DLB

  instance->agentMemory->incrementOthersStolenTasks(loadInfo->stolenTasks, latestGuard);
  instance->agentMemory->incrementOthersMigratedTasks(loadInfo->migratedTasks, latestGuard);
  instance->agentMemory->incrementOthersFailedTasks(loadInfo->failedTasks, latestGuard);
  instance->agentMemory->incrementOthersFailedStealingAttempts(loadInfo->failedStealingAttempts, latestGuard);

#elif defined(JSQ) || defined(JBT) || defined(POWD)

  auto nextIndex = (latestGuard + 1) % 2;
  instance->clusterQueueSizes[nextIndex][loadInfo->serverID] = loadInfo->queueSize;

#endif

  auto &resp = req_handle->pre_resp_msgbuf_;
  instance->heartbeatMgrRPC->resize_msg_buffer(&resp, 1);
  instance->heartbeatMgrRPC->enqueue_response(req_handle, &resp);
}

void Server::createHeartbeatMgrRPC() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Create heartbeatMgr RPC!" << std::endl;

  int physicalPortNumber = config.get_integer(address.getURI() + ".heartbeatMgrPhysicalPortNumber",
                                              address.getPortNumber() % 2);

  heartbeatMgrRPC = new erpc::Rpc<erpc::CTransport>(
      &nexus, static_cast<void *>(this), HEARTBEAT_MGR_RPC_ID,
      heartbeatMgrSMHandler, physicalPortNumber);

  heartbeatMgrRPC->retry_connect_on_invalid_rpc_id_ = true;

  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Allocating context buffers!" << std::endl;

  for (size_t i = 0; i < heartbeatMgrContext.maxOnFlyMsgs; i++) {
    heartbeatMgrContext.reqBufs[i] = heartbeatMgrRPC->alloc_msg_buffer_or_die(sizeof(Message));
    heartbeatMgrContext.respBufs[i] = heartbeatMgrRPC->alloc_msg_buffer_or_die(sizeof(Message));
  }
}

void Server::establishHeartbeatMgrConnections() {
  BOOST_LOG_TRIVIAL(info) << "Server ID: " << serverID << " - Establishing heartbeat connection to neighbours!" << std::endl;

  for (auto neighbour = 0; neighbour < numNeighbours + 1; neighbour++) {
    if (neighbour == serverID)
      continue;

    int sessionNum = heartbeatMgrRPC->create_session(peersEndPoints[neighbour].getURI(), HEARTBEAT_MGR_RPC_ID);
    assert(sessionNum >= 0);
    heartbeatMgrSessionMap[neighbour] = sessionNum;
  }

  while (heartbeatMgrSMResponses != numNeighbours) {
    heartbeatMgrRPC->run_event_loop_once();
  }
}

inline void Server::broadcastLoadToNeighbours() {
  Message message{};
  message.type = LOAD;
  auto loadInfo = reinterpret_cast<LoadInfo *>(message.data);
  auto latestGuard = policyAccessGuard.load(std::memory_order_acquire);
  loadInfo->load = serverLoad;

#ifdef DLB

  loadInfo->stolenTasks = agentMemory->getServerStolenTasks(latestGuard);
  loadInfo->migratedTasks = agentMemory->getServerMigratedTasks(latestGuard);
  loadInfo->failedTasks = agentMemory->getServerFailedTasks(latestGuard);
  loadInfo->failedStealingAttempts = agentMemory->getServerFailedStealingAttempts(latestGuard);

#elif defined(JSQ) || defined(POWD) || defined(JBT)

  loadInfo->queueSize = taskQueue->size_approx();

#endif

  loadInfo->serverID = serverID;
  message.dataSize = sizeof(LoadInfo);
  message.senderID = serverID;
  broadcastMessageToNeighbours(message, UPDATE_LOAD_ID);
}

inline void Server::broadcastMessageToNeighbours(Message &message, int rpcType) {
  for (auto neighbour = 0; neighbour < numNeighbours + 1; neighbour++) {

    if (neighbour == serverID)
      continue;

    auto startTime = getCurrentTime();
    bool flag = false;

    while (!heartbeatMgrContext.slotIsAvailable()) {
      if (!flag && (getCurrentTime() - startTime) >= SECOND_TO_NANOSECOND) {
        std::cout << "WARNING: heartbeatMgr has been waiting for too long with rpcType = " << rpcType << "!" << std::endl;
        flag = true;
      }
      heartbeatMgrRPC->run_event_loop_once();
    }

    auto resourceIndex = heartbeatMgrContext.get();

    heartbeatMgrRPC->resize_msg_buffer(&heartbeatMgrContext.reqBufs[resourceIndex], MESSAGE_HEADER_SIZE + message.dataSize);
    message.senderID = serverID;
    std::memcpy(heartbeatMgrContext.reqBufs[resourceIndex].buf_,
                reinterpret_cast<uint8_t *>(&message),
                MESSAGE_HEADER_SIZE + message.dataSize);

    heartbeatMgrRPC->enqueue_request(heartbeatMgrSessionMap[neighbour],
        rpcType, &heartbeatMgrContext.reqBufs[resourceIndex],
        &heartbeatMgrContext.respBufs[resourceIndex],
        [](void *context, void *tag) {
          auto server = static_cast<Server *>(context);
          auto index = reinterpret_cast<size_t>(tag);
          server->putBackHeartbeatMgrResource(index);
        },
        reinterpret_cast<void *>(resourceIndex));
    heartbeatMgrRPC->run_event_loop_once();
  }
}

inline void Server::putBackHeartbeatMgrResource(size_t id) {
  heartbeatMgrContext.putBack(id);
}

inline float Server::normalizeLoad(float load) const {
  return load * normalizationFactor;
}

inline void Server::calculateServerLoad() {
  serverLoad = taskQueueSize * execTimeNs * NANOSECOND_TO_MICROSECOND / (float) taskQueueCapacity;
//  serverLoad = taskQueue->size_approx() / (float) taskQueueCapacity;
  // TODO: data race between gateway and heartbeatMgr
  auto serverQueueSize = receivedTasks - executedTasks - migratedTasks;
//  maxServerQueueSize = 0.99 * maxServerQueueSize + 0.01 * serverQueueSize;
  maxServerQueueSize -= maxServerQueueSize >> 6;
  maxServerQueueSize += serverQueueSize >> 6;

}

inline void Server::printStats() {
  std::cout << "Server : " << address.getURI() << " : server load " << serverLoad << std::endl;
  std::cout << "Server : " << address.getURI() << " : avg execution time " << execTimeNs << std::endl;
  std::cout << "Server : " << address.getURI() << " : task queue size " << taskQueueSize << std::endl;
  std::cout << "Server : " << address.getURI() << " : admission rate " << (admittedTasks - stolenTasks) / (float) receivedTasks << std::endl;
  std::cout << "Server : " << address.getURI() << " : admitted tasks " << admittedTasks << std::endl;
  std::cout << "Server : " << address.getURI() << " : migrated (and stolen) tasks " << migratedTasks << std::endl;
  std::cout << "Server : " << address.getURI() << " : stolen tasks " << stolenTasks << std::endl;
  std::cout << "Server : " << address.getURI() << " : failed on full queue " << failedOnFullQueue << std::endl;
  std::cout << "Server : " << address.getURI() << " : arrival rate " << arrivalRate << std::endl;
  std::cout << "Server : " << address.getURI() << " : max server queue size " << maxServerQueueSize << std::endl;
  for (int type = 0; type < numTaskTypes; type++)
    std::cout << "Server : " << address.getURI() << " : execution time for type " << type << " " << execTimePerTypeNs[type] << std::endl;
  std::cout << "Server : " << address.getURI() << " : mean reward " << rewardExpAvg << std::endl;
  std::cout << "Server : " << address.getURI() << " : mean imbalance " << imbalanceExpAvg << std::endl;

#ifdef DLB

  std::cout << "Server : " << address.getURI() << " : received stealing requests " << receivedStealingReqs << std::endl;
  std::cout << "Server : " << address.getURI() << " : learning step " << learningStep << std::endl;

  int policyIndex = policyAccessGuard.load(std::memory_order_acquire);
  auto states = agentMemory->getState(policyIndex);
  for (int i = 0; i < numNeighbours + 1; i++) {
    std::cout << states[i] << " ";
  }

  std::cout << std::endl;
  auto probs = agentMemory->getMigrationProbs(policyIndex);
  for (int i = 0; i < numNeighbours + 1; i++)
    std::cout << probs[i] << " ";

  std::cout << std::endl;
  auto mLinear = agentMemory->getMigrationLinearOutput(policyIndex);
  for (int i = 0; i < numNeighbours + 1; i++)
    std::cout << mLinear[i] << " ";

  std::cout << std::endl;

#endif

}

inline float Server::globalReward(float *state, uint64_t numOfMigratedTasks,
                                  uint64_t numOfStolenTasks, uint64_t numOfFailedTasks,
                                  uint64_t numOfFailedStealingAttempts) {
  float imbalance = 0.0f;
  for (auto i = 0; i < numNeighbours + 1; i++)
    for (auto j = i + 1; j < numNeighbours + 1; j++)
      imbalance += powf(state[i] - state[j], 2);
  imbalance /= ((float)((numNeighbours + 1) * numNeighbours) / 2);
  imbalance /= normalizationFactor;

  auto improvement = previousImbalance - imbalance;
  previousImbalance = imbalance;

  imbalanceExpAvg = imbalanceExpAvg * imbalanceExpAvgCoef + imbalance * (1.0f - imbalanceExpAvgCoef);

  auto costs = migrationCost * numOfMigratedTasks +
      stealingCost * numOfStolenTasks +
      failingCost * numOfFailedTasks +
      failedStealingCost * numOfFailedStealingAttempts;
  costs /= (float)(numNeighbours + 1);
  costs *= normalizationFactor;

//  auto loss = imbalance + costs;
//  return (-1 * loss * normalizationFactor);
  return improvement - costs;
}

#ifdef DLB

void Server::consensus(erpc::ReqHandle *req_handle, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  auto message_buf = req_handle->get_req_msgbuf()->buf_;
  auto message = reinterpret_cast<Message *>(message_buf);
  assert(message->type == CONSENSUS);
  auto consensusInfo = reinterpret_cast<ConsensusInfo *>(message->data);
  alignas(32) float parameters[MAX_CONS_SIZE];
  memcpy(parameters, consensusInfo->parameters, message->dataSize);
  instance->agent->consensusIncrementalStep(parameters, message->dataSize / sizeof(float));
  auto &resp = req_handle->pre_resp_msgbuf_;
  instance->heartbeatMgrRPC->resize_msg_buffer(&resp, 1);
  instance->heartbeatMgrRPC->enqueue_response(req_handle, &resp);
}

void Server::dequeueTask(erpc::ReqHandle *req_handle, void *context) {
  auto instance = reinterpret_cast<Server *>(context);
  auto message_buf = req_handle->get_req_msgbuf()->buf_;
  auto *message = reinterpret_cast<Message *>(message_buf);
  assert(message->type == STEAL);
  instance->handleTaskDequeue(message);
  auto &resp = req_handle->pre_resp_msgbuf_;
  instance->gatewayRPC->resize_msg_buffer(&resp, 1);
  instance->gatewayRPC->enqueue_response(req_handle, &resp);
}

void Server::handleTaskDequeue(Message *message){
  receivedStealingReqs++;
  Packet packet{};

  if (taskQueue->try_dequeue_from_producer(*taskQueueProducerToken, packet.message)) {
    stolenTasks++;
    agentMemory->incrementServerMigratedTasks(policyAccessGuard.load(std::memory_order_acquire));
    packet.message.senderID = serverID;
    auto taskInfo = reinterpret_cast<TaskInfo *>(packet.message.data);
    taskInfo->numOfMigrations = taskInfo->numOfMigrations + 1;

    if (localNeighbours[message->senderID] != nullptr) {
      localNeighbours[message->senderID]->inbox->enqueue(packet.message);
      migratedTasks++;
      migratedTasksPerType[taskInfo->taskType]++;
    } else {
      packet.rpcType = ENQUEUE_TASK_ID;
      packet.destination = message->senderID;
      outbox->enqueue(packet);
    }
  } else {
    agentMemory->incrementServerFailedStealingAttempts(policyAccessGuard.load(std::memory_order_acquire));
  }
}

inline int Server::consultStealingPolicy(const uint8_t policyIndex) {
  auto index = policyIndex % 2;
  auto next = uRanGenerator.getNext().second;
  int stealFrom = (int) (std::upper_bound(stealingProbs[index], stealingProbs[index] + numNeighbours, next) - stealingProbs[index]);
  agentMemory->incrementStealingAction(stealFrom, policyIndex);
  assert(stealFrom <= numNeighbours);
  return stealFrom;
}

void Server::makeStealingDecision() {
  uint8_t policyGuard = policyAccessGuard.load(std::memory_order_acquire);
  auto stealFromID = consultStealingPolicy(policyGuard);

  if (stealFromID == serverID)
    return;

  agentMemory->incrementServerStolenTasks(policyGuard);
  Packet packet{};
  packet.message.type = STEAL;
  packet.message.dataSize = 0;
  packet.message.senderID = serverID;

  if (localNeighbours[stealFromID] != nullptr) {
    localNeighbours[stealFromID]->inbox->enqueue(packet.message);
  } else {
    packet.rpcType = DEQUEUE_TASK_ID;
    packet.destination = stealFromID;
    outbox->enqueue(packet);
  }
}

inline void Server::updateMigrationProbs(uint8_t nextPolicyIndex) {
  auto index = nextPolicyIndex % 2;
  const auto probs = agentMemory->getMigrationProbs(nextPolicyIndex);
  float sum = std::accumulate(probs, probs + numNeighbours + 1, 0.0f);
  float partialSum = 0;
  for (int i = 0; i < numNeighbours + 1; i++) {
    partialSum += probs[i];
    migrationProbs[index][i] = partialSum / sum;
  }
}

inline void Server::updateStealingProbs(uint8_t nextPolicyIndex) {
  auto index = nextPolicyIndex % 2;
  const auto probs = agentMemory->getStealingProbs(nextPolicyIndex);
  float sum = std::accumulate(probs, probs + numNeighbours + 1, 0.0f);
  float partialSum = 0;
  for (int i = 0; i < numNeighbours + 1; i++) {
    partialSum += probs[i];
    stealingProbs[index][i] = partialSum / sum;
  }
}

inline void Server::broadcastParametersToNeighbours() {
  Message message{};
  message.type = CONSENSUS;
  auto consensusInfo = reinterpret_cast<ConsensusInfo *>(message.data);
  message.dataSize = agent->getCriticParameters(consensusInfo->parameters);
  message.senderID = serverID;
  broadcastMessageToNeighbours(message, CONSENSUS_ID);
}

inline void Server::updateACParameters() {
  int previousPolicyIndex = policyAccessGuard.load(std::memory_order_acquire);
  int nextPolicyIndex = agentMemory->getNewIndex();

  assert(nextPolicyIndex == agentMemory->getNextIndex(previousPolicyIndex));

  auto reward = Server::globalReward(systemState,
      agentMemory->getTotalNumOfMigratedTasks(previousPolicyIndex),
      agentMemory->getTotalNumOfStolenTasks(previousPolicyIndex),
      agentMemory->getTotalNumOfFailedTasks(previousPolicyIndex),
      agentMemory->getTotalNumOfFailedStealingAttempts(previousPolicyIndex));

  agentMemory->setReward(previousPolicyIndex, reward);

  rewardExpAvg = rewardExpAvgCoef * rewardExpAvg + (1.0f - rewardExpAvgCoef) * reward;

  rewardHistory += reward;

  // agentMemory->setState(systemState, nextPolicyIndex, numNeighbours + 1);

  actorFeatures(systemState, agentMemory->getState(nextPolicyIndex), actorStateSize, serverID);
  criticFeatures(systemState, agentMemory->getPolyFeatures(nextPolicyIndex), criticStateSize);

  learningStep++;
  agent->newSampleReady(previousPolicyIndex);

  updateMigrationProbs(nextPolicyIndex);
  updateStealingProbs(nextPolicyIndex);

  policyAccessGuard.store(nextPolicyIndex, std::memory_order_release);

  if (learningStep % consensusPeriod == 1 && learningStep > 1)
    agent->consensusFinalStep();
  if (learningStep % consensusPeriod == 0)
    broadcastParametersToNeighbours();
}

inline void Server::normalizeSystemState() {
  auto maxIndex = std::max_element(systemState, systemState + numNeighbours + 1) - systemState;
  auto maxState = systemState[maxIndex];
  for (int i = 0; i < numNeighbours + 1; i++) {
    systemState[i] = normalizeLoad(systemState[i] / maxState);
  }
}

#endif
