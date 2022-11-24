#ifndef DLB_CONTEXT_H
#define DLB_CONTEXT_H

class Driver;

class Server;

// UPDATE: Moved here, both drivers and servers need this!
// This object cannot be shared between multiple threads. Methods are not thread
// safe.x
class Context {
  friend Driver;
  friend Server;

public:
  Context(size_t max_on_fly_msgs) : maxOnFlyMsgs(max_on_fly_msgs) {
    reqBufs = new (std::align_val_t(64)) erpc::MsgBuffer[maxOnFlyMsgs];
    respBufs = new (std::align_val_t(64)) erpc::MsgBuffer[maxOnFlyMsgs];
    availableSlots = new (std::align_val_t(64)) size_t[maxOnFlyMsgs];
    for (int i = 0; i < max_on_fly_msgs; i++) {
      availableSlots[i] = i;
    }
    head = 0;
    tail = maxOnFlyMsgs - 1;
    numAvailableSlots = maxOnFlyMsgs;
  }

  size_t availableSlotSize() const { return numAvailableSlots; }

  bool slotIsAvailable() const { return (numAvailableSlots > 0); }

  // get only should be called if slotIsAvailable () == true!
  size_t get() {
    assert(numAvailableSlots > 0);
    numAvailableSlots--;
    auto index = availableSlots[head];
    head = (head + 1) % maxOnFlyMsgs;
    return index;
  }

  void getList(size_t *indices, size_t number) {
    assert(number <= numAvailableSlots);
    for (size_t i = 0; i < number; i++) {
      indices[i] = availableSlots[head];
      head = (head + 1) % maxOnFlyMsgs;
    }
    numAvailableSlots -= number;
  }

  void putBack(size_t index) {
    assert(index < maxOnFlyMsgs);
    numAvailableSlots++;
    tail = (tail + 1) % maxOnFlyMsgs;
    availableSlots[tail] = index;
  }

  erpc::MsgBuffer *reqBufs;
  erpc::MsgBuffer *respBufs;
  size_t *availableSlots;
  size_t head, tail, numAvailableSlots;
  const size_t maxOnFlyMsgs;
};

#endif // DLB_CONTEXT_H
