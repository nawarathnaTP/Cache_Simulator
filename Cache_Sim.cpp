#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "DirectMapped.h"
#include "SetAssociative.h"
#include "FullyAssociative.h"

using namespace std;

void printUsage(const char* prog) {
    cerr << "Usage: " << prog
         << " --type <dm|sa|fa> --cache-size <bytes> --block-size <bytes> [--ways <n>] --trace <file>\n"
         << "  Types: dm = Direct Mapped, sa = Set Associative, fa = Fully Associative\n"
         << "  --ways is required only for sa\n";
}

vector<uint64_t> readTrace(const string& filename) {
    vector<uint64_t> addresses;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: could not open trace file: " << filename << "\n";
        return addresses;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        uint64_t addr;
        stringstream ss(line);
        ss >> hex >> addr;
        addresses.push_back(addr);
    }

    return addresses;
}

double hitRate(uint64_t hits, uint64_t misses) {
    uint64_t total = hits + misses;
    return total > 0 ? (100.0 * hits / total) : 0.0;
}

int main(int argc, char* argv[]) {
    size_t cacheSize = 0;
    int blockSize = 0;
    int ways = 0;
    string traceFile;
    string type;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--type" && i + 1 < argc) type = argv[++i];
        else if (arg == "--cache-size" && i + 1 < argc) cacheSize = stoull(argv[++i]);
        else if (arg == "--block-size" && i + 1 < argc) blockSize = stoi(argv[++i]);
        else if (arg == "--ways" && i + 1 < argc) ways = stoi(argv[++i]);
        else if (arg == "--trace" && i + 1 < argc) traceFile = argv[++i];
    }

    if (type.empty() || cacheSize == 0 || blockSize == 0 || traceFile.empty()) {
        printUsage(argv[0]);
        return 1;
    }

    if (type == "sa" && ways == 0) {
        cerr << "Error: --ways is required for set associative cache\n";
        return 1;
    }

    vector<uint64_t> addresses = readTrace(traceFile);
    if (addresses.empty()) return 1;

    cout << "Trace: " << traceFile << " (" << addresses.size() << " accesses)\n\n";

    if (type == "dm") {
        DirectMappedCache cache(cacheSize, blockSize);
        int offBits = cache.getOffsetBits();
        int idxBits = cache.getIndexBits();

        cout << "Direct Mapped Cache: " << cacheSize << " bytes, "
             << blockSize << " bytes/block, " << cache.getNumLines() << " lines\n";
        cout << "Address breakdown: [tag:" << (64 - offBits - idxBits)
             << " | index:" << idxBits << " | offset:" << offBits << "]\n\n";

        for (size_t i = 0; i < addresses.size(); i++) {
            uint64_t addr = addresses[i];
            size_t index = (addr >> offBits) & ((1 << idxBits) - 1);
            uint64_t tag = addr >> (offBits + idxBits);

            // Check state before access
            const auto& line = cache.getCache()[index];
            bool wasValid = line.valid;
            uint64_t oldTag = line.tag;

            bool hit = cache.access(addr);

            cout << "#" << dec << setfill(' ') << setw(4) << left << (i + 1) << right
                 << "  addr: 0x" << hex << setw(4) << setfill('0') << addr
                 << "  tag: 0x" << setw(4) << tag
                 << "  index: " << dec << setfill(' ') << setw(3) << index;

            if (hit) {
                cout << "  -> HIT\n";
            } else if (!wasValid) {
                cout << "  -> MISS (cold, line was empty)\n";
            } else {
                cout << "  -> MISS (conflict, evicted tag 0x"
                     << hex << setw(4) << setfill('0') << oldTag << ")\n" << dec;
            }
        }

        cout << "\n--- Summary ---\n";
        cout << "Total accesses: " << addresses.size() << "\n";
        cout << "Hits: " << cache.getHits() << "  Misses: " << cache.getMisses()
             << "  Hit Rate: " << fixed << setprecision(2)
             << hitRate(cache.getHits(), cache.getMisses()) << "%\n";

    } else if (type == "sa") {
        SetAssociativeCache cache(cacheSize, blockSize, ways);
        int offBits = cache.getOffsetBits();
        int idxBits = cache.getIndexBits();

        cout << ways << "-Way Set Associative Cache: " << cacheSize << " bytes, "
             << blockSize << " bytes/block, " << cache.getNumSets() << " sets\n";
        cout << "Address breakdown: [tag:" << (64 - offBits - idxBits)
             << " | index:" << idxBits << " | offset:" << offBits << "]\n\n";

        for (size_t i = 0; i < addresses.size(); i++) {
            uint64_t addr = addresses[i];
            size_t setIdx = (addr >> offBits) & ((1 << idxBits) - 1);
            uint64_t tag = addr >> (offBits + idxBits);

            // Snapshot set state before access
            vector<SetLine> before = cache.getCache()[setIdx];
            list<int> lruBefore = cache.getLruOrder()[setIdx];

            bool hit = cache.access(addr);

            // Find which way holds the tag now
            int usedWay = 0;
            for (int w = 0; w < ways; w++) {
                if (cache.getCache()[setIdx][w].valid && cache.getCache()[setIdx][w].tag == tag) {
                    usedWay = w;
                    break;
                }
            }

            cout << "#" << dec << setfill(' ') << setw(4) << left << (i + 1) << right
                 << "  addr: 0x" << hex << setw(4) << setfill('0') << addr
                 << "  tag: 0x" << setw(4) << tag
                 << "  set: " << dec << setfill(' ') << setw(3) << setIdx;

            if (hit) {
                cout << "  -> HIT (way " << usedWay << ")";
            } else if (!before[usedWay].valid) {
                cout << "  -> MISS (cold, placed in way " << usedWay << ")";
            } else {
                cout << "  -> MISS (evicted tag 0x" << hex << setw(4) << setfill('0')
                     << before[usedWay].tag << " from way " << dec << usedWay
                     << " via LRU)";
            }

            // Show LRU order after access
            cout << "  lru:[";
            const auto& lru = cache.getLruOrder()[setIdx];
            bool first = true;
            for (int w : lru) {
                if (!first) cout << ",";
                cout << w;
                first = false;
            }
            cout << "]\n";
        }

        cout << "\n--- Summary ---\n";
        cout << "Total accesses: " << addresses.size() << "\n";
        cout << "Hits: " << cache.getHits() << "  Misses: " << cache.getMisses()
             << "  Hit Rate: " << fixed << setprecision(2)
             << hitRate(cache.getHits(), cache.getMisses()) << "%\n";

    } else if (type == "fa") {
        FullyAssociativeCache cache(cacheSize, blockSize);
        int offBits = cache.getOffsetBits();

        cout << "Fully Associative Cache: " << cacheSize << " bytes, "
             << blockSize << " bytes/block, " << cache.getNumLines() << " lines\n";
        cout << "Address breakdown: [tag:" << (64 - offBits)
             << " | offset:" << offBits << "]\n\n";

        for (size_t i = 0; i < addresses.size(); i++) {
            uint64_t addr = addresses[i];
            uint64_t tag = addr >> offBits;

            // Snapshot before access
            vector<FALine> before(cache.getCache().begin(), cache.getCache().end());
            list<int> lruBefore(cache.getLruOrder().begin(), cache.getLruOrder().end());

            bool hit = cache.access(addr);

            // Find which line holds the tag now
            int usedLine = 0;
            for (size_t l = 0; l < cache.getCache().size(); l++) {
                if (cache.getCache()[l].valid && cache.getCache()[l].tag == tag) {
                    usedLine = l;
                    break;
                }
            }

            cout << "#" << dec << setfill(' ') << setw(4) << left << (i + 1) << right
                 << "  addr: 0x" << hex << setw(4) << setfill('0') << addr
                 << "  tag: 0x" << setw(4) << tag << dec << setfill(' ');

            if (hit) {
                cout << "  -> HIT (line " << usedLine << ")";
            } else if (!before[usedLine].valid) {
                cout << "  -> MISS (cold, placed in line " << usedLine << ")";
            } else {
                cout << "  -> MISS (evicted tag 0x" << hex << setw(4) << setfill('0')
                     << before[usedLine].tag << " from line " << dec << usedLine
                     << " via LRU)";
            }

            // Show LRU order after access
            cout << "  lru:[";
            const auto& lru = cache.getLruOrder();
            bool first = true;
            for (int l : lru) {
                if (!first) cout << ",";
                cout << l;
                first = false;
            }
            cout << "]\n";
        }

        cout << "\n--- Summary ---\n";
        cout << "Total accesses: " << addresses.size() << "\n";
        cout << "Hits: " << cache.getHits() << "  Misses: " << cache.getMisses()
             << "  Hit Rate: " << fixed << setprecision(2)
             << hitRate(cache.getHits(), cache.getMisses()) << "%\n";

    } else {
        cerr << "Error: unknown type '" << type << "'. Use dm, sa, or fa.\n";
        return 1;
    }

    return 0;
}
