// include/block_cache.h

#ifndef BLOCK_CACHE_H
#define BLOCK_CACHE_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <mutex>

// Размер блока 4KB
constexpr size_t BLOCK_SIZE = 4096;

struct CacheBlock {
    uint64_t block_number;         // Номер блока в файле
    std::vector<char> data;        // Данные блока
    bool dirty;                    // Флаг "грязности" блока
    size_t access_count;           // Счетчик обращений (для LFU)

    // Конструктор по умолчанию
    CacheBlock() : block_number(0), data(BLOCK_SIZE, 0), dirty(false), access_count(0) {}

    // Конструктор с номером блока
    CacheBlock(uint64_t num) : block_number(num), data(BLOCK_SIZE, 0), dirty(false), access_count(0) {}
};

class BlockCache {
public:
    BlockCache(size_t capacity);
    ~BlockCache();

    bool read_block(int fd, uint64_t block_num, std::vector<char> &out_data);
    bool write_block(int fd, uint64_t block_num, const std::vector<char> &in_data);
    void flush(int fd);

private:
    size_t capacity_;
    // Изменили структуру кэша для поддержки разных файлов
    std::unordered_map<int, std::unordered_map<uint64_t, CacheBlock>> cache_;
    std::mutex mtx_;

    // Внутренние методы для управления кэшем
    void evict(int fd);
};

#endif // BLOCK_CACHE_H
