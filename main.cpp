#include <thread>
#include <chrono>
#include <iostream>
#include <utility>
#include <cstring>
#include <sched.h>

#include "flushrld.h"
#include "events.h"

unsigned int MONITORED_MEMORY = 0;

unsigned int TARGET_DATA = 0xF0A4FB21;
size_t TARGET_DATA_SIZE = sizeof(TARGET_DATA) * 8;
unsigned int DELAY_BETWEEN_VICTIM_ACCESS = 50;

unsigned int DELAY_AFTER_FLUSH = 185000;
int ATTACK_CPU_AFFINITY = 7;
int ATTACK_THREAD_PRIORITY = 99;

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
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(DELAY_BETWEEN_VICTIM_ACCESS));
    unsigned int mask = 0x01 << i;
    if((TARGET_DATA & mask) == mask)
      _event_queue.record_victim_access(&MONITORED_MEMORY);
  }
}

void attack() {
  set_affinity(ATTACK_CPU_AFFINITY);
  set_priority(ATTACK_THREAD_PRIORITY);
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    _event_queue.record_flush(&MONITORED_MEMORY);
    wait(DELAY_AFTER_FLUSH);
    _event_queue.record_reload(&MONITORED_MEMORY);
  }
}

double calibrate() {
  size_t n = 100000;
  double s1, s2;
  
  for(size_t i = 0; i < n; i++) {
    flush(&MONITORED_MEMORY);
    size_t delta = reload(&MONITORED_MEMORY);
    s1 = s1 + (delta / (double)n);
  }

  for(size_t i = 0; i < n; i++) {
    size_t delta = reload(&MONITORED_MEMORY);
    s2 = s2 + (delta / (double)n);
  }

  double delta = s1 - s2;
  double reload_threshold = s1 - delta / 4;
  return reload_threshold;
}

double compute_error(double reload_threshold) {
  unsigned int correct = 0;
  
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    unsigned int mask = 0x01 << i;
    size_t cycles = _event_queue.next_reload_cycles();

    unsigned int bit1 = cycles < reload_threshold ? 0x01 : 0x00;
    unsigned int bit2 = (TARGET_DATA & mask) >> i;
    std::cout << bit1 << "," << bit2 << std::endl;
    if(bit1 == bit2)
      correct++;
  }
  
  double error = (double)correct / (double)TARGET_DATA_SIZE;
  return error;
}

int main() {
  double reload_threshold = calibrate();
  
  std::thread attack_thread(attack);
  std::thread victim_thread(victim);
  
  attack_thread.join();
  victim_thread.join();
  
  while(!_event_queue.is_empty()) {
    event e = _event_queue.next_event();
    std::cout << e.id << "[" << e.start << "," << e.end << "]" << e.cycles << std::endl;
  }

  std::cout << reload_threshold << std::endl;
  double error = compute_error(reload_threshold);
  std::cout << error * 100 << "%" << std::endl;
  
  return 0;
}
