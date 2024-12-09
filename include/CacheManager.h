#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include "CacheBlock.h"
#include "FileHandle.h"
#include <unordered_map>
#include <mutex>
#include <vector>


// Общие константы
const size_t BLOCK_SIZE = 4096; // Размер блока
const size_t MAX_CACHE_BLOCKS = 1024; // Максимальное количество блоков в кэше
const size_t MAX_WRITE_LIMIT = 1 << 30; // Максимальный размер записи (1GB)


// Класс для управления кэшем блоков и файловыми операциями
class CacheManager {
public:
    // Получение единственного экземпляра (Singleton)
    static CacheManager& getInstance();

    // Открыть файл
    int openFile(const char* path);

    // Закрыть файл
    int closeFile(int fd);

    // Чтение из файла
    int64_t readFile(int fd, void* buf, size_t count);

    // Запись в файл
    int64_t writeFile(int fd, const void* buf, size_t count);

    // Переместить указатель в файле
    int64_t seekFile(int fd, int64_t offset, int whence);

    // Синхронизация данных из кэша на диск
    int fsyncFile(int fd);

private:
    CacheManager(); // Конструктор
    ~CacheManager(); // Деструктор

    // Получить блок из кэша (или загрузить его)
    CacheBlock* getCacheBlock(const BlockKey& key, bool for_write);

    // Прочитать блок данных с диска
    bool readBlockFromDisk(const BlockKey& key, char* buffer, size_t& out_valid_bytes);

    // Записать блок данных на диск
    bool writeBlockToDisk(const BlockKey& key, const char* buffer);

    // Вытеснить наименее используемый блок из кэша
    void evictBlock();

    // Сбросить данные конкретного файла на диск
    int flushFile(int fd);

    // Карта открытых файлов
    std::unordered_map<int, FileHandle> open_files_;

    // Карта блоков в кэше
    std::unordered_map<BlockKey, CacheBlock, BlockKeyHash> cache_map_;

    // Счётчик для выдачи уникальных дескрипторов файлов
    int next_fd_;

    // Мьютекс для потокобезопасности
    std::mutex mutex_;
};

#endif // CACHEMANAGER_H
