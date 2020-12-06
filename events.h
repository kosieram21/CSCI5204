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
  size_t cycles;
};

class event_queue {
private:
  std::queue<event> _attack_queue;
  std::queue<event> _victim_queue;

  std::queue<size_t> _reload_cycles_queue;

public:
  void record_flush(void* p) {
    _attack_queue.push(record_event("flush", [p]() { return flush(p); }));
  }

  void record_reload(void* p) {
    event e = record_event("reload", [p]() { return reload(p); });
    _attack_queue.push(e);
    _reload_cycles_queue.push(e.cycles);
  }

  void record_victim_access(void* p) {
    _victim_queue.push(record_event("victim access", [p]() { return reload(p); }));
  }
  
  event next_event() {
    if(_attack_queue.empty()) {
      event e = _victim_queue.front();
      _victim_queue.pop();
      return e;
    } else if(_victim_queue.empty()) {
      event e = _attack_queue.front();
      _attack_queue.pop();
      return e;
    } else {
      event e1 = _attack_queue.front();
      event e2 = _victim_queue.front();
      if(e1.start < e2.start) {
	_attack_queue.pop();
	return e1;
      } else {
	_victim_queue.pop();
	return e2;
      }
    }
  }

  size_t next_reload_cycles() {
    size_t cycles = _reload_cycles_queue.front();
    _reload_cycles_queue.pop();
    return cycles;
  }

  bool is_empty() {
    return _attack_queue.empty() && _victim_queue.empty();
  }

private:
  event record_event(std::string id, const std::function<size_t()>& evt) {
    std::chrono::duration<double, std::milli> start = timestamp();
    size_t cycles = evt();
    std::chrono::duration<double, std::milli> end = timestamp();
    double duration = end.count() - start.count();
    event e{std::move(id), start.count(), end.count(), duration, cycles};
    return e;
  }
  
  static std::chrono::duration<double, std::milli> timestamp() {
    static std::chrono::high_resolution_clock::time_point pstp = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ts = tp - pstp;
    return ts;
  }
};

#endif /* _EVENTS_H_ */
