#ifndef CACHEBLOCK_H
#define CACHEBLOCK_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <cstdlib>


// Структура для ключа блока в кэше
struct BlockKey {
    int fd;                     // Файловый дескриптор
    int64_t block_number;       // Номер блока в файле

    // Оператор сравнения для ключей
    bool operator==(const BlockKey& other) const;
};

// Хэш-функция для BlockKey
struct BlockKeyHash {
    std::size_t operator()(const BlockKey& key) const;
};

// Структура для хранения данных в блоке
struct CacheBlock {
    char* data;                // Указатель на данные блока
    int frequency;             // Частота использования блока
    bool dirty;                // Флаг "грязности" (изменён ли блок)
    size_t valid_bytes;        // Количество валидных данных в блоке

    // Конструктор по умолчанию
    CacheBlock();

    // Конструктор копирования
    CacheBlock(const CacheBlock& other);

    // Оператор присваивания
    CacheBlock& operator=(const CacheBlock& other);

    // Деструктор
    ~CacheBlock();
};

#endif // CACHEBLOCK_H
