#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>
#include <chrono>
#include <functional>
#include <queue>

struct event {
    std::string id;
    double start;
    double end;
    double duration;
};

class event_queue {
private:
    std::queue<event> _queue;

public:
    void record_event(std::string id, const std::function<void()>& evt) {
        std::chrono::duration<double, std::milli> start = timestamp();
        evt();
        std::chrono::duration<double, std::milli> end = timestamp();
        double duration = end.count() - start.count();
        _queue.push(event{ std::move(id), start.count(), end.count(), duration });
    }

    event next_event() {
        event evt = _queue.front();
        _queue.pop();
        return evt;
    }

    bool is_empty() {
        return _queue.empty();
    }

private:
    static std::chrono::duration<double, std::milli> timestamp() {
        static std::chrono::high_resolution_clock::time_point pstp = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ts = tp - pstp;
        return ts;
    }
};

#endif /* _EVENTS_H_ */
