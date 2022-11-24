
#include <iostream>
#include <limits>

#include "simulator.h"

#define NS_TO_US 0.001f

Simulator::SimStats Simulator::Run(const int num_events) {
  // create first arrival
  CreateNewArrivalEvent(/*current_time=*/0);

  int num_popped_events = 0;
  std::vector<double> latencies;

  while (num_popped_events < num_events) {
    // Pop the first event and increment counter
    auto event = std::move(event_queue_.top());
    event_queue_.pop();
    num_popped_events++;

    if (event.event_type == ARRIVAL) {
      OnArrival(event);
      CreateNewArrivalEvent(event.task.arrival_time);
    } else if (event.event_type == DEPARTURE) {
      latencies.push_back(NS_TO_US * (event.time - event.task.arrival_time));
      OnDeparture(event);
    } else if (event.event_type == CONTEXT_SWITCH_START) {
      // decrement task remaining execution time!
      event.task.remaining_execution_time -= preemption_interval_;
      OnCTXSwitchStart(event);
    } else if (event.event_type == CONTEXT_SWITCH_FINISH) {
      OnCTXSwitchFinish(event);
    } else {
      std::runtime_error("Simulator is at undefined state!");
    }
  }

  const auto mean_latency = std::accumulate(latencies.begin(), latencies.end(),
                                      decltype(latencies)::value_type(0.0)) / latencies.size();

  return {mean_latency, Percentile(latencies, 99),
          Percentile(latencies, 99.99)};
}

bool Simulator::all_queues_empty() const {
  for (auto const &queue : task_queues_)
    if (!queue.empty())
      return false;
  return true;
}

void Simulator::CreateNewArrivalEvent(const uint64_t current_time) {
  const auto arrival_time = arrival_dist_->GetNext().second + current_time;
  const auto [type, execution_time] = departure_dist_->GetNext();
  Task task {arrival_time, execution_time, type};
  event_queue_.emplace(arrival_time, ARRIVAL, task);
}

void Simulator::DeqTaskAndCreateEvent(const uint64_t current_time, int server_id) {
  assert(!all_queues_empty());
  auto task = DequeueTask(current_time, server_id);
  CreateNextDepOrCTXSwitchEvent(current_time, task);
  DecrementAvailableWorkers(server_id);
}

void Simulator::CreateNextDepOrCTXSwitchEvent(const uint64_t current_time, Task task) {
  // if the remaining execution time is greater than preemptionInterval,
  // create a new ctx switch event else, create a departure event!
  if (intra_server_sched_policy_ == PS &&
      task.remaining_execution_time > preemption_interval_) {
    event_queue_.emplace(current_time + preemption_interval_,
                         CONTEXT_SWITCH_START, task);
  } else {
    event_queue_.emplace(current_time + task.remaining_execution_time,
                         DEPARTURE, task);
  }
}

double Simulator::Percentile(std::vector<double> &vectorIn, double percent) {
  auto nth = vectorIn.begin() + (percent * vectorIn.size()) / 100;
  std::nth_element(vectorIn.begin(), nth, vectorIn.end());
  return *nth;
}

void InterServerPDSim::OnArrival(Simulator::Event &event) {
  // If there is an available worker, generate contextSwitch/Departure
  // event! Else, enqueue task to the task queue!
  int dest = ChooseDestination();
  event.SetServerId(dest);
  if (available_worker_per_server_[dest] > 0) {
    available_worker_per_server_[dest]--;
    CreateNextDepOrCTXSwitchEvent(event.time, event.task);
  } else {
    task_queues_[dest].push(event.task);
  }
}

void InterServerPDSim::OnDeparture(Simulator::Event &event) {
  // Upon departure, a worker becomes free.
  available_worker_per_server_[event.server_id]++;
  piggy_backed_queue_sizes_[event.server_id] =
      task_queues_[event.server_id].size();
  // If there is a task to be scheduled, do so!
  if (!task_queues_[event.server_id].empty()) {
    DeqTaskAndCreateEvent(event.time, event.server_id);
  }
}

