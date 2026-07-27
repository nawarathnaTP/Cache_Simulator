#include "DirectMapped.h"

DirectMappedCache::DirectMappedCache(size_t cacheSize, int bytesPerBlock)
    : cacheSize(cacheSize), bytesPerBlock(bytesPerBlock), hits(0), misses(0) {
    size_t numLines = cacheSize / bytesPerBlock;
    cache.resize(numLines);
}

bool DirectMappedCache::access(uint64_t address) {
    size_t index = getIndex(address);
    uint64_t tag = getTag(address);

    if (cache[index].valid && cache[index].tag == tag) {
        hits++;
        return true;
    }

    cache[index].valid = true;
    cache[index].tag = tag;
    misses++;
    return false;
}

uint64_t DirectMappedCache::getTag(uint64_t address) {
    return address >> (getOffsetBits() + getIndexBits());
}

size_t DirectMappedCache::getIndex(uint64_t address) {
    size_t numLines = cacheSize / bytesPerBlock;
    return (address >> getOffsetBits()) & (numLines - 1);
}

uint64_t DirectMappedCache::getHits() const {
    return hits;
}

uint64_t DirectMappedCache::getMisses() const {
    return misses;
}

size_t DirectMappedCache::getNumLines() const {
    return cache.size();
}

const std::vector<cacheLine>& DirectMappedCache::getCache() const {
    return cache;
}

int DirectMappedCache::getOffsetBits() const {
    int bits = 0;
    int val = bytesPerBlock;
    while (val > 1) {
        val >>= 1;
        bits++;
    }
    return bits;
}

int DirectMappedCache::getIndexBits() const {
    int bits = 0;
    size_t numLines = cacheSize / bytesPerBlock;
    while (numLines > 1) {
        numLines >>= 1;
        bits++;
    }
    return bits;
}
