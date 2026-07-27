#include <iostream>
#include <iomanip>
#include "DirectMapped.h"

int main() {
    // 256 bytes cache, 64 bytes per block = 4 cache lines
    size_t cacheSize = 256;
    int blockSize = 64;
    DirectMappedCache cache(cacheSize, blockSize);

    // Sample addresses to test
    uint64_t addresses[] = {
        0x0000, // index 0, miss
        0x0040, // index 1, miss
        0x0080, // index 2, miss
        0x00C0, // index 3, miss
        0x0000, // index 0, hit (same as first)
        0x0100, // index 0, miss (conflicts with 0x0000)
        0x0000, // index 0, miss (evicted by 0x0100)
        0x0080, // index 2, hit (still there)
    };

    int numAddresses = sizeof(addresses) / sizeof(addresses[0]);

    std::cout << "Direct Mapped Cache: " << cacheSize << " bytes, "
              << blockSize << " bytes/block, " << cache.getNumLines() << " lines\n";
    std::cout << "Offset bits: " << cache.getOffsetBits()
              << "  Index bits: " << cache.getIndexBits() << "\n\n";

    for (int i = 0; i < numAddresses; i++) {
        bool hit = cache.access(addresses[i]);
        std::cout << "Access 0x" << std::hex << std::setw(4) << std::setfill('0')
                  << addresses[i] << ": " << (hit ? "HIT" : "MISS") << "\n";
    }

    std::cout << "\nTotal hits:   " << std::dec << cache.getHits() << "\n";
    std::cout << "Total misses: " << cache.getMisses() << "\n";
    std::cout << "Hit rate:     " << std::fixed << std::setprecision(1)
              << (100.0 * cache.getHits() / (cache.getHits() + cache.getMisses()))
              << "%\n";

    return 0;
}
