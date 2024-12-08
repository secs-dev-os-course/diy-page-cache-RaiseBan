// src/block_cache.cpp

#include "block_cache.h"
#include "platform_win.h"

#include <algorithm>
#include <iostream>

BlockCache::BlockCache(size_t capacity) : capacity_(capacity) {
    std::cout << "BlockCache: Initialized with capacity " << capacity_ << std::endl;
}

BlockCache::~BlockCache() {
    std::cout << "BlockCache: Destructor called." << std::endl;
    // При разрушении кэша можно сбросить все "грязные" блоки
    // Здесь оставляем пустым, так как flush должен быть вызван явно
}

bool BlockCache::read_block(int fd, uint64_t block_num, std::vector<char> &out_data) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto &file_cache = cache_[fd];
    auto it = file_cache.find(block_num);
    if (it != file_cache.end()) {
        // Блок найден в кэше
        it->second.access_count++;
        out_data = it->second.data;
        std::cout << "BlockCache::read_block: Block " << block_num << " found in cache." << std::endl;
        return true;
    }

    // Блок не найден, нужно загрузить с диска
    if (file_cache.size() >= capacity_) {
        evict(fd);
    }

    // Загрузка блока с диска
    CacheBlock new_block(block_num);
    std::cout << "BlockCache::read_block: Loading block " << block_num << " from disk." << std::endl;
    if (!platform_read_block(fd, block_num, new_block.data)) {
        std::cerr << "BlockCache::read_block: Failed to read block " << block_num << " from disk." << std::endl;
        return false;
    }
    std::cout << "after platform_read_block" << std::endl;
    new_block.access_count = 1;
    file_cache[block_num] = new_block;
    out_data = new_block.data;
    std::cout << "BlockCache::read_block: Block " << block_num << " loaded into cache." << std::endl;
    return true;
}

bool BlockCache::write_block(int fd, uint64_t block_num, const std::vector<char> &in_data) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto &file_cache = cache_[fd];
    auto it = file_cache.find(block_num);
    if (it != file_cache.end()) {
        // Блок найден в кэше
        it->second.data = in_data;
        it->second.dirty = true;
        it->second.access_count++;
        std::cout << "BlockCache::write_block: Block " << block_num << " updated in cache." << std::endl;
        return true;
    }

    // Блок не найден, нужно загрузить или создать
    if (file_cache.size() >= capacity_) {
        evict(fd);
    }

    // Создаем новый блок
    CacheBlock new_block(block_num);
    new_block.data = in_data;
    new_block.dirty = true;
    new_block.access_count = 1;
    file_cache[block_num] = new_block;
    std::cout << "BlockCache::write_block: Block " << block_num << " created and written to cache." << std::endl;
    return true;
}

void BlockCache::flush(int fd) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cache_.find(fd);
    if (it == cache_.end()) {
        std::cerr << "BlockCache::flush: No cache found for fd " << fd << std::endl;
        return;
    }

    std::cout << "BlockCache::flush: Flushing cache for fd " << fd << std::endl;
    for (auto &pair : it->second) {
        if (pair.second.dirty) {
            std::cout << "BlockCache::flush: Writing dirty block " << pair.first << " to disk." << std::endl;
            if (!platform_write_block(fd, pair.first, pair.second.data)) {
                std::cerr << "BlockCache::flush: Failed to write block " << pair.first << " to disk." << std::endl;
            } else {
                pair.second.dirty = false;
                std::cout << "BlockCache::flush: Block " << pair.first << " written to disk." << std::endl;
            }
        }
    }
}

void BlockCache::evict(int fd) {
    auto &file_cache = cache_[fd];
    if (file_cache.empty()) {
        std::cout << "BlockCache::evict: Cache is empty for fd " << fd << std::endl;
        return;
    }

    // Находим блок с наименьшим счетчиком обращений (LFU)
    auto it = std::min_element(file_cache.begin(), file_cache.end(),
                               [](const std::pair<uint64_t, CacheBlock> &a, const std::pair<uint64_t, CacheBlock> &b) -> bool {
                                   return a.second.access_count < b.second.access_count;
                               });

    if (it != file_cache.end()) {
        // Если блок "грязный", записываем его на диск
        if (it->second.dirty) {
            std::cout << "BlockCache::evict: Writing dirty block " << it->first << " to disk before eviction." << std::endl;
            if (!platform_write_block(fd, it->first, it->second.data)) {
                std::cerr << "BlockCache::evict: Failed to write block " << it->first << " to disk." << std::endl;
            } else {
                std::cout << "BlockCache::evict: Block " << it->first << " written to disk." << std::endl;
            }
        } else {
            std::cout << "BlockCache::evict: Evicting clean block " << it->first << " from cache." << std::endl;
        }
        // Удаляем блок из кэша
        file_cache.erase(it);
        std::cout << "BlockCache::evict: Block evicted." << std::endl;
    }
}
