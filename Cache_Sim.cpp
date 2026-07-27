#include <iostream>
#include <iomanip>
#include "DirectMapped.h"
#include "SetAssociative.h"
#include "FullyAssociative.h"

using namespace std;

void printStats(uint64_t hits, uint64_t misses) {
    cout << "Hits: " << hits << "  Misses: " << misses
         << "  Hit rate: " << fixed << setprecision(1)
         << (100.0 * hits / (hits + misses)) << "%\n";
}

int main() {
    size_t cacheSize = 256;
    int blockSize = 64;

    uint64_t addresses[] = {
        0x0000, 0x0040, 0x0080, 0x00C0,
        0x0000, 0x0100, 0x0000, 0x0080,
    };
    int numAddresses = sizeof(addresses) / sizeof(addresses[0]);

    // Direct Mapped
    cout << "=== Direct Mapped Cache ===\n";
    cout << "256 bytes, 64B blocks, 4 lines\n\n";
    DirectMappedCache dm(cacheSize, blockSize);
    for (int i = 0; i < numAddresses; i++) {
        bool hit = dm.access(addresses[i]);
        cout << "  0x" << hex << setw(4) << setfill('0')
             << addresses[i] << ": " << (hit ? "HIT" : "MISS") << "\n";
    }
    cout << dec;
    printStats(dm.getHits(), dm.getMisses());

    // 2-Way Set Associative
    cout << "\n=== 2-Way Set Associative Cache ===\n";
    cout << "256 bytes, 64B blocks, 2 sets x 2 ways\n\n";
    SetAssociativeCache sa(cacheSize, blockSize, 2);
    for (int i = 0; i < numAddresses; i++) {
        bool hit = sa.access(addresses[i]);
        cout << "  0x" << hex << setw(4) << setfill('0')
             << addresses[i] << ": " << (hit ? "HIT" : "MISS") << "\n";
    }
    cout << dec;
    printStats(sa.getHits(), sa.getMisses());

    // Fully Associative
    cout << "\n=== Fully Associative Cache ===\n";
    cout << "256 bytes, 64B blocks, 4 lines\n\n";
    FullyAssociativeCache fa(cacheSize, blockSize);
    for (int i = 0; i < numAddresses; i++) {
        bool hit = fa.access(addresses[i]);
        cout << "  0x" << hex << setw(4) << setfill('0')
             << addresses[i] << ": " << (hit ? "HIT" : "MISS") << "\n";
    }
    cout << dec;
    printStats(fa.getHits(), fa.getMisses());

    return 0;
}