void InterServerPDSim::OnCTXSwitchStart(Simulator::Event &event) {
  if (task_queues_[event.server_id].empty()) {
    CreateNextDepOrCTXSwitchEvent(event.time, event.task);
  } else {
    // If queue was empty, reschedule current task!
    assert(event.task.remaining_execution_time > 0);
    task_queues_[event.server_id].push(event.task);
    event_queue_.emplace(event.time + ctx_switching_overhead_,
                         CONTEXT_SWITCH_FINISH, event.server_id);
  }
}

void InterServerPDSim::OnCTXSwitchFinish(Simulator::Event &event) {
  available_worker_per_server_[event.server_id]++;
  if (task_queues_[event.server_id].empty())
    DeqTaskAndCreateEvent(event.time, event.server_id);
}

int InterServerPDSim::ChooseDestination() const {
  std::set<int> d_selected_servers;
  while (d_selected_servers.size() < this->d_)
    d_selected_servers.insert(std::rand() %
                              available_worker_per_server_.size());
  unsigned min_load = std::numeric_limits<int>::max();
  int index = -1;
  for (const auto &id : d_selected_servers) {
    const auto size =
        piggy_back_ ? piggy_backed_queue_sizes_[id] : task_queues_[id].size();
    if (size < min_load) {
      min_load = task_queues_[id].size();
      index = id;
    }
  }
  assert(index != -1);
  return index;
}

Simulator::Task InterServerPDSim::DequeueTask(const uint64_t time,
                                              const int server_id) {
  auto task = std::move(task_queues_[server_id].front());
  task_queues_[server_id].pop();
  return task;
}

void InterServerPDSim::DecrementAvailableWorkers(int server_id) {
  available_worker_per_server_[server_id]--;
}

void IntraServerSchedulerSim::OnArrival(Simulator::Event &event) {
  // If there is an available worker, generate contextSwitch/Departure
  // event! Else, enqueue task to the task queue!
  if (available_workers_ > 0) {
    available_workers_--;
    CreateNextDepOrCTXSwitchEvent(event.time, event.task);
  } else {
    task_queues_[multi_queue_ ? event.task.type : 0].push(event.task);
  }
}

void IntraServerSchedulerSim::OnDeparture(Simulator::Event &event) {
  // cFCFS network delay is similar to a contextSwitchFinish Event!
  if (network_delay_ == 0) {
    available_workers_++;
    if (!all_queues_empty())
      DeqTaskAndCreateEvent(event.time);
  } else {
    event_queue_.emplace(event.time + network_delay_, CONTEXT_SWITCH_FINISH);
  }
}

void IntraServerSchedulerSim::OnCTXSwitchStart(Simulator::Event &event) {
  if (all_queues_empty()) {
    CreateNextDepOrCTXSwitchEvent(event.time, event.task);
  } else {
    assert(event.task.remaining_execution_time > 0);
    task_queues_[multi_queue_ ? event.task.type : 0].push(event.task);
    event_queue_.emplace(event.time + ctx_switching_overhead_,
                         CONTEXT_SWITCH_FINISH);
  }
}

void IntraServerSchedulerSim::OnCTXSwitchFinish(Simulator::Event &event) {
  available_workers_++;
  if (!all_queues_empty()) {
    DeqTaskAndCreateEvent(event.time);
  }
}

Simulator::Task IntraServerSchedulerSim::DequeueTask(const uint64_t time,
                                                     const int server_id) {
  int queue = 0;
  // The multi queue implementation uses alogirthm One (Queue Selection Policy)
  // in Shinjuku: Preemptive Scheduling for µsecond-scale Tail Latency here:
  // https://www.usenix.org/system/files/nsdi19-kaffes.pdf
  if (multi_queue_) {
    double max = 0;
    queue = -1;
    for (int i = 0; i < task_queues_.size(); i++) {
      if (!task_queues_[i].empty()) {
        double ratio =
            (double)(time - task_queues_[i].front().arrival_time) / slos_[i];
        if (ratio >= max) {
          max = ratio;
          queue = i;
        }
      }
    }
  }
  auto task = std::move(task_queues_[queue].front());
  task_queues_[queue].pop();
  return task;
}

void IntraServerSchedulerSim::DecrementAvailableWorkers(int server_id) {
  available_workers_--;
}