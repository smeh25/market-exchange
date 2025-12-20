#pragma once
#include <cstdint>
#include <atomic>

class IdGenerator {
public:
    // Implementation inside the class is implicitly 'inline'
    explicit IdGenerator(uint64_t start_id = 0) : current_id(start_id) {}

    uint64_t next() {
        return ++current_id;
    }

private:
    // Atomic prevent two threads from getting the same ID.
    std::atomic<uint64_t> current_id;
};