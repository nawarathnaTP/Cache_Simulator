#ifndef SET_ASSOCIATIVE_CACHE_H
#define SET_ASSOCIATIVE_CACHE_H

#include <cstdint>
#include <vector>
#include <list>

using namespace std;

struct SetLine {
    bool valid = false;
    uint64_t tag = 0;
};

class SetAssociativeCache {
    public:
        SetAssociativeCache(size_t cacheSize, int bytesPerBlock, int ways);

        bool access(uint64_t address);

        uint64_t getHits() const;
        uint64_t getMisses() const;
        size_t getNumSets() const;
        int getWays() const;
        const vector<vector<SetLine>>& getCache() const;
        const vector<list<int>>& getLruOrder() const;
        int getOffsetBits() const;
        int getIndexBits() const;

    private:
        vector<vector<SetLine>> cache;
        vector<list<int>> lruOrder;
        size_t cacheSize;
        int bytesPerBlock;
        int ways;

        uint64_t hits;
        uint64_t misses;

        uint64_t getTag(uint64_t address);
        size_t getIndex(uint64_t address);
        void updateLRU(size_t setIndex, int way);
};

#endif
