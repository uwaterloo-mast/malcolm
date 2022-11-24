#ifndef ACRLTEST_LRSCHEDULER_H
#define ACRLTEST_LRSCHEDULER_H

#include <math.h>
#include <cassert>

class LRScheduler {
public:
    LRScheduler() {}
    virtual float getLearningRateMultiplier() = 0;
};

class CosineAnnealingLR : public LRScheduler {
public:
    CosineAnnealingLR(float etaMin, float etaMax, int T0, int TMult): etaMin(etaMin), etaMax(etaMax), Ti(T0), Tmult(TMult) {
        assert(etaMin < etaMax);
        assert(Ti >0);
    }
    float getLearningRateMultiplier() override {
        float eta = etaMin + 0.5f * (etaMax - etaMin) * (1.0f + cosf(M_PI * epoch / Ti));
        // Check for warm restart!
        if (++epoch % Ti == 0){
            Ti *= Tmult;
            epoch = 0;
        }
        return eta;
    };

private:
    const float etaMin;
    const float etaMax;
    const int Tmult;
    int Ti;
    int epoch = 0;
};

class ConstantLR : public LRScheduler {
public:
    ConstantLR() {}

    float getLearningRateMultiplier () {
        return 1.0f;
    }
};

#endif //ACRLTEST_LRSCHEDULER_H
