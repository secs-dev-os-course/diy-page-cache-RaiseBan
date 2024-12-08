// src/platform_win.cpp

#include "platform_win.h"
#include "block_cache.h"

#include <iostream>

// Определение глобальных переменных
std::unordered_map<int, HANDLE> g_fd_table;
CRITICAL_SECTION g_fd_table_cs;

// Флаг инициализации CRITICAL_SECTION
bool g_cs_initialized = false;

// RAII-класс для CRITICAL_SECTION
class CriticalSectionLock {
public:
    CriticalSectionLock(CRITICAL_SECTION* cs) : cs_(cs), locked_(false) {
        EnterCriticalSection(cs_);
        locked_ = true;
    }

    ~CriticalSectionLock() {
        if (locked_) {
            LeaveCriticalSection(cs_);
        }
    }

    // Запрещаем копирование
    CriticalSectionLock(const CriticalSectionLock&) = delete;
    CriticalSectionLock& operator=(const CriticalSectionLock&) = delete;

private:
    CRITICAL_SECTION* cs_;
    bool locked_;
};

// Открытие файла с флагами для обхода системного кэша
HANDLE platform_open_file(const char *path) {
    if (!g_cs_initialized) {
        InitializeCriticalSection(&g_fd_table_cs);
        g_cs_initialized = true;
    }

    HANDLE hFile = CreateFileA(
            path,
            GENERIC_READ | GENERIC_WRITE,
            0, // Без совместного доступа
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, // Убраны FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH
            NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "platform_open_file: Failed to open file: " << path << " Error: " << GetLastError() << std::endl;
    } else {
        std::cout << "platform_open_file: File opened successfully." << std::endl;
    }

    return hFile;
}

// Закрытие файла
void platform_close_file(HANDLE hFile) {
    if (CloseHandle(hFile)) {
        std::cout << "platform_close_file: File closed successfully." << std::endl;
    } else {
        std::cerr << "platform_close_file: Failed to close file. Error: " << GetLastError() << std::endl;
    }
}

// Чтение блока с диска
bool platform_read_block(int fd, uint64_t block_num, std::vector<char> &data) {
    std::cout << "platform_read_block: Entered function." << std::endl;

    // Захват CRITICAL_SECTION с помощью RAII
    CriticalSectionLock lock(&g_fd_table_cs);
    std::cout << "platform_read_block: Entered critical section." << std::endl;

    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "platform_read_block: Invalid fd " << fd << std::endl;
        return false;
    }
    HANDLE hFile = it->second;
    std::cout << "platform_read_block: hFile obtained." << std::endl;

    LARGE_INTEGER li;
    li.QuadPart = block_num * BLOCK_SIZE;
    std::cout << "platform_read_block: Setting file pointer to position " << li.QuadPart << std::endl;

    if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN)) {
        std::cerr << "platform_read_block: SetFilePointerEx failed with error " << GetLastError() << std::endl;
        return false;
    }
    std::cout << "platform_read_block: SetFilePointerEx succeeded." << std::endl;

    DWORD bytesRead = 0;
    std::cout << "platform_read_block: Calling ReadFile with BLOCK_SIZE = " << BLOCK_SIZE << std::endl;

    BOOL success = ReadFile(
            hFile,
            data.data(),
            BLOCK_SIZE,
            &bytesRead,
            NULL
    );

    std::cout << "platform_read_block: ReadFile returned success = " << success << ", bytesRead = " << bytesRead << std::endl;

    if (!success) {
        std::cerr << "platform_read_block: ReadFile failed with error " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "platform_read_block: Read " << bytesRead << " bytes from block " << block_num << std::endl;

    // Если считано меньше BLOCK_SIZE, возможно конец файла
    if (bytesRead < BLOCK_SIZE) {
        std::cerr << "platform_read_block: Unexpected bytesRead (" << bytesRead << " < " << BLOCK_SIZE << ")" << std::endl;
        // Заполняем оставшуюся часть буфера нулями
        std::fill(data.begin() + bytesRead, data.end(), 0);
        std::cout << "platform_read_block: Filled remaining buffer with zeros." << std::endl;
    }

    std::cout << "platform_read_block: Function completed successfully." << std::endl;
    return true;
}

// Запись блока на диск
bool platform_write_block(int fd, uint64_t block_num, const std::vector<char> &data) {
    std::cout << "platform_write_block: Entered function." << std::endl;

    // Захват CRITICAL_SECTION с помощью RAII
    CriticalSectionLock lock(&g_fd_table_cs);
    std::cout << "platform_write_block: Entered critical section." << std::endl;

    auto it = g_fd_table.find(fd);
    if (it == g_fd_table.end()) {
        std::cerr << "platform_write_block: Invalid fd " << fd << std::endl;
        return false;
    }
    HANDLE hFile = it->second;
    std::cout << "platform_write_block: hFile obtained." << std::endl;

    LARGE_INTEGER li;
    li.QuadPart = block_num * BLOCK_SIZE;
    std::cout << "platform_write_block: Setting file pointer to position " << li.QuadPart << std::endl;

    if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN)) {
        std::cerr << "platform_write_block: SetFilePointerEx failed with error " << GetLastError() << std::endl;
        return false;
    }
    std::cout << "platform_write_block: SetFilePointerEx succeeded." << std::endl;

    DWORD bytesWritten = 0;
    std::cout << "platform_write_block: Calling WriteFile with BLOCK_SIZE = " << BLOCK_SIZE << std::endl;

    BOOL success = WriteFile(
            hFile,
            data.data(),
            BLOCK_SIZE,
            &bytesWritten,
            NULL
    );

    std::cout << "platform_write_block: WriteFile returned success = " << success << ", bytesWritten = " << bytesWritten << std::endl;

    if (!success) {
        std::cerr << "platform_write_block: WriteFile failed with error " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "platform_write_block: Wrote " << bytesWritten << " bytes to block " << block_num << std::endl;

    std::cout << "platform_write_block: Function completed successfully." << std::endl;
    return true;
}
