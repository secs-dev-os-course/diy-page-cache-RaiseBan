// src/platform_win.h

#ifndef PLATFORM_WIN_H
#define PLATFORM_WIN_H

#include <windows.h>
#include <cstdint>
#include <vector>
#include <unordered_map>

// Глобальные переменные для управления файловыми дескрипторами
extern std::unordered_map<int, HANDLE> g_fd_table;

// Используем CRITICAL_SECTION вместо std::mutex
extern CRITICAL_SECTION g_fd_table_cs;

// Функции платформо-зависимых операций
HANDLE platform_open_file(const char *path);
void platform_close_file(HANDLE hFile);
bool platform_read_block(int fd, uint64_t block_num, std::vector<char> &data);
bool platform_write_block(int fd, uint64_t block_num, const std::vector<char> &data);

#endif // PLATFORM_WIN_H
