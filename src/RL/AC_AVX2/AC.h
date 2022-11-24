#ifndef UNTITLED_AC_H
#define UNTITLED_AC_H

#include <cassert>
#include <cmath>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <memory>

#include "AVXMath.h"
#include "LRScheduler.h"
#include "memory.h"
#include "uniformInit.h"
#include "utils.h"


int alignedSize(int size, int alignment);

enum LRSchedulerType { CONSTANT, COSINEANNEALING };

class AC {
public:
  AC(int nServers, float criticLR, float mActorLR, float sActorLR, float gamma,
     size_t updatePeriod, size_t batchSize,
     int criticWeightsSize, int actorsWeightsSize,
     float expBeta = 1,
     float cLambda = 0,
     float aLambda = 0,
     float betaOne = 0.9, float betaTwo = 0.999, float weightDecay = 0.001,
     LRSchedulerType lrSchedulerType = CONSTANT, float minEta = 1e-4,
     float maxEta = 1e-2, int lrResetPeriod = 100, int lrWarmStartMultiplier = 2,
     bool adamW = true);
  ~AC();
  Memory * getSharedMemory();

  void initCritic(const float *weights, int size);
  void initActors(const float *mWeights, const float *sWeights);

  float predictCriticValue(const float *input, int size);
  void updateProbabilities(const float *input, float *mProbabilities, float *sProbabilities,
                           float *mLinearOutput, float *sLinearOutput,
                           const int inSize, const int outSize);
  void updateProbabilitiesNoVec(const float *input, float *mProbabilities, float *sProbabilities,
                                const int inSize, const int outSize);

  void newSampleReady(int sampleIndex);

  int getCriticParameters(float *parameters);

  void consensus(const float *parameters, int n);
  void consensusIncrementalStep(float *parameters, int size);
  void consensusFinalStep();

#ifdef TEST_AC
public:
#else
private:
#endif

  void cacheACUpdate(int sampleIndex, int nextSampleIndex);
  void cacheCriticUpdate(const float *input, int size, float advantage);
  void cacheActorsUpdate(const float *input,
                         const float *mProbabilities, const float *sProbabilities,
                         const int inSize, const int probabilitySize, const float advantage,
                         const int *mActions, const int *sActions,
                         const int mSum, const int sSum);

  void updateMiniBatchAdamW();
  void updateCriticMiniBatchAdamW();
  void updateActorsMiniBatchAdamW();

  void updateAC(int sampleIndex, int nextSampleIndex);
  void updateCritic(const float *input, int size, float advantage);
  void updateActors(const float *input,
                    const float *mProbabilities, const float *sProbabilities,
                    const int inSize, const int probabilitySize, const float advantage,
                    const int *mActions, const int *sActions,
                    const int mSum, const int sSum);

  void softmax(const float *mInput, float *mProbabilities,
               const float *sInput, float *sProbabilities,
               const int inSize, const int outSize) const;

  bool miniBatchAdamW = false;

  int numOfServers;
  int alignedNumOfServers;
  LRScheduler *lrScheduler;

  // avg Reward for continuing tasks
  float avgReward = 0;
  float avgRewardStepSize;

  // TD(lambda)
  float criticTDLambda = 0;
  float actorsTDLambda = 0;

  // Critic parameters
  int alignedCriticWeightsSize;
  int criticWeightsSize;
  float criticLearningRate;
  float *criticWeights;
  float *criticCachedGradiant;
  float *criticEligibility;
  float *criticCachedConsensusWeights;
  int numAvailableConsensusData = 0;

  // actors parameters
  int alignedActorsWeightsSize;
  int actorsWeightsSize;
  float mActorLearningRate;
  float sActorLearningRate;
  float **mActorWeights;
  float **sActorWeights;
  float **mActorEligibility;
  float **sActorEligibility;
  float **mActorCachedGradiant;
  float **sActorCachedGradiant;

  Memory * sampleMemory;
  size_t batchSize, updatePeriod;
  float expBeta;
  unsigned availableSamples = 0;

  // used by Adam
  int learningStep = 0;
  float betaOne, betaTwo;
  float weightDecay;

  // exp average
  float *criticExpAvg;
  float *criticSqrExpAvg;
  float **mActorExpAvg;
  float **sActorExpAvg;
  float **mActorSqrExpAvg;
  float **sActorSqrExpAvg;


};

#endif // UNTITLED_AC_H
