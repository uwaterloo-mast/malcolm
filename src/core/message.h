#ifndef DLB_MESSAGE_H
#define DLB_MESSAGE_H

#include <cstdlib>

#include "endpointAddr.h"

#define MAX_CONS_SIZE 600 // IMPORTANT
#define MAX_DATA_SIZE (MAX_CONS_SIZE * 4)

enum MessageType { TASK, LOAD, RESULT, CONSENSUS, STEAL, READY };

struct Message {
  int senderID;
  MessageType type;
  size_t dataSize;
  char data[MAX_DATA_SIZE];
};

struct LoadInfo {
  int serverID;
  float load;
  int queueSize;
  unsigned stolenTasks = 0;
  unsigned migratedTasks = 0;
  unsigned failedTasks = 0;
  unsigned failedStealingAttempts = 0;
};

struct TaskInfo {
  uint8_t taskType = 0;
  unsigned taskID;
  int driverID;
  int numOfMigrations = 0;
  uint64_t creationTime;
  size_t dataSize;
  char *data;
};

struct ResultInfo {
  int serverID;
  unsigned taskID;
  bool failed = false;
  int numOfMigrations;
  uint8_t  taskType;
  uint64_t creationTime;
  uint64_t executionTimeNs;
  size_t dataSize;
  char *data;
};

struct Packet {
  Message message;
  int rpcType;
  int destination;
};

struct ConsensusInfo {
  float parameters[MAX_CONS_SIZE];
};

#define MESSAGE_HEADER_SIZE (sizeof(Message) - MAX_DATA_SIZE)

#endif // DLB_MESSAGE_H
