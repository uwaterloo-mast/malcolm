#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <functional>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "dists.h"

enum SchedulingPolicy { PS, FCFS };

// Base class for defining power-of-d-choices simulator as well as mutiple cFCFS
// and other types of simulators.
// All times are assumed to be in nanoseconds. Moreover, the interarrival time
// is assumed to have epxonential distribution.
class Simulator {
public:
  // Simulation stats corresponding to mean and tail latency values.
  using SimStats = std::tuple<double, double, double>;

  Simulator(SchedulingPolicy intra_server_sched_policy,
            stats::DistType departure_rv_type,
            const stats::StatsCharsList &dep_rv_characteristics, double load,
            int preemption_interval, int context_switching_overhead,
            int num_workers, int num_queues)
      : intra_server_sched_policy_(intra_server_sched_policy),
        preemption_interval_(preemption_interval),
        ctx_switching_overhead_(context_switching_overhead),
        departure_dist_(
            StatsFactory(departure_rv_type, dep_rv_characteristics)) {
    const double arrival_rate = (load * num_workers) / departure_dist_->Mean();
    arrival_dist_ = std::make_unique<stats::Exponential>(arrival_rate);
    task_queues_.resize(num_queues);
  }

  // Run simulation while processing num_events events. Returns mean and tail
  // latency informations. The event driven simulator process events at the head
  // of the priority queue, sorted by time, and creates new events accordingly.
  SimStats Run(int num_events = 2e7);

protected:
  enum EventType {
    ARRIVAL,
    DEPARTURE,
    CONTEXT_SWITCH_START,
    CONTEXT_SWITCH_FINISH
  };

  class Task {
  public:
    Task(uint64_t gen_time, uint64_t rem, int type)
        : arrival_time(gen_time), remaining_execution_time(rem), type(type) {}
    Task() {}
    uint64_t arrival_time;
    uint64_t remaining_execution_time;
    int type;
    // optional, for load balancing simulations
    int server_id;
  };

  class Event {
  public:
    Event(uint64_t t, EventType type) : time(t), event_type(type) {}
    Event(uint64_t t, EventType type, int server_id)
        : time(t), event_type(type), server_id(server_id) {}
    Event(uint64_t t, EventType type, Task task)
        : time(t), task(task), event_type(type) {
      server_id = task.server_id;
    }

    void SetServerId(const int id) {
      server_id = id;
      task.server_id = id;
    }
    struct CompareTimes {
      bool operator()(Event const &left, Event const &right) {
        return left.time > right.time;
      }
    };

    uint64_t time;
    EventType event_type;
    // optional, an event may involve a task or not!
    Task task;
    int server_id;
  };

  // Returns true if all task queues are empty.
  bool all_queues_empty() const;

  void DeqTaskAndCreateEvent(const uint64_t current_time, const int server_id = -1);

  void CreateNextDepOrCTXSwitchEvent(const uint64_t current_time, Task task);

  void CreateNewArrivalEvent(const uint64_t current_time);

  static double Percentile(std::vector<double> &vectorIn, double percent);

  virtual Task DequeueTask(const uint64_t time, const int server_id) = 0;
  virtual void DecrementAvailableWorkers(const int server_id) = 0;
  virtual void OnArrival(Event &event) = 0;
  virtual void OnDeparture(Event &event) = 0;
  virtual void OnCTXSwitchStart(Event &event) = 0;
  virtual void OnCTXSwitchFinish(Event &event) = 0;

  // Simulator configurations
  const SchedulingPolicy intra_server_sched_policy_;
  const int preemption_interval_;
  const int ctx_switching_overhead_;

  std::unique_ptr<stats::Distribution> arrival_dist_;
  std::unique_ptr<stats::Distribution> departure_dist_;

  std::priority_queue<Event, std::vector<Event>, Event::CompareTimes>
      event_queue_;
  std::vector<std::queue<Task>> task_queues_;
};

// Inter server power-of-d scheduler simulator.
class InterServerPDSim : public Simulator {
public:
  InterServerPDSim(const std::vector<int> &worker_chars,
                   SchedulingPolicy intra_server_sched_policy,
                   stats::DistType departure_rv_type,
                   const stats::StatsCharsList &dep_rv_characteristics,
                   double load, int preemption_interval = 500000,
                   int context_switching_overhead = 0, int d = 2,
                   bool piggy_back = false)
      : Simulator(intra_server_sched_policy, departure_rv_type,
                  dep_rv_characteristics, load, preemption_interval,
                  context_switching_overhead,
                  /*num_workers=*/
                  std::accumulate(worker_chars.begin(), worker_chars.end(), 0),
                  /*num_queues*/ worker_chars.size()),
        worker_chars_(worker_chars), piggy_back_(piggy_back), d_(d),
        available_worker_per_server_(worker_chars),
        piggy_backed_queue_sizes_(worker_chars.size(), 0) {}

private:
  // Implement power-of-d choices.
  int ChooseDestination() const;

  Task DequeueTask(const uint64_t time, const int server_id) override;
  void DecrementAvailableWorkers(const int server_id) override;
  void OnArrival(Event &event) override;
  void OnDeparture(Event &event) override;
  void OnCTXSwitchStart(Event &event) override;
  void OnCTXSwitchFinish(Event &event) override;

  // Holds number of workers for each server.
  const std::vector<int> worker_chars_;
  // Keeps track of unbusy workers in each server.
  std::vector<int> available_worker_per_server_;
  const int d_;
  // If true, load information are only updated on piggy backs.
  const bool piggy_back_;

  std::vector<int> piggy_backed_queue_sizes_;
};

// Intra-server centerliazed FCFC and PS scheduler simulator.
class IntraServerSchedulerSim : public Simulator {
public:
  IntraServerSchedulerSim(const int num_workers, SchedulingPolicy sch_policy,
                          stats::DistType departure_rv_type,
                          const stats::StatsCharsList &dep_rv_characteristics,
                          double load, int network_delay = 0,
                          std::vector<int> slos = {},
                          bool queue_per_type = false,
                          int preemption_interval = 5000,
                          int context_switching_overhead = 200)
      : Simulator(sch_policy, departure_rv_type, dep_rv_characteristics, load,
                  preemption_interval, context_switching_overhead, num_workers,
                  /*num_queues*/ queue_per_type ? slos.size() : 1),
        network_delay_(network_delay), slos_(slos), num_workers_(num_workers),
        available_workers_(num_workers), multi_queue_(queue_per_type) {
    if (queue_per_type)
      assert(slos_.size() == dep_rv_characteristics.size());
  }

private:
  // server_id is ignored in IntraServer simulator.
  Task DequeueTask(const uint64_t time, const int server_id) override;
  void DecrementAvailableWorkers(const int server_id=0) override;
  void OnArrival(Event &event) override;
  void OnDeparture(Event &event) override;
  void OnCTXSwitchStart(Event &event) override;
  void OnCTXSwitchFinish(Event &event) override;

  const int num_workers_;
  int available_workers_;
  const int network_delay_;
  const std::vector<int> slos_;
  const bool multi_queue_;
};