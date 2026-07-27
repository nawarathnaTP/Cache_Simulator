#ifndef DIRECT_MAPPED_CACHE_H
#define DIRECT_MAPPED_CACHE_H

#include <cstdint>
#include <vector>

struct cacheLine {
    bool valid =  false;
    uint64_t tag = 0;
};

class DirectMappedCache {
    public:
        DirectMappedCache(size_t cacheSize, int bytesPerBlock);

        bool access(uint64_t address);

        uint64_t getHits() const;
        uint64_t getMisses() const;
        size_t getNumLines() const;
        const std::vector<cacheLine>& getCache() const;
        int getOffsetBits() const;
        int getIndexBits() const;
    
    private:
        std::vector<cacheLine> cache;
        size_t cacheSize;
        int bytesPerBlock;

        uint64_t hits;
        uint64_t misses;

        uint64_t getTag(uint64_t address);
        size_t getIndex(uint64_t address);
};

#endif


