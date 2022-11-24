#include "memory.h"

int alignedSize(int size, int alignment) {
  int aligned = size / alignment;
  if ((size % alignment) != 0)
    aligned++;
  return (aligned * alignment);
}

LearningSample::LearningSample(int numServers, int alignedPolyStateSize,
                               int alignedStateSize, int stateSize,
                               int polyStateSize)
    : numServers(numServers) {
  size_t alignedNumServers = alignedSize(numServers, 8);
  state = new (std::align_val_t(64)) float[alignedStateSize];
  statePolyFeatures = new (std::align_val_t(64)) float[alignedPolyStateSize];
  mProbabilities = new (std::align_val_t(64)) float[alignedNumServers];
  sProbabilities = new (std::align_val_t(64)) float[alignedNumServers];
  mActions = new (std::align_val_t(64)) int[numServers];
  sActions = new (std::align_val_t(64)) int[numServers];
  mLinearOutput = new (std::align_val_t(64)) float[alignedNumServers];
  sLinearOutput = new (std::align_val_t(64)) float[alignedNumServers];
  memset(mLinearOutput, 0, alignedNumServers * sizeof(*mLinearOutput));
  memset(sLinearOutput, 0, alignedNumServers * sizeof(*sLinearOutput));
  // setting padding to zero
  for (int i = 0; i < alignedStateSize; i++)
    state[i] = 0;
  // setting the bias
  state[stateSize - 1] = 1;
  // setting padding to zero
  for (int i = 0; i < alignedPolyStateSize; i++) {
    statePolyFeatures[i] = 0;
  }
  for (int i = 0; i < numServers; i++) {
    mActions[i] = 0;
    sActions[i] = 0;
  }
}

void LearningSample::clear() {
  memset(mActions, 0, numServers * sizeof(*mActions));
  memset(sActions, 0, numServers * sizeof(*mActions));

  mSum = 0;
  sSum = 0;

  serverNumOfStolenTasks = 0;
  serverNumOfMigratedTasks = 0;
  serverNumOfFailedTasks = 0;
  serverNumOfFailedStealingAttempts = 0;

  othersNumOfStolenTasks = 0;
  othersNumOfMigratedTasks = 0;
  othersNumOfFailedTasks = 0;
  othersNumOfFailedStealingAttempts = 0;

}

Memory::Memory(int numServers, int polyStateSize, int stateWeightsSize, int batchSize, int maxSamples)
    : maxSamples(maxSamples),
      batchSize(batchSize),
      polyStateSize(polyStateSize),
      alignedPolyStateSize(alignedSize(polyStateSize, 8)),
      stateSize(stateWeightsSize),
      alignedStateSize(alignedSize(stateWeightsSize, 8)) {

  void *raw_memory = operator new[](maxSamples * sizeof(LearningSample), std::align_val_t(64));
  samples = static_cast<LearningSample *>(raw_memory);
  for (int i = 0; i < maxSamples; i++) {
    new (&samples[i]) LearningSample(numServers, alignedPolyStateSize, alignedStateSize, stateSize, polyStateSize);
  }
}
