#include "FileHandle.h"

// Конструктор по умолчанию
FileHandle::FileHandle()
        : handle(INVALID_HANDLE_VALUE), current_offset(0), known_size(0) {}

// Конструктор, принимающий дескриптор файла
FileHandle::FileHandle(HANDLE h)
        : handle(h), current_offset(0), known_size(0) {}
