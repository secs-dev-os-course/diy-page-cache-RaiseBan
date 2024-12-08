// include/lab2cache.h

#ifndef LAB2CACHE_H
#define LAB2CACHE_H

#ifdef LAB2CACHE_EXPORTS
#define LAB2CACHE_API __declspec(dllexport)
#else
#define LAB2CACHE_API __declspec(dllimport)
#endif

#include <windows.h>      // для HANDLE
#include <sys/types.h>    // для off_t
#include <cstddef>        // для size_t и ptrdiff_t
#include <cstdint>        // для int64_t
#include <cstdio>

// Условное определение ssize_t
#ifndef _SSIZE_T_DEFINED
typedef ptrdiff_t ssize_t;
#define _SSIZE_T_DEFINED
#endif

// Объявления функций API
extern "C" LAB2CACHE_API int lab2_open(const char *path);
extern "C" LAB2CACHE_API int lab2_close(int fd);
extern "C" LAB2CACHE_API ssize_t lab2_read(int fd, void *buf, size_t count);
extern "C" LAB2CACHE_API ssize_t lab2_write(int fd, const void *buf, size_t count);
extern "C" LAB2CACHE_API off_t lab2_lseek(int fd, off_t offset, int whence);
extern "C" LAB2CACHE_API int lab2_fsync(int fd);

#endif // LAB2CACHE_H
