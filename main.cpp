#include <thread>
#include <iostream>

#include "events.h"

unsigned int DELAY_AFTER_FLUSH = 200;
unsigned int DELAY_AFTER_RELOAD = 50;
size_t RELOAD_THRESHOLD = 700;

unsigned int MONITORED_MEMORY = 0;
unsigned int TARGET_DATA = 0xFFFFFFFF;
size_t TARGET_DATA_SIZE = sizeof(unsigned int) * 8;

event_queue g_event_queue;
unsigned int g_result = 0;

void victim() {
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsigned int mask = 0x01 << i;
    if((TARGET_DATA & mask) == mask)
      g_event_queue.record_victim_access(&MONITORED_MEMORY);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }
}

void attack() {
  for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
    g_event_queue.record_flush(&MONITORED_MEMORY);
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_AFTER_FLUSH));

    event e = g_event_queue.record_reload(&MONITORED_MEMORY);
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_AFTER_RELOAD));

    if(e.rdtsc < RELOAD_THRESHOLD) {
      unsigned int mask = 0x01 << i;
      g_result = g_result | mask;
    }
  }
}

int main() {
  std::thread victim_thread(victim);
  std::thread attack_thread(attack);

  victim_thread.join();
  attack_thread.join();

  while(!g_event_queue.is_empty()) {
    event evt = g_event_queue.next_event();
    //std::cout << evt.id << "[" << evt.start << "," << evt.end << "]" << evt.rdtsc << std::endl;
    std::cout << evt.id << " " << evt.rdtsc << std::endl;
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
