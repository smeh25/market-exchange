#pragma once
#include <cstdint>

class IdGenerator {
private:
    uint64_t current_id;

public:
    // 'explicit' prevents accidental conversions from int to IdGenerator.
    explicit IdGenerator(uint64_t start_id = 0);

    // Returns the next sequential ID
    uint64_t next();
};