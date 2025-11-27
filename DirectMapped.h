#include <iostream>
#include <cstdint>

struct cacheLine {
    bool valid =  false;
    uint64_t tag = 0;
};


