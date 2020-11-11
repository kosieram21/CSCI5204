#include <thread>
#include <iostream>

#include "flushrld.h"
#include "events.h"

unsigned int MONITORED_MEMORY = 0;
size_t TARGET_DATA_SIZE = sizeof(unsigned int) * 8;

event_queue g_event_queue;

void victim() {
    unsigned int target_data = 0xFFFF;
    size_t m = 100;
    size_t n = TARGET_DATA_SIZE;

    for(size_t i = 0; i < n; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        g_event_queue.record_event("victim access" + std::to_string(i), [target_data, i, m]() {
            unsigned int mask = 0x01 << i;
            if((target_data & mask) == mask) {
                for(size_t j = 0; j < m; j++) {
                    reload(&MONITORED_MEMORY);
                }
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

void attack() {
    for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
        g_event_queue.record_event("flush", []() { flush(&MONITORED_MEMORY); });
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        g_event_queue.record_event("reload", []() { reload(&MONITORED_MEMORY); });
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    /*g_event_queue.record_event("reload", []() { unsigned int t = MONITORED_MEMORY; });
    event e = g_event_queue.next_event();
    std::cout << e.id << ": " << e.duration << "ms" << std::endl;

    g_event_queue.record_event("flush", []() { flush(&MONITORED_MEMORY); });
    e = g_event_queue.next_event();
    std::cout << e.id << ": " << e.duration << "ms" << std::endl;

    g_event_queue.record_event("reload", []() { unsigned int t = MONITORED_MEMORY; });
    e = g_event_queue.next_event();
    std::cout << e.id << ": " << e.duration << "ms" << std::endl;

    g_event_queue.record_event("reload", []() { unsigned int t = MONITORED_MEMORY; });
    e = g_event_queue.next_event();
    std::cout << e.id << ": " << e.duration << "ms" << std::endl;*/

    std::thread victim_thread(victim);
    std::thread attack_thread(attack);

    victim_thread.join();
    attack_thread.join();

    while(!g_event_queue.is_empty()) {
        event evt = g_event_queue.next_event();
        std::cout << evt.id << "[" << evt.start << "," << evt.end << "] " << evt.duration << "ms" << std::endl;
    }

    return 0;
}