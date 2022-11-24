#ifndef DLB_DISTRIBUTIONS_H
#define DLB_DISTRIBUTIONS_H

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <cmath>

class Distribution {
public:
  Distribution(uint64_t seed) {
    gsl_rng_env_setup();
    T = gsl_rng_taus2;
    generator = gsl_rng_alloc(T);
    gsl_rng_set(generator, seed);
  }

  virtual std::pair<uint8_t, double> getNext() = 0;
  virtual double getMean() = 0;
  std::pair<uint8_t, double> operator()() { return getNext(); }

protected:
  const gsl_rng_type *T;
  gsl_rng *generator;
};

class Uniform : public Distribution {
public:
  explicit Uniform(uint64_t seed) : Distribution(seed) {}

  std::pair<uint8_t, double> getNext() {
    return {0, gsl_rng_uniform(generator)};
  }

  double getMean() { return 0.5; }

};

class UniformInt : public Distribution {
public:
  UniformInt(uint64_t seed, uint64_t n) : Distribution(seed), n(n) {}

  std::pair<uint8_t, double> getNext() {
    return {0, (double)gsl_rng_uniform_int(generator, n)};
  }

  double getMean() { return n / 2.0; }

private:
  uint64_t n;
};

class Exponential : public Distribution {
public:
  Exponential(uint64_t seed, double mean)
      : Distribution(seed), mean(mean) {}

  std::pair<uint8_t, double> getNext() {
    return {0, gsl_ran_exponential(generator, mean)};
  }

  double getMean() { return mean; }

private:
  double mean;
};

class LogNormal : public Distribution {
public:
  LogNormal(uint64_t seed, double m, double s)
      : Distribution(seed), zeta(m), sigma(s) {}

  std::pair<uint8_t, double> getNext() {
    return {0, gsl_ran_lognormal(generator, zeta, sigma)};
  }

  double getMean() { return exp(zeta + sigma * sigma / 2); }

private:
  double zeta;
  double sigma;
};

class DetMultiModal : public Distribution {
public:
  DetMultiModal(uint64_t seed,
                const std::vector<std::pair<double, double>>& distribution)
      : Distribution(seed) {
    int size = distribution.size();
    weights = new (std::align_val_t(64)) double[size];
    int index = 0;
    totalMean = 0;
    double totalWeight = 0;
    for (auto &pair : distribution) {
      means.emplace_back(pair.second);
      weights[index++] = pair.first;
      totalWeight += pair.first;
      totalMean += (pair.second * pair.first);
    }
    totalMean /= totalWeight;
    g = gsl_ran_discrete_preproc(size, weights);
  }

  std::pair<uint8_t, double> getNext() {
    auto i = gsl_ran_discrete(generator, g);
    return {i, means[i]};
  }

  double getMean() { return totalMean; }

private:
  std::vector<double> means;
  double totalMean;
  double *weights;
  gsl_ran_discrete_t *g;
};

class ExpMultiModal : public Distribution {
public:
  ExpMultiModal(uint64_t seed,
                const std::vector<std::pair<double, double>>& distribution)
      : Distribution(seed) {
    int size = distribution.size();
    weights = new (std::align_val_t(64)) double[size];
    int index = 0;
    totalMean = 0;
    double totalWeight = 0;
    for (auto &pair: distribution) {
      means.emplace_back(pair.second);
      weights[index++] = pair.first;
      totalWeight += pair.first;
      totalMean += (pair.first * pair.second);
    }
    totalMean /= totalWeight;
    g = gsl_ran_discrete_preproc(size, weights);
  }

  std::pair<uint8_t, double> getNext() {
    auto i = gsl_ran_discrete(generator, g);
    return {i, gsl_ran_exponential(generator, means[i])};
  }

  double getMean() { return totalMean; }

private:
  std::vector<double> means;
  double totalMean;
  double *weights;
  gsl_ran_discrete_t *g;
};

#endif // DLB_DISTRIBUTIONS_H
