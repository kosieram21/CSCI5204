#include <iostream>
#include <thread>

#include "cacheutils.h"

// TODO: this needs to be a valid memory address
void* MONITORED_MEMORY = nullptr;

void victim() {
    unsigned int target_data = 100;
    size_t m = 100;
    size_t n = sizeof(target_data) * 8;

    for(size_t i = 0; i < n; i++) {
        unsigned int mask = 0x01 << i;
        if((target_data & mask) == mask) {
            for(size_t j = 0; j < m; j++) {
                maccess(MONITORED_MEMORY);
            }
        }
    }
}

void attack() {
    // TODO: break condition for this while loop?
    while(true) {
        flush(MONITORED_MEMORY);
        // TODO: thread sleep for some amount of time?
        maccess(MONITORED_MEMORY);
    }
}

int main() {
    std::thread victim_thread(victim);
    std::thread attack_thread(attack);

    victim_thread.join();
    attack_thread.join();

    return 0;
}