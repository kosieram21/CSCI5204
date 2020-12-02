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
    return record_event("flush", [p]() { return flush(p); });
  }

  event record_reload(void* p) {
    return record_event("reload", [p]() { return reload(p); });
  }

  event record_victim_access(void* p) {
    return record_event("victim access", [p]() { return reload(p); });
  }
  
  event next_event() {
    event evt = _queue.front();
    _queue.pop();
    return evt;
  }

  bool is_empty() {
    return _queue.empty();
  }

  static event_queue merge(event_queue q1, event_queue q2) {
    event_queue q3;
    while(!q1.is_empty() || !q2.is_empty()) {
      if(q1.is_empty()) {
	q3._queue.push(q2.next_event());
      } else if(q2.is_empty()) {
	q3._queue.push(q1.next_event());
      } else {
	event e1 = q1._queue.front();
	event e2 = q2._queue.front();
	if(e1.start < e2.start)
	  q3._queue.push(q1.next_event());
	else
	  q3._queue.push(q2.next_event());
      }
    }
    return q3;
  }

private:
  event record_event(std::string id, const std::function<size_t()>& evt) {
    std::chrono::duration<double, std::milli> start = timestamp();
    size_t rdtsc = evt();
    std::chrono::duration<double, std::milli> end = timestamp();
    double duration = end.count() - start.count();
    _queue.push(event{ std::move(id), start.count(), end.count(), duration, rdtsc });
    return _queue.back();
  }
  
  static std::chrono::duration<double, std::milli> timestamp() {
    static std::chrono::high_resolution_clock::time_point pstp = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ts = tp - pstp;
    return ts;
  }
};

#endif /* _EVENTS_H_ */
