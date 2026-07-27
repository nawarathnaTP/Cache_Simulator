#include "FullyAssociative.h"

FullyAssociativeCache::FullyAssociativeCache(size_t cacheSize, int bytesPerBlock)
    : cacheSize(cacheSize), bytesPerBlock(bytesPerBlock), hits(0), misses(0) {
    size_t numLines = cacheSize / bytesPerBlock;
    cache.resize(numLines);
}

bool FullyAssociativeCache::access(uint64_t address) {
    uint64_t tag = getTag(address);

    // Check every line for a hit
    for (size_t i = 0; i < cache.size(); i++) {
        if (cache[i].valid && cache[i].tag == tag) {
            hits++;
            lruOrder.remove(i);
            lruOrder.push_front(i);
            return true;
        }
    }

    // Miss — find an empty line or evict LRU
    misses++;

    // Look for an empty line first
    for (size_t i = 0; i < cache.size(); i++) {
        if (!cache[i].valid) {
            cache[i].valid = true;
            cache[i].tag = tag;
            lruOrder.push_front(i);
            return false;
        }
    }

    // All lines full — evict the LRU (back of list)
    int victim = lruOrder.back();
    lruOrder.pop_back();
    cache[victim].tag = tag;
    lruOrder.push_front(victim);
    return false;
}

uint64_t FullyAssociativeCache::getTag(uint64_t address) {
    return address >> getOffsetBits();
}

uint64_t FullyAssociativeCache::getHits() const {
    return hits;
}

uint64_t FullyAssociativeCache::getMisses() const {
    return misses;
}

size_t FullyAssociativeCache::getNumLines() const {
    return cache.size();
}

const vector<FALine>& FullyAssociativeCache::getCache() const {
    return cache;
}

const list<int>& FullyAssociativeCache::getLruOrder() const {
    return lruOrder;
}

int FullyAssociativeCache::getOffsetBits() const {
    int bits = 0;
    int val = bytesPerBlock;
    while (val > 1) {
        val >>= 1;
        bits++;
    }
    return bits;
}
