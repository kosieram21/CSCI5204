#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>
#include <chrono>
#include <functional>
#include <queue>

#include "flushrld.h"

struct event {
  std::string id;
  double start;
  double end;
  double duration;
  size_t rdtsc;
};

class event_queue {
private:
  std::queue<event> _queue;

public:
  event record_flush(void* p) {
    return record_event("flush", [&]() { return flush(p); });
  }

  event record_reload(void* p) {
    return record_event("reload", [&]() { return reload(p); });
  }

  event record_victim_access(void* p) {
    return record_event("victim access", [&]() { return reload(p); });
  }
  
  event next_event() {
    event evt = _queue.front();
    _queue.pop();
    return evt;
  }

  bool is_empty() {
    return _queue.empty();
  }

private:
  event record_event(std::string id, const std::function<size_t()>& evt) {
    std::chrono::duration<double, std::nano> start = timestamp();
    size_t rdtsc = evt();
    std::chrono::duration<double, std::nano> end = timestamp();
    double duration = end.count() - start.count();
    _queue.push(event{ std::move(id), start.count(), end.count(), duration, rdtsc });
    return _queue.back();
  }
  
  static std::chrono::duration<double, std::nano> timestamp() {
    static std::chrono::high_resolution_clock::time_point pstp = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> ts = tp - pstp;
    return ts;
  }
};

#endif /* _EVENTS_H_ */
