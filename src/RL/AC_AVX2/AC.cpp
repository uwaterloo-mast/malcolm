#include "AC.h"

AC::AC(int nServers, float criticLR, float mActorLR, float sActorLR,
       float gamma, size_t uPeriod, size_t batchSize,
       int cWeightsSize, int actorsWeightsSize,
       float expBeta, float cLambda, float aLambda, float betaOne,
       float betaTwo, float weightDecay, LRSchedulerType lrSchedulerType,
       float minEta, float maxEta, int lrResetPeriod, int lrWarmStartMultiplier,
       bool adamW) :
       numOfServers(nServers),
       criticWeightsSize(cWeightsSize),
       criticLearningRate(criticLR),
       mActorLearningRate(mActorLR),
       actorsWeightsSize(actorsWeightsSize),
       criticTDLambda(cLambda),
       actorsTDLambda(aLambda),
       sActorLearningRate(sActorLR),
       avgRewardStepSize(gamma),
       batchSize(batchSize),
       expBeta(expBeta),
       updatePeriod(uPeriod),
       betaOne(betaOne),
       betaTwo(betaTwo),
       weightDecay(weightDecay),
       miniBatchAdamW(adamW) {

  float mean = 0;

  alignedNumOfServers = alignedSize(numOfServers, 8);
  alignedCriticWeightsSize = alignedSize(criticWeightsSize, 8);
  alignedActorsWeightsSize = alignedSize(actorsWeightsSize, 8);

  criticWeights = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  UniformInit::generateData(criticWeights, criticWeightsSize, criticWeightsSize, mean);
  memset(criticWeights + criticWeightsSize, 0.0f, (alignedCriticWeightsSize - criticWeightsSize) * sizeof(float));

  criticEligibility = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  memset(criticEligibility, 0.0f, alignedCriticWeightsSize * sizeof(float));

  criticCachedGradiant = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  memset(criticCachedGradiant, 0.0f, alignedCriticWeightsSize * sizeof(float));

  criticCachedConsensusWeights = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  memset(criticCachedConsensusWeights, 0.0f, alignedCriticWeightsSize * sizeof(float));

  criticExpAvg = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  memset(criticExpAvg, 0, alignedCriticWeightsSize * sizeof(float));

  criticSqrExpAvg = new (std::align_val_t(64)) float[alignedCriticWeightsSize];
  memset(criticSqrExpAvg, 0, alignedCriticWeightsSize * sizeof(float));


  // Setup actors
  mActorWeights = new (std::align_val_t(64))float *[numOfServers];
  mActorEligibility = new (std::align_val_t(64))float *[numOfServers];
  mActorCachedGradiant = new (std::align_val_t(64))float *[numOfServers];
  mActorExpAvg = new (std::align_val_t(64)) float *[numOfServers];
  mActorSqrExpAvg = new (std::align_val_t(64))float *[numOfServers];

  sActorWeights = new (std::align_val_t(64))float *[numOfServers];
  sActorEligibility = new (std::align_val_t(64))float *[numOfServers];
  sActorCachedGradiant = new (std::align_val_t(64))float *[numOfServers];
  sActorExpAvg = new (std::align_val_t(64))float *[numOfServers];
  sActorSqrExpAvg = new (std::align_val_t(64)) float *[numOfServers];

  for (int i = 0; i < numOfServers; i++) {
    mActorWeights[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    UniformInit::generateData(mActorWeights[i], actorsWeightsSize, actorsWeightsSize, mean);
    memset(mActorWeights[i] + actorsWeightsSize, 0.0f, (alignedActorsWeightsSize - actorsWeightsSize) * sizeof(float));
//    memset(mActorWeights[i], 0.0f, (alignedActorsWeightsSize) * sizeof(float));
//    mActorWeights[i][i] = 1;

    mActorEligibility[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(mActorEligibility[i], 0, alignedActorsWeightsSize * sizeof(float));

    mActorCachedGradiant[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(mActorCachedGradiant[i], 0, alignedActorsWeightsSize * sizeof(float));

    mActorExpAvg[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(mActorExpAvg[i], 0.0f, alignedActorsWeightsSize * sizeof(float));

    mActorSqrExpAvg[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(mActorSqrExpAvg[i], 0.0f, alignedActorsWeightsSize * sizeof(float));

    sActorWeights[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    UniformInit::generateData(sActorWeights[i], actorsWeightsSize, actorsWeightsSize, mean);
    memset(sActorWeights[i] + actorsWeightsSize, 0.0f, (alignedActorsWeightsSize - actorsWeightsSize) * sizeof(float));

    sActorEligibility[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(sActorEligibility[i], 0, alignedActorsWeightsSize * sizeof(float));

    sActorCachedGradiant[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(sActorCachedGradiant[i], 0, alignedActorsWeightsSize * sizeof(float));

    sActorExpAvg[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(sActorExpAvg[i], 0.0f, alignedActorsWeightsSize * sizeof(float));

    sActorSqrExpAvg[i] = new (std::align_val_t(64)) float[alignedActorsWeightsSize];
    memset(sActorSqrExpAvg[i], 0.0f, alignedActorsWeightsSize * sizeof(float));

  }

  sampleMemory = new Memory(numOfServers, criticWeightsSize, actorsWeightsSize, batchSize, updatePeriod);

  if (lrSchedulerType == CONSTANT) {
    lrScheduler = new ConstantLR();
  } else if (lrSchedulerType == COSINEANNEALING) {
    lrScheduler = new CosineAnnealingLR(minEta, maxEta, lrResetPeriod, lrWarmStartMultiplier);
  }
}

AC::~AC() {
  delete criticWeights;
  delete criticCachedGradiant;
  delete criticEligibility;
  delete criticCachedConsensusWeights;
  delete criticExpAvg;
  delete criticSqrExpAvg;

  for (int i = 0; i < numOfServers; i++) {
    delete mActorWeights[i];
    delete mActorEligibility[i];
    delete mActorCachedGradiant[i];
    delete mActorExpAvg[i];
    delete mActorSqrExpAvg[i];

    delete sActorWeights[i];
    delete sActorEligibility[i];
    delete sActorCachedGradiant[i];
    delete sActorExpAvg[i];
    delete sActorSqrExpAvg[i];
  }

  delete mActorWeights;
  delete mActorEligibility;
  delete mActorCachedGradiant;
  delete mActorExpAvg;
  delete mActorSqrExpAvg;

  delete sActorWeights;
  delete sActorEligibility;
  delete sActorCachedGradiant;
  delete sActorExpAvg;
  delete sActorSqrExpAvg;
}

// Used only for testing
void AC::initCritic(const float *weights, const int size) {
  assert(size <= alignedCriticWeightsSize);
  for (int i = 0; i < size; i++)
    criticWeights[i] = weights[i];
}

// Input should be 32-byte aligned
float AC::predictCriticValue(const float *input, const int size) {
  assert(size == alignedCriticWeightsSize);
  __m256 resultV = _mm256_setzero_ps();
  for (int i = 0; i < size / 8; i++) {
    __m256 inputV = _mm256_load_ps(&input[i * 8]);
    __m256 weightV = _mm256_load_ps(&criticWeights[i * 8]);
    resultV = _mm256_add_ps(resultV, _mm256_mul_ps(inputV, weightV));
  }
  return _mm256_reduce_add_ps(resultV);
}

// Used only for testing
void AC::initActors(const float *mWeights, const float *sWeights) {
  int k = 0;
  for (int i = 0; i < numOfServers; i++) {
    for (int j = 0; j < actorsWeightsSize - 1; j++) {
      mActorWeights[i][j] = mWeights[k];
      sActorWeights[i][j] = sWeights[k++];
    }
  }
  for (int i = 0; i < numOfServers; i++) {
    mActorWeights[i][actorsWeightsSize - 1] = mWeights[k];
    sActorWeights[i][actorsWeightsSize - 1] = sWeights[k++];
  }
}

// Used only for performance testing
void AC::updateProbabilitiesNoVec(const float *input,
                                  float *mOutput, float *sOutput,
                                  const int inSize, const int outSize) {
  assert(inSize == alignedActorsWeightsSize);
  assert(outSize == numOfServers);
  float mDenominator = 0;
  float sDenominator = 0;
  for (int i = 0; i < outSize; i++) {
    __m256 mDenominatorV = _mm256_setzero_ps();
    __m256 sDenominatorV = _mm256_setzero_ps();
    for (int j = 0; j < inSize / 8; j++) {
      __m256 inputV = _mm256_load_ps(&input[j * 8]);
      __m256 mWeightV = _mm256_load_ps(&mActorWeights[i][j * 8]);
      __m256 sWeightV = _mm256_load_ps(&sActorWeights[i][j * 8]);
      mDenominatorV = _mm256_add_ps(mDenominatorV, _mm256_mul_ps(inputV, mWeightV));
      sDenominatorV = _mm256_add_ps(sDenominatorV, _mm256_mul_ps(inputV, sWeightV));
    }
    mOutput[i] = fast_exp(_mm256_reduce_add_ps(mDenominatorV));
    sOutput[i] = fast_exp(_mm256_reduce_add_ps(sDenominatorV));
    mDenominator += mOutput[i];
    sDenominator += sOutput[i];
  }

  for (int i = 0; i < outSize; i++) {
    mOutput[i] = mOutput[i] / mDenominator;
    sOutput[i] = sOutput[i] / sDenominator;
  }
}

void AC::updateProbabilities(const float *input,
                             float *mProbabilities, float *sProbabilities,
                             float *mLinearOutput, float *sLinearOutput,
                             const int inSize, const int outSize) {

  assert(inSize == alignedActorsWeightsSize);
  assert(outSize == numOfServers);
  for (int i = 0; i < outSize; i++) {
    __m256 mDenominatorV = _mm256_setzero_ps();
    __m256 sDenominatorV = _mm256_setzero_ps();
    for (int j = 0; j < inSize / 8; j++) {
      __m256 inputV = _mm256_load_ps(&input[j * 8]);

      __m256 mWeightV = _mm256_load_ps(&mActorWeights[i][j * 8]);
      mDenominatorV = _mm256_add_ps(mDenominatorV, _mm256_mul_ps(inputV, mWeightV));

      __m256 sWeightV = _mm256_load_ps(&sActorWeights[i][j * 8]);
      sDenominatorV = _mm256_add_ps(sDenominatorV, _mm256_mul_ps(inputV, sWeightV));
    }
    mLinearOutput[i] = expBeta * _mm256_reduce_add_ps(mDenominatorV);
    sLinearOutput[i] = expBeta * _mm256_reduce_add_ps(sDenominatorV);
  }
  softmax(mLinearOutput, mProbabilities, sLinearOutput, sProbabilities, inSize, outSize);
}

void AC::softmax(const float *mInput, float *mProbabilities,
                 const float *sInput, float *sProbabilities,
                 const int inSize, const int outSize) const {
  assert(inSize == alignedActorsWeightsSize);
  assert(outSize == numOfServers);

  float mDenominator, sDenominator;

  __m256 mDenominatorV = _mm256_setzero_ps();
  __m256 sDenominatorV = _mm256_setzero_ps();

  for (int i = 0; i < alignedNumOfServers / 8; i++) {
    __m256 mInputV = _mm256_load_ps(&mInput[i * 8]);
    __m256 mExpV = _mm256_exp_ps(mInputV);
    _mm256_store_ps(&mProbabilities[i * 8], mExpV);
    mDenominatorV = _mm256_add_ps(mDenominatorV, mExpV);

    __m256 sInputV = _mm256_load_ps(&sInput[i * 8]);
    __m256 sExpV = _mm256_exp_ps(sInputV);
    _mm256_store_ps(&sProbabilities[i * 8], sExpV);
    sDenominatorV = _mm256_add_ps(sDenominatorV, sExpV);
  }

  // m/sInput is zero for alignedNumOfServers - numOfServers elements
  mDenominator = _mm256_reduce_add_ps(mDenominatorV) - (float)(alignedNumOfServers - numOfServers);
  sDenominator = _mm256_reduce_add_ps(sDenominatorV) - (float)(alignedNumOfServers - numOfServers);

  mDenominatorV = _mm256_set1_ps(mDenominator);
  sDenominatorV = _mm256_set1_ps(sDenominator);

  for (int i = 0; i < alignedNumOfServers / 8; i++) {
    __m256 mInputV = _mm256_load_ps(&mProbabilities[i * 8]);
    __m256 mProbV = _mm256_div_ps(mInputV, mDenominatorV);
    _mm256_store_ps(&mProbabilities[i * 8], mProbV);

    __m256 sInputV = _mm256_load_ps(&sProbabilities[i * 8]);
    __m256 sProbV = _mm256_div_ps(sInputV, sDenominatorV);
    _mm256_store_ps(&sProbabilities[i * 8], sProbV);
  }
}

void AC::updateCriticMiniBatchAdamW(){
  const __m256 b1V = _mm256_set1_ps(betaOne);
  const __m256 b2V = _mm256_set1_ps(betaTwo);

  const __m256 oneSubB1V = _mm256_set1_ps(1 - betaOne);
  const __m256 oneSubB2V = _mm256_set1_ps(1 - betaTwo);

  const __m256 b1BiasCorV = _mm256_set1_ps(1 / (float)(1 - pow(betaOne, learningStep)));
  const __m256 b2BiasCorV = _mm256_set1_ps(1 / (float)(1 - pow(betaTwo, learningStep)));

  const __m256 epsilonV = _mm256_set1_ps(1.0e-6);
  const float eta = lrScheduler->getLearningRateMultiplier();
  const __m256 oneSubWeightDecayV = _mm256_set1_ps(1 - criticLearningRate * weightDecay * eta);

  const __m256 criticLearningRateV = _mm256_set1_ps(eta * criticLearningRate);

  for (int i = 0; i < alignedCriticWeightsSize / 8; i++) {
    __m256 gradiantV = _mm256_load_ps(&criticCachedGradiant[i * 8]);
    __m256 sqrGradiantV = _mm256_mul_ps(gradiantV, gradiantV);
    __m256 expAvgV = _mm256_load_ps(&criticExpAvg[i * 8]);
    __m256 sqrExpAvgV = _mm256_load_ps(&criticSqrExpAvg[i * 8]);
    expAvgV = _mm256_add_ps(_mm256_mul_ps(b1V, expAvgV), _mm256_mul_ps(oneSubB1V, gradiantV));
    sqrExpAvgV = _mm256_add_ps(_mm256_mul_ps(b2V, sqrExpAvgV), _mm256_mul_ps(oneSubB2V, sqrGradiantV));

    _mm256_store_ps(&criticExpAvg[i * 8], expAvgV);
    _mm256_store_ps(&criticSqrExpAvg[i * 8], sqrExpAvgV);

    __m256 biasCorrectedExpAvgV = _mm256_mul_ps(expAvgV, b1BiasCorV);
    __m256 biasCorrectedSqrExpAvgV = _mm256_mul_ps(sqrExpAvgV, b2BiasCorV);

    __m256 weightV = _mm256_load_ps(&criticWeights[i * 8]);
    weightV = _mm256_mul_ps(weightV, oneSubWeightDecayV);
    __m256 deltaWeightV = _mm256_mul_ps(criticLearningRateV, _mm256_div_ps(biasCorrectedExpAvgV, _mm256_add_ps(_mm256_sqrt_ps(biasCorrectedSqrExpAvgV), epsilonV)));
    _mm256_store_ps(&criticWeights[i * 8], _mm256_add_ps(weightV, deltaWeightV));
  }
  memset(criticCachedGradiant, 0, alignedCriticWeightsSize * sizeof(float));
//  memset(criticEligibility, 0, alignedCriticWeightsSize * sizeof(float));

}


void AC::updateActorsMiniBatchAdamW(){
  const __m256 b1V = _mm256_set1_ps(betaOne);
  const __m256 b2V = _mm256_set1_ps(betaTwo);

  const __m256 oneSubB1V = _mm256_set1_ps(1 - betaOne);
  const __m256 oneSubB2V = _mm256_set1_ps(1 - betaTwo);

  const __m256 b1BiasCorV = _mm256_set1_ps(1 / (float)(1 - pow(betaOne, learningStep)));
  const __m256 b2BiasCorV = _mm256_set1_ps(1 / (float)(1 - pow(betaTwo, learningStep)));

  const __m256 epsilonV = _mm256_set1_ps(1.0e-6);
  const float eta = lrScheduler->getLearningRateMultiplier();
  const __m256 mOneSubWeightDecayV = _mm256_set1_ps(1 - mActorLearningRate * weightDecay * eta);
  const __m256 sOneSubWeightDecayV = _mm256_set1_ps(1 - sActorLearningRate * weightDecay * eta);

  const __m256 mActorLearningRateV = _mm256_set1_ps(eta * mActorLearningRate);
  const __m256 sActorLearningRateV = _mm256_set1_ps(eta * sActorLearningRate);

  for (int i = 0; i < numOfServers; i++) {
    for (int j = 0; j < alignedActorsWeightsSize / 8; j++) {
      // updating migration actor!
      __m256 mGradiantV = _mm256_load_ps(&mActorCachedGradiant[i][j * 8]);
      __m256 mSqrGradiantV = _mm256_mul_ps(mGradiantV, mGradiantV);

      __m256 mExpAvgV = _mm256_load_ps(&mActorExpAvg[i][j * 8]);
      __m256 mSqrExpAvgV = _mm256_load_ps(&mActorSqrExpAvg[i][j * 8]);

      mExpAvgV = _mm256_add_ps(_mm256_mul_ps(b1V, mExpAvgV), _mm256_mul_ps(oneSubB1V, mGradiantV));
      mSqrExpAvgV = _mm256_add_ps(_mm256_mul_ps(b2V, mSqrExpAvgV), _mm256_mul_ps(oneSubB2V, mSqrGradiantV));

      _mm256_store_ps(&mActorExpAvg[i][j * 8], mExpAvgV);
      _mm256_store_ps(&mActorSqrExpAvg[i][j * 8], mSqrExpAvgV);

      __m256 mBiasCorExpAvgV = _mm256_mul_ps(mExpAvgV, b1BiasCorV);
      __m256 mBiasCorSqrExpAvgV = _mm256_mul_ps(mSqrExpAvgV, b2BiasCorV);

      __m256 mWeightV = _mm256_load_ps(&mActorWeights[i][j * 8]);
      mWeightV = _mm256_mul_ps(mWeightV, mOneSubWeightDecayV);

      __m256 mDeltaWeightV = _mm256_mul_ps(mActorLearningRateV, _mm256_div_ps(mBiasCorExpAvgV, _mm256_add_ps(_mm256_sqrt_ps(mBiasCorSqrExpAvgV), epsilonV)));
      _mm256_store_ps(&mActorWeights[i][j * 8], _mm256_add_ps(mWeightV, mDeltaWeightV));

      // updating stealing actor!
      __m256 sGradiantV = _mm256_load_ps(&sActorCachedGradiant[i][j * 8]);
      __m256 sSquareGradiantV = _mm256_mul_ps(sGradiantV, sGradiantV);

      __m256 sExpAvgV = _mm256_load_ps(&sActorExpAvg[i][j * 8]);
      __m256 sSqrExpAvgV = _mm256_load_ps(&sActorSqrExpAvg[i][j * 8]);

      sExpAvgV = _mm256_add_ps(_mm256_mul_ps(b1V, sExpAvgV), _mm256_mul_ps(oneSubB1V, sGradiantV));
      sSqrExpAvgV = _mm256_add_ps(_mm256_mul_ps(b2V, sSqrExpAvgV), _mm256_mul_ps(oneSubB2V, sSquareGradiantV));

      _mm256_store_ps(&sActorExpAvg[i][j * 8], sExpAvgV);
      _mm256_store_ps(&sActorSqrExpAvg[i][j * 8], sSqrExpAvgV);

      __m256 sBiasCorExpAvgV = _mm256_mul_ps(sExpAvgV, b1BiasCorV);
      __m256 sBiasCorSqrExpAvgV = _mm256_mul_ps(sSqrExpAvgV, b2BiasCorV);

      __m256 sWeightV = _mm256_load_ps(&sActorWeights[i][j * 8]);
      sWeightV = _mm256_mul_ps(sWeightV, sOneSubWeightDecayV);

      __m256 sDeltaWeightV = _mm256_mul_ps(sActorLearningRateV, _mm256_div_ps(sBiasCorExpAvgV, _mm256_add_ps(_mm256_sqrt_ps(sBiasCorSqrExpAvgV), epsilonV)));
      _mm256_store_ps(&sActorWeights[i][j * 8], _mm256_add_ps(sWeightV, sDeltaWeightV));
    }
    memset(mActorCachedGradiant[i], 0.0f, alignedActorsWeightsSize * sizeof(float));
    memset(sActorCachedGradiant[i], 0.0f, alignedActorsWeightsSize * sizeof(float));

//    memset(mActorEligibility[i], 0.0f, alignedActorsWeightsSize * sizeof(float));
//    memset(sActorEligibility[i], 0.0f, alignedActorsWeightsSize * sizeof(float));
  }
}

void AC::updateMiniBatchAdamW() {
  learningStep++;
  updateCriticMiniBatchAdamW();
  updateActorsMiniBatchAdamW();
}

void AC::newSampleReady(int sampleIndex) {
  availableSamples++;
  auto nextSampleIndex = (int)sampleMemory->getNextIndex(sampleIndex);

  if (miniBatchAdamW) {
    cacheACUpdate(sampleIndex, nextSampleIndex);
    if (availableSamples % batchSize == 0)
      updateMiniBatchAdamW();
  } else {
    updateAC(sampleIndex, nextSampleIndex);
  }

  // Get probabilities for new state
  updateProbabilities(sampleMemory->getState(nextSampleIndex),
                      sampleMemory->getMigrationProbs(nextSampleIndex),
                      sampleMemory->getStealingProbs(nextSampleIndex),
                      sampleMemory->getMigrationLinearOutput(nextSampleIndex),
                      sampleMemory->getStealingLinearOutput(nextSampleIndex),
                      sampleMemory->alignedStateSize,
                      numOfServers);
}

void AC::cacheACUpdate(int sampleIndex, int nextSampleIndex) {
  float advantage = sampleMemory->getReward(sampleIndex) - avgReward -
      predictCriticValue(sampleMemory->getPolyFeatures(sampleIndex),
                         sampleMemory->alignedPolyStateSize) +
      predictCriticValue(sampleMemory->getPolyFeatures(nextSampleIndex),
                         sampleMemory->alignedPolyStateSize);

  avgReward = avgReward + avgRewardStepSize * advantage;

  cacheCriticUpdate(sampleMemory->getPolyFeatures(sampleIndex), sampleMemory->alignedPolyStateSize, advantage);

  cacheActorsUpdate(sampleMemory->getState(sampleIndex),
                    sampleMemory->getMigrationProbs(sampleIndex),
                    sampleMemory->getStealingProbs(sampleIndex),
                    sampleMemory->alignedStateSize,
                    numOfServers,
                    advantage,
                    sampleMemory->getMigrationActions(sampleIndex),
                    sampleMemory->getStealingActions(sampleIndex),
                    sampleMemory->getMSum(sampleIndex),
                    sampleMemory->getSSum(sampleIndex));
}


void AC::updateAC(int sampleIndex, int nextSampleIndex) {
  float advantage = sampleMemory->getReward(sampleIndex) - avgReward -
      predictCriticValue(sampleMemory->getPolyFeatures(sampleIndex),
                         sampleMemory->alignedPolyStateSize) +
      predictCriticValue(sampleMemory->getPolyFeatures(nextSampleIndex),
                         sampleMemory->alignedPolyStateSize);

  avgReward = avgReward + avgRewardStepSize * advantage;

  updateCritic(sampleMemory->getPolyFeatures(sampleIndex), sampleMemory->alignedPolyStateSize, advantage);

  updateActors(sampleMemory->getState(sampleIndex),
               sampleMemory->getMigrationProbs(sampleIndex),
               sampleMemory->getStealingProbs(sampleIndex),
               sampleMemory->alignedStateSize,
               numOfServers,
               advantage,
               sampleMemory->getMigrationActions(sampleIndex),
               sampleMemory->getStealingActions(sampleIndex),
               sampleMemory->getMSum(sampleIndex),
               sampleMemory->getSSum(sampleIndex));
}

void AC::cacheCriticUpdate(const float *input, int size, float advantage) {
  __m256 deltaV = _mm256_set1_ps(advantage / (float)batchSize);
  __m256 lambdaVector = _mm256_set1_ps(criticTDLambda);
  for (int i = 0; i < size / 8; i++) {
    __m256 gradiantV = _mm256_load_ps(&input[i * 8]);
    __m256 eligibilityV = _mm256_load_ps(&criticEligibility[i * 8]);
    eligibilityV = _mm256_add_ps(_mm256_mul_ps(eligibilityV, lambdaVector), gradiantV);
    __m256 cachedGradiantV = _mm256_load_ps(&criticCachedGradiant[i * 8]);
    _mm256_store_ps(&criticCachedGradiant[i * 8], _mm256_add_ps(cachedGradiantV, _mm256_mul_ps(deltaV, eligibilityV)));
    _mm256_store_ps(&criticEligibility[i * 8], eligibilityV);
  }
}

void AC::updateCritic(const float *input, int size, float advantage) {
  __m256 deltaV = _mm256_set1_ps(criticLearningRate * advantage);
  __m256 lambdaVector = _mm256_set1_ps(criticTDLambda);
  for (int i = 0; i < size / 8; i++) {
    __m256 gradiantV = _mm256_load_ps(&input[i * 8]);
    __m256 eligibilityV = _mm256_load_ps(&criticEligibility[i * 8]);
    eligibilityV = _mm256_add_ps(_mm256_mul_ps(eligibilityV, lambdaVector), gradiantV);
    __m256 weightV = _mm256_load_ps(&criticWeights[i * 8]);
    _mm256_store_ps(&criticWeights[i * 8], _mm256_add_ps(weightV, _mm256_mul_ps(deltaV, eligibilityV)));
    _mm256_store_ps(&criticEligibility[i * 8], eligibilityV);
  }
}

void AC::cacheActorsUpdate(const float *input, const float *mProbabilities,
                           const float *sProbabilities, const int inSize,
                           const int probabilitySize, const float advantage,
                           const int *mActions, const int *sActions,
                           const int mSum, const int sSum) {

  assert(inSize == alignedActorsWeightsSize);
  assert(probabilitySize == numOfServers);

  __m256 deltaV = _mm256_set1_ps(advantage / (float)batchSize);
  __m256 lambdaVector = _mm256_set1_ps(actorsTDLambda);

  for (int i = 0; i < probabilitySize; i++) {
    __m256 mGradiantV1 = (mSum != 0) ? _mm256_set1_ps(((float)mActions[i] / mSum - mProbabilities[i]) * expBeta) : _mm256_setzero_ps();
    __m256 sGradiantV1 = (sSum != 0) ? _mm256_set1_ps(((float)sActions[i] / sSum - sProbabilities[i]) * expBeta) : _mm256_setzero_ps();
    for (int j = 0; j < inSize / 8; j++) {
      __m256 inputV = _mm256_load_ps(&input[j * 8]);

      __m256 mGradiantV2 = _mm256_mul_ps(mGradiantV1, inputV);
      __m256 mEligibilityV = _mm256_load_ps(&mActorEligibility[i][j * 8]);
      mEligibilityV = _mm256_add_ps(_mm256_mul_ps(mEligibilityV, lambdaVector), mGradiantV2);
      __m256 mCachedDeltaWeightV = _mm256_load_ps(&mActorCachedGradiant[i][j * 8]);
      _mm256_store_ps(&mActorCachedGradiant[i][j * 8], _mm256_add_ps(mCachedDeltaWeightV, _mm256_mul_ps(deltaV, mEligibilityV)));
      _mm256_store_ps(&mActorEligibility[i][j * 8], mEligibilityV);

      __m256 sGradiantV2 = _mm256_mul_ps(sGradiantV1, inputV);
      __m256 sEligibilityV = _mm256_load_ps(&sActorEligibility[i][j * 8]);
      sEligibilityV = _mm256_add_ps(_mm256_mul_ps(sEligibilityV, lambdaVector), sGradiantV2);
      __m256 sCachedDeltaWeightV = _mm256_load_ps(&sActorCachedGradiant[i][j * 8]);
      _mm256_store_ps(&sActorCachedGradiant[i][j * 8], _mm256_add_ps(sCachedDeltaWeightV, _mm256_mul_ps(deltaV, sEligibilityV)));
      _mm256_store_ps(&sActorEligibility[i][j * 8], sEligibilityV);
    }
  }
}

void AC::updateActors(const float *input, const float *mProbabilities,
                      const float *sProbabilities, const int inSize,
                      const int probabilitySize, const float advantage,
                      const int *mActions, const int *sActions,
                      const int mSum, const int sSum) {

  assert(inSize == alignedActorsWeightsSize);
  assert(probabilitySize == numOfServers);

  __m256 mDeltaV = _mm256_set1_ps(mActorLearningRate * advantage);
  __m256 sDeltaV = _mm256_set1_ps(sActorLearningRate * advantage);
  __m256 lambdaVector = _mm256_set1_ps(actorsTDLambda);

  for (int i = 0; i < probabilitySize; i++) {
    __m256 mGradiantV1 = (mSum != 0) ? _mm256_set1_ps(((float)mActions[i] / mSum - mProbabilities[i]) * expBeta) : _mm256_setzero_ps();
    __m256 sGradiantV1 = (sSum != 0) ? _mm256_set1_ps(((float)sActions[i] / sSum - sProbabilities[i]) * expBeta) : _mm256_setzero_ps();

    for (int j = 0; j < inSize / 8; j++) {
      __m256 inputV = _mm256_load_ps(&input[j * 8]);

      __m256 mGradiantV2 = _mm256_mul_ps(mGradiantV1, inputV);
      __m256 mEligibilityV = _mm256_load_ps(&mActorEligibility[i][j * 8]);
      mEligibilityV = _mm256_add_ps(_mm256_mul_ps(mEligibilityV, lambdaVector), mGradiantV2);
      __m256 mWeightV = _mm256_load_ps(&mActorWeights[i][j * 8]);
      _mm256_store_ps(&mActorWeights[i][j * 8], _mm256_add_ps(mWeightV, _mm256_mul_ps(mDeltaV, mEligibilityV)));
      _mm256_store_ps(&mActorEligibility[i][j * 8], mEligibilityV);

      __m256 sGradiantV2 = _mm256_mul_ps(sGradiantV1, inputV);
      __m256 sEligibilityV = _mm256_load_ps(&sActorEligibility[i][j * 8]);
      sEligibilityV = _mm256_add_ps(_mm256_mul_ps(sEligibilityV, lambdaVector), sGradiantV2);
      __m256 sWeightV = _mm256_load_ps(&sActorWeights[i][j * 8]);
      _mm256_store_ps(&sActorWeights[i][j * 8], _mm256_add_ps(sWeightV, _mm256_mul_ps(sDeltaV, sEligibilityV)));
      _mm256_store_ps(&sActorEligibility[i][j * 8], sEligibilityV);
    }
  }
}

Memory * AC::getSharedMemory() { return sampleMemory; }

int AC::getCriticParameters(float *parameters) {
  std::memcpy(parameters, criticWeights, sizeof(float) * alignedCriticWeightsSize);
  return (int)(sizeof(float)) * alignedCriticWeightsSize;
}

// deprecated!
void AC::consensus(const float *parameters, int n) {
  if (n > 0)
    for (int i = 0; i < ((numOfServers + 1) * (numOfServers + 2)) / 2; ++i)
      criticWeights[i] = (criticWeights[i] + parameters[i]) / (float)(n + 1);
}

void AC::consensusIncrementalStep(float *parameters, int size) {
  assert(size == alignedCriticWeightsSize);
  for (int i = 0; i < alignedCriticWeightsSize / 8; i++) {
    __m256 parametersV = _mm256_load_ps(&parameters[i * 8]);
    __m256 cachedWeightsV = _mm256_load_ps(&criticCachedConsensusWeights[i * 8]);
    _mm256_store_ps(&criticCachedConsensusWeights[i * 8], _mm256_add_ps(cachedWeightsV, parametersV));
  }
  numAvailableConsensusData++;
}

void AC::consensusFinalStep() {
  __m256 oneOverNV = _mm256_set1_ps(1 / (float)(numAvailableConsensusData + 1));
  for (int i = 0; i < alignedCriticWeightsSize / 8; i++) {
    __m256 weightV = _mm256_load_ps(&criticWeights[i * 8]);
    __m256 cachedWeightsV = _mm256_load_ps(&criticCachedConsensusWeights[i * 8]);
    _mm256_store_ps(&criticWeights[i * 8], _mm256_mul_ps(_mm256_add_ps(cachedWeightsV, weightV), oneOverNV));
  }
  memset(criticCachedConsensusWeights, 0.0f, alignedCriticWeightsSize * sizeof(float));
  numAvailableConsensusData = 0;
}
