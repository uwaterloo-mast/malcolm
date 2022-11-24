#include "dists.h"

std::unique_ptr<stats::Distribution>
StatsFactory(stats::DistType type, const stats::StatsCharsList &exec_dist) {
  if (type == stats::DETMULTIMODAL) {
    return std::make_unique<stats::DetMultiModal>(exec_dist);
  } else if (type == stats::HYPEREXPONENTIAL) {
    return std::make_unique<stats::HyperExponential>(exec_dist);
  } else {
    throw std::runtime_error("Undefined RV!");
  }
}
