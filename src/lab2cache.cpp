// src/lab2cache.cpp

#include "lab2cache.h"
#include "block_cache.h"
#include "platform_win.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <iostream>

// Экземпляр кэша (можно настроить емкость по необходимости)
static BlockCache cache(1024); // Например, 1024 блока

// Генератор уникальных fd
static int generate_fd() {
    static int current_fd = 0;
    return ++current_fd;
}

// Объявление глобальных переменных
extern std::unordered_map<int, HANDLE> g_fd_table;
extern std::mutex g_fd_table_mtx;

extern "C" LAB2CACHE_API int lab2_open(const char *path) {
    HANDLE hFile = platform_open_file(path);
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }
    std::lock_guard<std::mutex> lock(g_fd_table_mtx);
    int fd = generate_fd();
    g_fd_table[fd] = hFile;
    std::cout << "lab2_open: File opened with fd " << fd << std::endl;
    return fd;
}

extern "C" LAB2CACHE_API int lab2_close(int fd) {
    std::lock_guard<std::mutex> lock(g_fd_table_mtx);
    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "lab2_close: Invalid fd " << fd << std::endl;
        return -1;
    }
    cache.flush(fd);
    platform_close_file(it->second);
    g_fd_table.erase(it);
    std::cout << "lab2_close: File with fd " << fd << " closed." << std::endl;
    return 0;
}

extern "C" LAB2CACHE_API ssize_t lab2_read(int fd, void *buf, size_t count) {
    CriticalSectionLock lock(&g_fd_table_cs);
    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "lab2_read: Invalid fd " << fd << std::endl;
        return -1; // Неверный fd
    }

    HANDLE hFile = it->second;

    // Получаем текущую позицию файла
    LARGE_INTEGER current_pos;
    if (!SetFilePointerEx(hFile, {0}, &current_pos, FILE_CURRENT)) {
        std::cerr << "lab2_read: SetFilePointerEx failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_read: Current position: " << current_pos.QuadPart << std::endl;

    // Рассчитываем номер блока и смещение внутри блока
    uint64_t block_num = current_pos.QuadPart / BLOCK_SIZE;
    size_t offset = current_pos.QuadPart % BLOCK_SIZE;

    std::cout << "lab2_read: Block number: " << block_num << ", Offset: " << offset << std::endl;

    // Читаем блок из кэша
    std::vector<char> block_data(BLOCK_SIZE);
    if (!cache.read_block(fd, block_num, block_data)) {
        std::cerr << "lab2_read: Cache read_block failed." << std::endl;
        return -1;
    }
    std::cout << "lab2_read: 1 " << std::endl;
    // Копируем данные в буфер
    size_t bytes_to_copy = std::min(count, BLOCK_SIZE - offset);
    std::cout << "lab2_read: 2 " << std::endl;
    memcpy(buf, block_data.data() + offset, bytes_to_copy);
    std::cout << "lab2_read: 3 " << std::endl;

    // Обновляем позицию файла
    LARGE_INTEGER new_pos;
    new_pos.QuadPart = current_pos.QuadPart + bytes_to_copy;
    if (!SetFilePointerEx(hFile, new_pos, NULL, FILE_BEGIN)) {
        std::cerr << "lab2_read: SetFilePointerEx (new_pos) failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_read: Read " << bytes_to_copy << " bytes, New position: " << new_pos.QuadPart << std::endl;

    return bytes_to_copy;
}

extern "C" LAB2CACHE_API ssize_t lab2_write(int fd, const void *buf, size_t count) {
    std::lock_guard<std::mutex> lock(g_fd_table_mtx);
    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "lab2_write: Invalid fd " << fd << std::endl;
        return -1; // Неверный fd
    }

    HANDLE hFile = it->second;

    // Получаем текущую позицию файла
    LARGE_INTEGER current_pos;
    if (!SetFilePointerEx(hFile, {0}, &current_pos, FILE_CURRENT)) {
        std::cerr << "lab2_write: SetFilePointerEx failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_write: Current position: " << current_pos.QuadPart << std::endl;

    // Рассчитываем номер блока и смещение внутри блока
    uint64_t block_num = current_pos.QuadPart / BLOCK_SIZE;
    size_t offset = current_pos.QuadPart % BLOCK_SIZE;

    std::cout << "lab2_write: Block number: " << block_num << ", Offset: " << offset << std::endl;

    // Читаем блок из кэша
    std::vector<char> block_data(BLOCK_SIZE);
    if (!cache.read_block(fd, block_num, block_data)) {
        std::cerr << "lab2_write: Cache read_block failed." << std::endl;
        return -1;
    }

    // Копируем данные из буфера в блок
    size_t bytes_to_copy = std::min(count, BLOCK_SIZE - offset);
    memcpy(block_data.data() + offset, buf, bytes_to_copy);

    // Записываем блок обратно в кэш
    if (!cache.write_block(fd, block_num, block_data)) {
        std::cerr << "lab2_write: Cache write_block failed." << std::endl;
        return -1;
    }

    // Обновляем позицию файла
    LARGE_INTEGER new_pos;
    new_pos.QuadPart = current_pos.QuadPart + bytes_to_copy;
    if (!SetFilePointerEx(hFile, new_pos, NULL, FILE_BEGIN)) {
        std::cerr << "lab2_write: SetFilePointerEx (new_pos) failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_write: Wrote " << bytes_to_copy << " bytes, New position: " << new_pos.QuadPart << std::endl;

    return bytes_to_copy;
}

extern "C" LAB2CACHE_API off_t lab2_lseek(int fd, off_t offset, int whence) {
    std::lock_guard<std::mutex> lock(g_fd_table_mtx);
    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "lab2_lseek: Invalid fd " << fd << std::endl;
        return -1; // Неверный fd
    }

    HANDLE hFile = it->second;
    LARGE_INTEGER li_offset;
    LARGE_INTEGER new_pos;
    DWORD move_method;

    switch (whence) {
        case SEEK_SET:
            move_method = FILE_BEGIN;
            break;
        case SEEK_CUR:
            move_method = FILE_CURRENT;
            break;
        case SEEK_END:
            move_method = FILE_END;
            break;
        default:
            std::cerr << "lab2_lseek: Invalid whence " << whence << std::endl;
            return -1; // Неверное значение whence
    }

    li_offset.QuadPart = offset;
    if (!SetFilePointerEx(hFile, li_offset, &new_pos, move_method)) {
        std::cerr << "lab2_lseek: SetFilePointerEx failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_lseek: New position: " << new_pos.QuadPart << std::endl;

    return new_pos.QuadPart;
}

extern "C" LAB2CACHE_API int lab2_fsync(int fd) {
    std::lock_guard<std::mutex> lock(g_fd_table_mtx);
    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "lab2_fsync: Invalid fd " << fd << std::endl;
        return -1; // Неверный fd
    }

    // Сбрасываем кэш для данного файла
    cache.flush(fd);

    // Выполняем FlushFileBuffers для гарантии записи на диск
    if (!FlushFileBuffers(it->second)) {
        std::cerr << "lab2_fsync: FlushFileBuffers failed: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "lab2_fsync: File synchronized successfully." << std::endl;

    return 0;
}
