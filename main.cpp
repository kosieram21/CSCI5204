#include <thread>
#include <iostream>
#include <utility>
#include <cstring>
#include <sched.h>

#include "flushrld.h"
#include "events.h"

unsigned int MONITORED_MEMORY = 0;

//unsigned int TARGET_DATA = 0xF0F0F0F0;
unsigned int TARGET_DATA = 0xF0A4FB21;
size_t TARGET_DATA_SIZE = sizeof(TARGET_DATA) * 8;
unsigned int DELAY_BETWEEN_VICTIM_ACCESS = 500000;

unsigned int DELAY_AFTER_FLUSH = 500000;
size_t RELOAD_THRESHOLD = 310;

int VICTIM_CPU_AFFINITY = 1;
int ATTACK_CPU_AFFINITY = 7;
int THREAD_PRIORITY = 99;

event_queue _event_queue;

void set_affinity(int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);
}

void set_priority(int priority) {
  sched_param sp;
  memset(&sp, 0, sizeof(sp));
  sp.sched_priority = priority;
  sched_setscheduler(0, SCHED_BATCH, &sp);
}

void victim() {
  set_affinity(VICTIM_CPU_AFFINITY);
  set_priority(THREAD_PRIORITY);
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    wait(DELAY_BETWEEN_VICTIM_ACCESS);
    unsigned int mask = 0x01 << i;
    if((TARGET_DATA & mask) == mask)
      _event_queue.record_victim_access(&MONITORED_MEMORY);
  }
}

void attack() {
  set_affinity(ATTACK_CPU_AFFINITY);
  set_priority(THREAD_PRIORITY);
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    _event_queue.record_flush(&MONITORED_MEMORY);
    wait(DELAY_AFTER_FLUSH);
    _event_queue.record_reload(&MONITORED_MEMORY);
  }
}

void initialize_cache() {
  size_t n = 100000;
  for(size_t i = 0; i < n; i++) {
    flush(&MONITORED_MEMORY);
    reload(&MONITORED_MEMORY);
  }
}

int main() {
  initialize_cache();
  
  std::thread attack_thread(attack);
  std::thread victim_thread(victim);
  
  attack_thread.join();
  victim_thread.join();
  
  while(!_event_queue.is_empty()) {
    event e = _event_queue.next_event();
    std::cout << e.id << "[" << e.start << "," << e.end << "]" << e.cycles << std::endl;
  }

  unsigned int result = 0;
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    unsigned int mask = 0x01 << i;
    size_t cycles = _event_queue.next_reload_cycles();
    if(cycles < RELOAD_THRESHOLD)
      result = result | mask;
  }

  unsigned int correct = 0;
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    unsigned int mask = 0x01 << i;
    unsigned int masked = result & mask;
    unsigned int expected = TARGET_DATA & mask;

    if(masked == expected)
      correct++;
  }

  double error_rate = (double)correct / (double)TARGET_DATA_SIZE;
  std::cout << error_rate * 100 << "%" << std::endl;
  
  return 0;
}
