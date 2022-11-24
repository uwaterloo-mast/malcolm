#pragma once

#include <memory>
#include <random>
#include <stdexcept>
#include <utility>

namespace stats {

enum DistType { DETMULTIMODAL, HYPEREXPONENTIAL, LOGNORMAL };

// Characteristics of a exp-det multidistribution.
// For example, for a HyperExponential with 90% exp(50us) and 10% exp(10us),
// StatsCharList = {{90, 50000}, {10, 10000}}. The time is assumed to be in ns.
using StatsCharsList = std::vector<std::pair<double, double>>;

// Base class used to define multiple distributions.
// For determinsim of simulation, it is important that
// psedue-random generators do not have random seeds.
class Distribution {
public:
  virtual std::pair<uint8_t, uint64_t> GetNext() = 0;
  virtual double Mean() = 0;
  std::pair<uint8_t, uint64_t> operator()() { return GetNext(); }

protected:
  std::default_random_engine generator;
};

class Exponential : public Distribution {
public:
  Exponential(double lambda) : dist(lambda) {}
  std::pair<uint8_t, uint64_t> GetNext() { return {0, dist(generator)}; }

  double Mean() { return 1.0 / dist.lambda(); }

private:
  std::exponential_distribution<> dist;
};

class LogNormal : public Distribution {
public:
  LogNormal(double m, double s) : dist(m, s) {}
  std::pair<uint8_t, uint64_t> GetNext() { return {0, dist(generator)}; }

  double Mean() {
    return exp(dist.param().m() + dist.param().s() * dist.param().s() / 2);
  }

private:
  std::lognormal_distribution<> dist;
};

class DetMultiModal : public Distribution {
public:
  DetMultiModal(const StatsCharsList &distribution) {
    std::vector<double> probs;
    for (auto &pair : distribution) {
      dists.emplace_back(pair.second);
      probs.emplace_back(pair.first);
    }
    probabilites_dist_ =
        new std::discrete_distribution<int>(probs.begin(), probs.end());
  }
  std::pair<uint8_t, uint64_t> GetNext() {
    auto i = probabilites_dist_->operator()(generator);
    return {i, dists[i]};
  }

  double Mean() {
    double mean = 0;
    auto probs = probabilites_dist_->probabilities();
    for (int i = 0; i < probs.size(); ++i) {
      mean += probs[i] * dists[i];
    }
    return mean;
  }

private:
  std::vector<uint64_t> dists;
  std::discrete_distribution<int> *probabilites_dist_;
};

class HyperExponential : public Distribution {
public:
  HyperExponential(const StatsCharsList &distribution) {
    std::vector<double> probs;
    for (auto &pair : distribution) {
      dists.emplace_back(1.0 / pair.second);
      probs.emplace_back(pair.first);
    }
    probabilites_dist_ =
        new std::discrete_distribution<int>(probs.begin(), probs.end());
  }
  std::pair<uint8_t, uint64_t> GetNext() {
    auto i = probabilites_dist_->operator()(generator);
    return {i, dists[i](generator)};
  }

  double Mean() {
    double mean = 0;
    auto probs = probabilites_dist_->probabilities();
    for (int i = 0; i < probs.size(); ++i) {
      mean += probs[i] * (1.0 / dists[i].lambda());
    }
    return mean;
  }

private:
  std::vector<std::exponential_distribution<>> dists;
  std::discrete_distribution<int> *probabilites_dist_;
};
} // namespace stats

// Factory function that builds appropriate distribution given the distribution
// name and characteristics.
std::unique_ptr<stats::Distribution>
StatsFactory(stats::DistType type, const stats::StatsCharsList &exec_dist);
