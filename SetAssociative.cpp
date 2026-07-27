#include "SetAssociative.h"

SetAssociativeCache::SetAssociativeCache(size_t cacheSize, int bytesPerBlock, int ways)
    : cacheSize(cacheSize), bytesPerBlock(bytesPerBlock), ways(ways), hits(0), misses(0) {
    size_t numSets = cacheSize / (bytesPerBlock * ways);
    cache.resize(numSets, vector<SetLine>(ways));
    lruOrder.resize(numSets);
}

bool SetAssociativeCache::access(uint64_t address) {
    size_t index = getIndex(address);
    uint64_t tag = getTag(address);

    // Check each way in the set for a hit
    for (int w = 0; w < ways; w++) {
        if (cache[index][w].valid && cache[index][w].tag == tag) {
            hits++;
            updateLRU(index, w);
            return true;
        }
    }

    // Miss — find an empty way or evict LRU
    misses++;

    // Look for an empty way first
    for (int w = 0; w < ways; w++) {
        if (!cache[index][w].valid) {
            cache[index][w].valid = true;
            cache[index][w].tag = tag;
            updateLRU(index, w);
            return false;
        }
    }

    // All ways full — evict the LRU (back of list)
    int victim = lruOrder[index].back();
    cache[index][victim].tag = tag;
    updateLRU(index, victim);
    return false;
}

void SetAssociativeCache::updateLRU(size_t setIndex, int way) {
    lruOrder[setIndex].remove(way);
    lruOrder[setIndex].push_front(way);
}

uint64_t SetAssociativeCache::getTag(uint64_t address) {
    return address >> (getOffsetBits() + getIndexBits());
}

size_t SetAssociativeCache::getIndex(uint64_t address) {
    size_t numSets = cacheSize / (bytesPerBlock * ways);
    return (address >> getOffsetBits()) & (numSets - 1);
}

uint64_t SetAssociativeCache::getHits() const {
    return hits;
}

uint64_t SetAssociativeCache::getMisses() const {
    return misses;
}

size_t SetAssociativeCache::getNumSets() const {
    return cache.size();
}

int SetAssociativeCache::getWays() const {
    return ways;
}

const vector<vector<SetLine>>& SetAssociativeCache::getCache() const {
    return cache;
}

const vector<list<int>>& SetAssociativeCache::getLruOrder() const {
    return lruOrder;
}

int SetAssociativeCache::getOffsetBits() const {
    int bits = 0;
    int val = bytesPerBlock;
    while (val > 1) {
        val >>= 1;
        bits++;
    }
    return bits;
}

int SetAssociativeCache::getIndexBits() const {
    int bits = 0;
    size_t numSets = cacheSize / (bytesPerBlock * ways);
    while (numSets > 1) {
        numSets >>= 1;
        bits++;
    }
    return bits;
}
