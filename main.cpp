#include <thread>
#include <iostream>
#include <sched.h>

#include "events.h"

unsigned int MONITORED_MEMORY = 0;

unsigned int DELAY_AFTER_FLUSH = 50;
size_t RELOAD_THRESHOLD = 600;

unsigned int TARGET_DATA = 0xF0F0F0F0;
size_t TARGET_DATA_SIZE = sizeof(TARGET_DATA) * 8;

unsigned int DELAY_BETWEEN_VICTIM_ACCESS = 50;

event_queue g_victim_event_queue;
event_queue g_attack_event_queue;

void set_affinity(int cpu) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);
}

void victim() {
  set_affinity(1);
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(DELAY_BETWEEN_VICTIM_ACCESS));
    unsigned int mask = 0x01 << i;
    if((TARGET_DATA & mask) == mask)
      g_victim_event_queue.record_victim_access(&MONITORED_MEMORY);
  }
}

void attack() {
  set_affinity(2);
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    g_attack_event_queue.record_flush(&MONITORED_MEMORY);
    std::this_thread::sleep_for(std::chrono::nanoseconds(DELAY_AFTER_FLUSH));
    g_attack_event_queue.record_reload(&MONITORED_MEMORY);
  }
}

int main() {
  std::thread attack_thread(attack);
  std::thread victim_thread(victim);
  
  attack_thread.join();
  victim_thread.join();
  
  event_queue g_event_queue = event_queue::merge(g_victim_event_queue, g_attack_event_queue);
  unsigned int g_result = 0;
  size_t i = 0;
  while(!g_event_queue.is_empty()) {
    event evt = g_event_queue.next_event();
    if(evt.id == "reload") {
      if(evt.rdtsc < RELOAD_THRESHOLD) {
        unsigned int mask = 0x01 << i;
        g_result = g_result | mask;
      }
      i++;
    }
    std::cout << evt.id << "[" << evt.start << "," << evt.end << "]" << evt.rdtsc << std::endl;
  }

  unsigned int correct = 0;
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    unsigned int mask = 0x01 << i;
    unsigned int result = g_result & mask;
    unsigned int expected = TARGET_DATA & mask;

    if(result == expected)
      correct++;
  }

  double error_rate = (double)correct / (double)TARGET_DATA_SIZE;
  std::cout << error_rate * 100 << "%" << std::endl;
  
  return 0;
}
