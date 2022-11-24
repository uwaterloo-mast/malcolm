#include "driver.h"
#include <algorithm>

void Driver::sm_handler(int session_num, erpc::SmEventType, erpc::SmErrType,
                        void *context) {
    auto instance = reinterpret_cast<Driver *>(context);
    instance->numSMResponses++;
    int sessionIndex = instance->numServers;
    for (int i = 0; i < instance->numServers; i++) {
        if (instance->sessionMap[i] == session_num) {
            sessionIndex = i;
            BOOST_LOG_TRIVIAL(info)
                    << "Driver ID: " << instance->getDriverID()
                    << ": Got gateway response from peer=" << i
                    << " with session_num=" << session_num << "!" << std::endl;
        }
    }
    assert(sessionIndex < instance->numServers);
}

Driver::Driver(EndPointAddr addr, Config config, TaskGenerationFunc func)
    : driverID(config.get_integer(addr.getURI() + ".id", 0)),
      config(config),
      address(addr),
      numServers(config.get_vector(addr.getURI() + ".servers").size()),

      taskGenerationFunc(func),

      nexus(addr.getURI(), config.get_integer(addr.getURI() + ".numaNode", 0),
            config.get_integer(addr.getURI() + ".num_nexus_threads", 0)),

      senderReceiverContext(config.get_integer("maxOnFlyMsgs", 128)),

      expRanGenerator(getCurrentTime(),
          1.0e9 / (config.get_float(addr.getURI() + ".arrivalRate", 1.0e2))),
      uIntRanGenerator(getCurrentTime(), numServers),
      startRecording(config.get_integer("startRecording", 1000000)),

      latencyHistory(std::string("./") + std::to_string(driverID) + "_latencyHistory.bin",
                     config.get_integer("numLatencySamples", 5000 * 60)) {

  nexus.register_req_func(RECEIVE_RESULT_ID, &Driver::receiveResult);
  nexus.register_req_func(NOTIFY_READY_ID, &Driver::receiveReadyNotification);
  sessionMap = new(std::align_val_t(64)) int[numServers];
  serversEndPoints = new(std::align_val_t(64)) EndPointAddr[numServers];

#ifdef FLUCTUATE_LOAD

  std::discrete_distribution<> d({18, 18, 18, 18, 1, 1, 1, 1, 1, 1, 1, 1});
  departureShares.emplace_back(d);

#endif

}

int Driver::getDriverID() {
    return driverID;
}

void Driver::receiveResult(erpc::ReqHandle *req_handle, void *context) {
    auto instance = reinterpret_cast<Driver *>(context);
    auto message = reinterpret_cast<Message *>(req_handle->get_req_msgbuf()->buf_);

    assert(message->type == RESULT);

    auto resultInfo = reinterpret_cast<ResultInfo *>(message->data);

    uint64_t latency;
    if (resultInfo->failed) {
      instance->numFailedTasks++;
      latency = -1;
    } else {
      latency = getCurrentTime() - resultInfo->creationTime;
    }

    if (resultInfo->taskID > instance->startRecording)
      instance->latencyHistory += latency;

    instance->numReceivedResponses++;

    instance->maxMigrations = std::max(instance->maxMigrations, resultInfo->numOfMigrations);
    if (resultInfo->numOfMigrations > 1)
      instance->numOverMigratedTasks++;

    auto &resp = req_handle->pre_resp_msgbuf_;
    instance->senderReceiverRPC->resize_msg_buffer(&resp, 1);
    instance->senderReceiverRPC->enqueue_response(req_handle, &resp);
}

void Driver::receiveReadyNotification(erpc::ReqHandle *req_handle, void *context) {
    auto instance = reinterpret_cast<Driver *>(context);
    instance->numReadyNotification++;
    auto message = reinterpret_cast<Message *>(req_handle->get_req_msgbuf()->buf_);
    assert(message->type == READY);
    BOOST_LOG_TRIVIAL(info)
            << "Driver ID: " << instance->getDriverID()
            << ": Got ready message from server=" << message->senderID
            << "!" << std::endl;
    auto &resp = req_handle->pre_resp_msgbuf_;
    instance->senderReceiverRPC->resize_msg_buffer(&resp, 1);
    instance->senderReceiverRPC->enqueue_response(req_handle, &resp);
}

