#include "CacheBlock.h"
#include "CacheManager.h"
#include <iostream>
#include <cstring>


bool BlockKey::operator==(const BlockKey& other) const {
    return fd == other.fd && block_number == other.block_number;
}

std::size_t BlockKeyHash::operator()(const BlockKey &key) const {
    return std::hash<int>()(key.fd) ^ std::hash<int64_t>()(key.block_number);
}

CacheBlock::CacheBlock() : data(nullptr), frequency(1), dirty(false), valid_bytes(0) {
    data = (char*)_aligned_malloc(BLOCK_SIZE, BLOCK_SIZE);
    if (!data) {
        std::cout << "ERROR: Failed to allocate aligned memory for CacheBlock." << std::endl;
    }
    memset(data, 0, BLOCK_SIZE);
}

CacheBlock::CacheBlock(const CacheBlock& other) {
    data = (char *) _aligned_malloc(BLOCK_SIZE, BLOCK_SIZE);
    if (!data) {
        std::cout << "ERROR: Failed to allocate aligned memory (copy)." << std::endl;
    }
    memcpy(data, other.data, BLOCK_SIZE);
    frequency = other.frequency;
    dirty = other.dirty;
    valid_bytes = other.valid_bytes;
}

CacheBlock& CacheBlock::operator=(const CacheBlock& other) {
    if (this != &other) {
        if (!data) {
            data = (char*)_aligned_malloc(BLOCK_SIZE, BLOCK_SIZE);
            if (!data) {
                std::cout << "ERROR: Failed to allocate aligned memory (assign)." << std::endl;
            }
        }
        memcpy(data, other.data, BLOCK_SIZE);
        frequency = other.frequency;
        dirty = other.dirty;
        valid_bytes = other.valid_bytes;
    }
    return *this;
}

CacheBlock::~CacheBlock() {
    if (data) {
        _aligned_free(data);
        data = nullptr;
    }
}
