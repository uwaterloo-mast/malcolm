#include <iostream>
#include <string>

#include "simulator.h"

int main() {
  using namespace std;
  // InterServer scheduler
  cout << string(50, '=') << endl;
  {
    cout << "power-of-2-choices with heterogenous configuration" << endl;
    // Departure RV is a exponential with mean equal to 50 usec.
    InterServerPDSim simulator(
        /*worker_chars=*/{12, 2, 2, 2, 2},
        /*intra_server_sched_policy=*/SchedulingPolicy::FCFS,
        /*departure_rv_type=*/stats::DistType::HYPEREXPONENTIAL,
        /*dep_rv_characteristics=*/{{1, 50000}},
        /*load=*/0.85);

    auto [mean, p99, p99_99] = simulator.Run();
    cout << "Mean    " << mean << endl;
    cout << "P99     " << p99 << endl;
    cout << "P99.99  " << p99_99 << endl;
  }
  cout << string(50, '-') << endl;
  {
    cout << "power-of-2-choices with homogenous configuration" << endl;
    InterServerPDSim simulator(
        /*worker_chars=*/{2, 2, 2, 2, 2},
        /*intra_server_sched_policy=*/SchedulingPolicy::FCFS,
        /*departure_rv_type=*/stats::DistType::HYPEREXPONENTIAL,
        /*dep_rv_characteristics=*/{{1, 50000}},
        /*load=*/0.85);

    auto [mean, p99, p99_99] = simulator.Run();
    cout << "Mean    " << mean << endl;
    cout << "P99     " << p99 << endl;
    cout << "P99.99  " << p99_99 << endl;
  }

  // IntraServer scheduler
  cout << string(50, '=') << endl;
  {
    cout << "cFCFS ..." << endl;
    IntraServerSchedulerSim simulator(
        /*num_workers=*/6,
        /*intra_server_sched_policy=*/SchedulingPolicy::FCFS,
        /*departure_rv_type=*/stats::DistType::HYPEREXPONENTIAL,
        /*dep_rv_characteristics=*/{{1, 50000}},
        /*load=*/0.95);
    auto [mean, p99, p99_99] = simulator.Run();
    cout << "Mean    " << mean << endl;
    cout << "P99     " << p99 << endl;
    cout << "P99.99  " << p99_99 << endl;
  }

  cout << string(50, '-') << endl;
  {
    cout << "PS ..." << endl;
    IntraServerSchedulerSim simulator(
        /*num_workers=*/6,
        /*intra_server_sched_policy=*/SchedulingPolicy::PS,
        /*departure_rv_type=*/stats::DistType::HYPEREXPONENTIAL,
        /*dep_rv_characteristics=*/{{1, 50000}},
        /*load=*/0.95);
    auto [mean, p99, p99_99] = simulator.Run();
    cout << "Mean    " << mean << endl;
    cout << "P99     " << p99 << endl;
    cout << "P99.99  " << p99_99 << endl;
  }
}