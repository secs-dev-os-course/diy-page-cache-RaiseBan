#include <cstdint>

#ifdef _WIN32
#ifdef LAB2_EXPORTS
#define LAB2_API __declspec(dllexport)
#else
#define LAB2_API __declspec(dllimport)
#endif
#else
#define LAB2_API
#endif

extern "C" {
LAB2_API int lab2_open(const char* path);
LAB2_API int lab2_close(int fd);
LAB2_API int64_t lab2_read(int fd, void* buf, size_t count);
LAB2_API int64_t lab2_write(int fd, const void* buf, size_t count);
LAB2_API int64_t lab2_lseek(int fd, int64_t offset, int whence);
LAB2_API int lab2_fsync(int fd);
}
