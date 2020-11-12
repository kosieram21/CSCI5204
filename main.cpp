#include <iostream>

#include "events.h"

unsigned int MONITORED_MEMORY = 0;

inline void reload(void* p) {
    asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

inline void flush(void* p) {
    asm volatile ( "clflush (%0)" :: "r"( p ) );
}

int main() {
    event_queue g_event_queue;

    g_event_queue.record_event("flush", []() { flush(&MONITORED_MEMORY); });
    event e = g_event_queue.next_event();
    std::cout << "flush: " << e.duration << "ms" << std::endl;

    g_event_queue.record_event("reload", []() { reload(&MONITORED_MEMORY); });
    e = g_event_queue.next_event();
    std::cout << "reload: " << e.duration << "ms" << std::endl;

    g_event_queue.record_event("reload", []() { reload(&MONITORED_MEMORY); });
    e = g_event_queue.next_event();
    std::cout << "reload: " << e.duration << "ms" << std::endl;
}