#ifndef BACKPROPTEST_MEMORY_H
#define BACKPROPTEST_MEMORY_H

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <iostream>

int alignedSize(int size, int alignment);

class LearningSample {
public:
  LearningSample(int numServers, int alignedPolySize, int alignedStateSize,
                 int stateSize, int polyStateSize);

  ~LearningSample() {
    delete state;
    delete statePolyFeatures;
    delete mActions;
    delete sActions;
  }

  void clear();

  float *state;
  float *statePolyFeatures;

  float *mProbabilities;
  float *sProbabilities;

  float *mLinearOutput;
  float *sLinearOutput;

  int *mActions;
  int *sActions;

  int mSum;
  int sSum;

  float reward = 0.0f;

  unsigned serverNumOfStolenTasks = 0;
  unsigned serverNumOfMigratedTasks = 0;
  unsigned serverNumOfFailedTasks = 0;
  unsigned serverNumOfFailedStealingAttempts = 0;

  unsigned othersNumOfStolenTasks = 0;
  unsigned othersNumOfMigratedTasks = 0;
  unsigned othersNumOfFailedTasks = 0;
  unsigned othersNumOfFailedStealingAttempts = 0;

  int numServers = 0;
};

// Memory is a shared object between the environment (server) and the agent.
// Memory is a circular buffer, the last element of which is used for training
// at next step.
class Memory {
public:
  Memory(int numServers, int polyStateSize, int stateWeightsSize, int batchSize = 4, int maxSamples = 20);

  int getNewIndex() {
    samples[currentSampleIndex % maxSamples].clear();
    return (currentSampleIndex++) % maxSamples;
  }

  unsigned getNextIndex(unsigned index) { return (++index) % maxSamples; }

  unsigned getPreviousIndex(unsigned index) { return (--index) % maxSamples; }

  void reset() { currentSampleIndex = 0; }

  float *getState(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].state;
  }

  void setState(float *state, int sampleIndex, size_t stateSize_) {
    assert(sampleIndex < maxSamples);
    assert(stateSize_ <= stateSize);
    std::memcpy(samples[sampleIndex].state, state,
                (stateSize_) * sizeof(float));
  }

  float *getPolyFeatures(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].statePolyFeatures;
  }

  void setPolyFeatures(float *polyState, int sampleIndex,
                       size_t polyStateSize_) {
    assert(sampleIndex < maxSamples);
    assert(polyStateSize == polyStateSize_);
    std::memcpy(samples[sampleIndex].statePolyFeatures, polyState,
                (polyStateSize) * sizeof(float));
  }

  float *getMigrationProbs(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].mProbabilities;
  }

  float *getStealingProbs(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].sProbabilities;
  }

  float *getMigrationLinearOutput(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].mLinearOutput;
  }

  float *getStealingLinearOutput(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].sLinearOutput;
  }

  void incrementServerStolenTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].serverNumOfStolenTasks++;
  }

  void incrementOthersStolenTasks(unsigned stolenTasks, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].othersNumOfStolenTasks += stolenTasks;
  }

  unsigned getServerStolenTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfStolenTasks;
  }

  unsigned getTotalNumOfStolenTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfStolenTasks +
           samples[sampleIndex].othersNumOfStolenTasks;
  }

  void incrementServerMigratedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].serverNumOfMigratedTasks++;
  }

  void incrementOthersMigratedTasks(unsigned migratedTasks, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].othersNumOfMigratedTasks += migratedTasks;
  }

  unsigned getServerMigratedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfMigratedTasks;
  }

  unsigned getTotalNumOfMigratedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfMigratedTasks +
           samples[sampleIndex].othersNumOfMigratedTasks;
  }

  void incrementServerFailedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].serverNumOfFailedTasks++;
  }

  void incrementOthersFailedTasks(unsigned failedTasks, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].othersNumOfFailedTasks += failedTasks;
  }

  unsigned getServerFailedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfFailedTasks;
  }

  unsigned getTotalNumOfFailedTasks(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfFailedTasks +
           samples[sampleIndex].othersNumOfFailedTasks;
  }

  void incrementServerFailedStealingAttempts(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].serverNumOfFailedStealingAttempts++;
  }

  void incrementOthersFailedStealingAttempts(unsigned failedStealingAttempts, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].othersNumOfFailedStealingAttempts += failedStealingAttempts;
  }

  unsigned getServerFailedStealingAttempts(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfFailedStealingAttempts;
  }

  unsigned getTotalNumOfFailedStealingAttempts(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].serverNumOfFailedStealingAttempts +
           samples[sampleIndex].othersNumOfFailedStealingAttempts;
  }

  int *getStealingActions(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].sActions;
  }

  int getSSum(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].sSum;
  }

  void incrementStealingAction(int actionIndex, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].sActions[actionIndex]++;
    samples[sampleIndex].sSum++;
  }

  int *getMigrationActions(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].mActions;
  }

  int getMSum(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].mSum;
  }

  void incrementMigrationsAction(int actionIndex, int sampleIndex) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].mActions[actionIndex]++;
    samples[sampleIndex].mSum++;
  }

  void setReward(int sampleIndex, float reward) {
    assert(sampleIndex < maxSamples);
    samples[sampleIndex].reward = reward;
  }

  float getReward(int sampleIndex) {
    assert(sampleIndex < maxSamples);
    return samples[sampleIndex].reward;
  }

  void print(int index) {
    std::cout << "State              : " << std::endl;
    for (int i = 0; i < stateSize; i++) {
      std::cout << samples[index].state[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "State Poly features: " << std::endl;
    for (int i = 0; i < polyStateSize; i++) {
      std::cout << samples[index].statePolyFeatures[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "ns Poly features   : " << std::endl;
    for (int i = 0; i < polyStateSize; i++) {
      std::cout << samples[getNextIndex(index)].statePolyFeatures[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "mActions           : " << std::endl;
    for (int i = 0; i < stateSize; i++) {
      std::cout << samples[index].mActions[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "sActions           : " << std::endl;
    for (int i = 0; i < stateSize; i++) {
      std::cout << samples[index].sActions[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "reward             : " << samples[index].reward << std::endl;
  }

  LearningSample *samples;
  int maxSamples;
  int batchSize;

  const int alignedPolyStateSize, alignedStateSize, stateSize, polyStateSize;

private:
  int currentSampleIndex = 0;
};

#endif // BACKPROPTEST_MEMORY_H
