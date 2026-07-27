#ifndef FULLY_ASSOCIATIVE_CACHE_H
#define FULLY_ASSOCIATIVE_CACHE_H

#include <cstdint>
#include <vector>
#include <list>

using namespace std;

struct FALine {
    bool valid = false;
    uint64_t tag = 0;
};

class FullyAssociativeCache {
    public:
        FullyAssociativeCache(size_t cacheSize, int bytesPerBlock);

        bool access(uint64_t address);

        uint64_t getHits() const;
        uint64_t getMisses() const;
        size_t getNumLines() const;
        const vector<FALine>& getCache() const;
        const list<int>& getLruOrder() const;
        int getOffsetBits() const;

    private:
        vector<FALine> cache;
        list<int> lruOrder;
        size_t cacheSize;
        int bytesPerBlock;

        uint64_t hits;
        uint64_t misses;

        uint64_t getTag(uint64_t address);
};

#endif
