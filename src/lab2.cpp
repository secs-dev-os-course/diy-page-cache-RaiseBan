#include "lab2.h"
#include "CacheManager.h"

extern "C" {

int lab2_open(const char* path) {
    if (!path) {
        std::cerr << "ERROR: lab2_open: path is nullptr" << std::endl;
        return -1;
    }
    return CacheManager::getInstance().openFile(path);
}

int lab2_close(int fd) {
    if (fd < 0) {
        std::cerr << "ERROR: lab2_close: invalid fd" << std::endl;
        return -1;
    }
    return CacheManager::getInstance().closeFile(fd);
}

int64_t lab2_read(int fd, void* buf, size_t count) {
    if (!buf || count == 0) {
        std::cerr << "ERROR: lab2_read: invalid parameters" << std::endl;
        return -1;
    }
    return CacheManager::getInstance().readFile(fd, buf, count);
}

int64_t lab2_write(int fd, const void* buf, size_t count) {
    if (!buf && count > 0) { // buf может быть nullptr только для count == 0
        std::cerr << "ERROR: lab2_write: buf is nullptr" << std::endl;
        return -1;
    }
    if (count > MAX_WRITE_LIMIT) {
        std::cerr << "ERROR: lab2_write: exceeds limit" << std::endl;
        return -1;
    }
    if (count == 0) {
        return 0; // Нулевая запись должна вернуть 0
    }
    return CacheManager::getInstance().writeFile(fd, buf, count);
}


int64_t lab2_lseek(int fd, int64_t offset, int whence) {
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        std::cerr << "ERROR: lab2_lseek: invalid whence" << std::endl;
        return -1;
    }
    return CacheManager::getInstance().seekFile(fd, offset, whence);
}

int lab2_fsync(int fd) {
    if (fd < 0) {
        std::cerr << "ERROR: lab2_fsync: invalid fd" << std::endl;
        return -1;
    }
    return CacheManager::getInstance().fsyncFile(fd);
}

} // extern "C"
