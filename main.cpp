#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <functional>
#include <utility>

// TODO: this needs to be a valid memory address
void* MONITORED_MEMORY = nullptr;
size_t TARGET_DATA_SIZE = sizeof(unsigned int) * 8;
std::chrono::high_resolution_clock::time_point PROGRAM_START = std::chrono::high_resolution_clock::now();

struct event {
    std::string id;
    double start;
    double end;
    double duration;
};

event record_event(std::string id, const std::function<void()>& evt) {
    std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> start = tp - PROGRAM_START;
    evt();
    tp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> end = tp - PROGRAM_START;
    double duration = end.count() - start.count();
    return event{ id, start.count(), end.count(), duration };
}

void maccess(void* p)
{
    asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

void flush(void* p) {
    asm volatile ( "clflush (%0)" :: "r"( p ) );
}

void victim() {
    unsigned int target_data = 0xFFFF;
    size_t m = 100;
    size_t n = TARGET_DATA_SIZE;

    for(size_t i = 0; i < n; i++) {
        record_event("victim access" + std::to_string(i), [target_data, i, m]() {
            unsigned int mask = 0x01 << i;
            if((target_data & mask) == mask) {
                for(size_t j = 0; j < m; j++) {
                    maccess(MONITORED_MEMORY);
                }
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void attack() {
    for(size_t i = 0; i < TARGET_DATA_SIZE; i++) {
        record_event("flush", []() {
            flush(MONITORED_MEMORY);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        record_event("reload", []() {
            maccess(MONITORED_MEMORY);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    std::thread victim_thread(victim);
    std::thread attack_thread(attack);

    victim_thread.join();
    attack_thread.join();

    return 0;
}