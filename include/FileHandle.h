#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <windows.h>
#include <cstdint>

// Структура для управления состоянием открытого файла
struct FileHandle {
    HANDLE handle;          // Дескриптор файла (Windows HANDLE)
    int64_t current_offset; // Текущая позиция в файле
    int64_t known_size;     // Известный логический размер файла

    // Конструктор по умолчанию
    FileHandle();

    // Конструктор с параметром HANDLE
    FileHandle(HANDLE h);
};

#endif // FILEHANDLE_H