[[noreturn]] void Driver::startRunning() {
    int firstCPUID = config.get_integer(address.getURI() + ".coreAffinityInit", 64);

    assert(numa_node_of_cpu(firstCPUID) == config.get_integer(address.getURI() + ".numaNode", 0));

    setCoreAffinity(firstCPUID);

    Message message{};
    message.type = TASK;
    message.senderID = driverID;
    int resets = 0;

    auto taskInfo = reinterpret_cast<TaskInfo *>(message.data);
    taskInfo->data = (char *)(message.data + sizeof(TaskInfo));
    taskInfo->driverID = driverID;
    taskInfo->numOfMigrations = 0;

    createdTaskCounter = 0;

    createSenderReceiverRPC();

    establishConnections();

    BOOST_LOG_TRIVIAL(info) << "Driver ID: " << driverID << " - Started driver!" << std::endl;

    uint64_t startTime = getCurrentTime();
    auto burstStartTime = startTime;

    while (true) {
        taskGenerationFunc(taskInfo->data, taskInfo->dataSize, taskInfo->taskType);
        message.dataSize = sizeof(TaskInfo) + taskInfo->dataSize;
        assert(message.dataSize < MAX_DATA_SIZE);
        taskInfo->taskID = createdTaskCounter++;

        auto resourceIndex = getResource();
        senderReceiverRPC->resize_msg_buffer(&senderReceiverContext.reqBufs[resourceIndex], MESSAGE_HEADER_SIZE + message.dataSize);
        std::memcpy(senderReceiverContext.reqBufs[resourceIndex].buf_, reinterpret_cast<uint8_t *>(&message), MESSAGE_HEADER_SIZE + message.dataSize);

        auto [tmp, interArrivalTime] = expRanGenerator.getNext();

        uint64_t currentTime = getCurrentTime();
        uint64_t nextDeparture = currentTime + (uint64_t)interArrivalTime;
        taskInfo->creationTime = nextDeparture;
        while (nextDeparture > currentTime) {
          senderReceiverRPC->run_event_loop_once();
          currentTime = getCurrentTime();
        }

        int recipient = sampleRecipient();
        assert(recipient < numServers);

        senderReceiverRPC->enqueue_request(
                sessionMap[recipient], ENQUEUE_TASK_ID,
                &senderReceiverContext.reqBufs[resourceIndex],
                &senderReceiverContext.respBufs[resourceIndex],
                [](void *context, void *tag) {
                    auto driver = static_cast<Driver *>(context);
                    driver->senderReceiverContext.putBack(reinterpret_cast<size_t>(tag));
                    driver->numSubmittedTasks++;
                },
                reinterpret_cast<void *>(resourceIndex));

        senderReceiverRPC->run_event_loop_once();

        numSentTasks++;

        if (currentTime - startTime > TEN_SECONDS_IN_NANOSECONDS) {
          startTime = currentTime;
          collectStats();

#ifdef FLUCTUATE_LOAD

          fluctuationCounter++;

#endif

        }



    }
}

inline size_t Driver::getResource() {
    while (!senderReceiverContext.slotIsAvailable())
        senderReceiverRPC->run_event_loop_once();

    auto resourceIndex = senderReceiverContext.get();
    return resourceIndex;
}

inline int Driver::sampleRecipient() {

#ifdef FLUCTUATE_LOAD

    if (fluctuationCounter >= 3)
        return departureShares[0](uRanGenerator);
    else
      return (int)uIntRanGenerator.getNext().second;

#else

    return (int)uIntRanGenerator.getNext().second;

#endif

}

inline void Driver::collectStats() const {
  std::cout << "Driver ID: " << driverID
            << " - Num sent Tasks: " << numSentTasks << std::endl;
  std::cout << "Driver ID: " << driverID
            << " - Num received responses: " << numReceivedResponses << std::endl;
  std::cout << "Driver ID: " << driverID
            << " - Num submitted tasks: " << numSubmittedTasks << std::endl;
  std::cout << "Driver ID: " << driverID
            << " - Maximum num migratedTasks: " << maxMigrations << std::endl;
  std::cout << "Driver ID: " << driverID
            << " - Num over-migrated tasks: " << numOverMigratedTasks << std::endl;
  std::cout << "Driver ID: " << driverID
            << " - Max failed tasks: " << numFailedTasks << std::endl;

  assert(numReceivedResponses <= numSentTasks);
}

void Driver::establishConnections() {
    BOOST_LOG_TRIVIAL(info) << "Driver ID: " << driverID
                            << " - Establishing connection to servers!"
                            << std::endl;
    std::vector<std::string> serversURIs =
            config.get_vector(address.getURI() + ".servers");

    for (auto &serverURI : serversURIs) {
        auto id = config.get_integer(serverURI + ".id");
        int sessionNum = senderReceiverRPC->create_session(serverURI, RECEIVER_RPC_ID);

        sessionMap[id] = sessionNum;
        serversEndPoints[id] = serverURI;

    }

    while (numSMResponses != numServers)
        senderReceiverRPC->run_event_loop_once();

    while (numReadyNotification < numServers)
      senderReceiverRPC->run_event_loop_once();
}

void Driver::createSenderReceiverRPC() {
    BOOST_LOG_TRIVIAL(info) << "Driver ID: " << driverID
                            << " - Create Sender RPC!" << std::endl;
    int physicalPortNumber = config.get_integer(
            address.getURI() + ".physicalPortNumber", address.getPortNumber() % 2);

    senderReceiverRPC =
            new erpc::Rpc<erpc::CTransport>(&nexus, static_cast<void *>(this),
                                            RECEIVER_RPC_ID, sm_handler, physicalPortNumber);

    // TODO: is this necessary?
    senderReceiverRPC->retry_connect_on_invalid_rpc_id_ = true;

    // TODO: is overallocation necessary?
    for (int i = 0; i < senderReceiverContext.maxOnFlyMsgs; i++) {
        senderReceiverContext.reqBufs[i] =
                senderReceiverRPC->alloc_msg_buffer_or_die(sizeof(Message));
        senderReceiverContext.respBufs[i] =
                senderReceiverRPC->alloc_msg_buffer_or_die(sizeof(Message));
    }
}
