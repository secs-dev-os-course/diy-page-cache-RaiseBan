#ifndef LAB2_H
#define LAB2_H

#ifdef lab2_EXPORTS
#define LAB2_API __declspec(dllexport)
#else
#define LAB2_API __declspec(dllimport)
#endif

#include <cstddef>
#include <cstdint>

extern "C" {
// Открытие файла. Возвращает файловый дескриптор или -1 в случае ошибки.
LAB2_API int lab2_open(const char* path);

// Закрытие файла. Возвращает 0 при успешном закрытии, -1 в случае ошибки.
LAB2_API int lab2_close(int fd);

// Чтение данных из файла. Возвращает количество прочитанных байт или -1 в случае ошибки.
LAB2_API int64_t lab2_read(int fd, void* buf, size_t count);

// Запись данных в файл. Возвращает количество записанных байт или -1 в случае ошибки.
LAB2_API int64_t lab2_write(int fd, const void* buf, size_t count);

// Перемещение позиции указателя в файле. Возвращает новую позицию или -1 в случае ошибки.
LAB2_API int64_t lab2_lseek(int fd, int64_t offset, int whence);

// Синхронизация данных из кэша с диском. Возвращает 0 при успешной синхронизации, -1 в случае ошибки.
LAB2_API int lab2_fsync(int fd);
}

#endif // LAB2_H
