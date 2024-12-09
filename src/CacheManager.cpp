#include "CacheManager.h"
#include <windows.h>
#include <iostream>
#include <algorithm>



CacheManager& CacheManager::getInstance() {
    static CacheManager instance;
    return instance;
}


int CacheManager::openFile(const char* path) {
    std::lock_guard<std::mutex> lock(mutex_);
    HANDLE hFile = CreateFileA(
            path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
            nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "ERROR: openFile: Failed to open file " << path << std::endl;
        return -1;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        size.QuadPart = 0;
    }

    int fd = next_fd_++;
    FileHandle fh(hFile);
    fh.known_size = size.QuadPart;
    open_files_[fd] = fh;
    return fd;
}

int CacheManager::closeFile(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = open_files_.find(fd);
    if (it == open_files_.end()) {
        std::cerr << "ERROR: closeFile: Invalid file descriptor " << fd << std::endl;
        return -1;
    }

    int res = flushFile(fd);
    if (res != 0) {
        std::cerr << "ERROR: closeFile: Failed to flush file " << fd << std::endl;
    }

    CloseHandle(it->second.handle);
    open_files_.erase(it);
    return 0;
}

int64_t CacheManager::readFile(int fd, void* buf, size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!buf || count == 0) {
        std::cerr << "ERROR: readFile: Invalid parameters" << std::endl;
        return -1;
    }

    auto it = open_files_.find(fd);
    if (it == open_files_.end()) {
        std::cout << "ERROR: readFile: Invalid fd" << std::endl;
        return -1;
    }

    FileHandle& fh = it->second;

    size_t bytes_read = 0;
    char* buffer = (char*)buf;

    while (bytes_read < count) {
        if (fh.current_offset >= fh.known_size) {
            break;
        }

        int64_t block_number = fh.current_offset / BLOCK_SIZE;
        size_t block_offset = (size_t)(fh.current_offset % BLOCK_SIZE);
        size_t bytes_left = count - bytes_read;

        CacheBlock* cb = getCacheBlock({fd, block_number}, false);
        if (!cb) {
            break;
        }

        if (block_offset >= cb->valid_bytes) {
            break;
        }

        size_t available = cb->valid_bytes - block_offset;
        size_t to_read = std::min(available, bytes_left);
        memcpy(buffer + bytes_read, cb->data + block_offset, to_read);

        bytes_read += to_read;
        fh.current_offset += to_read;
    }

    return (int64_t)bytes_read;
}

int64_t CacheManager::writeFile(int fd, const void* buf, size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!buf) {
        std::cout << "ERROR: writeFile: buf is nullptr" << std::endl;
        return -1;
    }
    if (count == 0) {
        return 0;
    }
    if (count > MAX_WRITE_LIMIT) {
        std::cout << "ERROR: writeFile: exceeds limit" << std::endl;
        return -1;
    }

    auto it = open_files_.find(fd);
    if (it == open_files_.end()) {
        std::cout << "ERROR: writeFile: Invalid fd" << std::endl;
        return -1;
    }

    FileHandle& fh = it->second;

    size_t bytes_written = 0;
    const char* buffer = (const char*)buf;

    while (bytes_written < count) {
        int64_t current_offset = fh.current_offset;
        int64_t block_number = current_offset / BLOCK_SIZE;
        auto block_offset = (size_t)(current_offset % BLOCK_SIZE);
        size_t to_write = std::min(BLOCK_SIZE - block_offset, count - bytes_written);

        CacheBlock* cb = getCacheBlock({fd, block_number}, true);
        if (!cb) {
            std::cout << "ERROR: writeFile: Failed to get block block_number=" << block_number << std::endl;
            return -1;
        }

        memcpy(cb->data + block_offset, buffer + bytes_written, to_write);
        cb->frequency++;
        cb->dirty = true;
        size_t end_of_data = block_offset + to_write;
        if (end_of_data > cb->valid_bytes) {
            cb->valid_bytes = end_of_data;
        }

        bytes_written += to_write;
        fh.current_offset += to_write;
    }

    if (fh.current_offset > fh.known_size) {
        fh.known_size = fh.current_offset;
    }

    return (int64_t)bytes_written;
}

int64_t CacheManager::seekFile(int fd, int64_t offset, int whence) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = open_files_.find(fd);
    if (it == open_files_.end()) {
        std::cout << "ERROR: seekFile: Invalid fd" << std::endl;
        return -1;
    }

    FileHandle& fh = it->second;
    int64_t new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = fh.current_offset + offset;
            break;
        case SEEK_END:
            new_offset = fh.known_size + offset;
            break;
        default:
            std::cout << "ERROR: seekFile: Invalid whence value " << whence << std::endl;
            return -1;
    }

    if (new_offset < 0) {
        std::cout << "ERROR: seekFile: resulting offset < 0" << std::endl;
        return -1;
    }

    fh.current_offset = new_offset;
    return fh.current_offset;
}

int CacheManager::fsyncFile(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    return flushFile(fd);
}



CacheManager::CacheManager() : next_fd_(0) {}

CacheManager::~CacheManager() {
    for (auto& p : open_files_) {
        CloseHandle(p.second.handle);
    }
}

CacheBlock* CacheManager::getCacheBlock(const BlockKey& key, bool for_write) {
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        it->second.frequency++;
        return &it->second;
    }

    // Добавляем проверку лимита
    if (cache_map_.size() >= MAX_CACHE_BLOCKS) {
        evictBlock();
    }

    CacheBlock new_block;
    if (!for_write) {
        size_t valid_bytes = 0;
        bool read_res = readBlockFromDisk(key, new_block.data, valid_bytes);
        auto f_it = open_files_.find(key.fd);
        int64_t block_start = key.block_number * BLOCK_SIZE;
        if (!read_res && valid_bytes == 0) {
            return nullptr;
        }

        if (f_it != open_files_.end()) {
            int64_t end = f_it->second.known_size;
            int64_t block_end = block_start + valid_bytes;
            if (block_end > end) {
                valid_bytes = (end > block_start) ? (size_t)(end - block_start) : 0;
            }
        }
        new_block.valid_bytes = valid_bytes;
    }
    cache_map_[key] = new_block;
    return &cache_map_[key];
}

bool CacheManager::readBlockFromDisk(const BlockKey& key, char* buffer, size_t& out_valid_bytes) {
    auto it = open_files_.find(key.fd);
    if (it == open_files_.end()) {
        out_valid_bytes = 0;
        return false;
    }

    FileHandle& fh = it->second;
    LARGE_INTEGER li;
    li.QuadPart = key.block_number * BLOCK_SIZE;
    if (!SetFilePointerEx(fh.handle, li, NULL, FILE_BEGIN)) {
        out_valid_bytes = 0;
        return false;
    }

    DWORD bytes_read = 0;
    BOOL result = ReadFile(fh.handle, buffer, (DWORD)BLOCK_SIZE, &bytes_read, nullptr);

    out_valid_bytes = bytes_read;
    return (result != 0);
}

bool CacheManager::writeBlockToDisk(const BlockKey& key, const char* buffer) {
    auto it = open_files_.find(key.fd);
    if (it == open_files_.end()) {
        return false;
    }

    FileHandle& fh = it->second;
    LARGE_INTEGER li;
    li.QuadPart = key.block_number * BLOCK_SIZE;
    if (!SetFilePointerEx(fh.handle, li, NULL, FILE_BEGIN)) {
        return false;
    }

    DWORD bytes_written = 0;
    BOOL res = WriteFile(fh.handle, buffer, (DWORD)BLOCK_SIZE, &bytes_written, NULL);
    if (!res || bytes_written != BLOCK_SIZE) {
        return false;
    }

    return true;
}

void CacheManager::evictBlock() {
    if (cache_map_.empty()) return;
    auto it = std::min_element(
            cache_map_.begin(), cache_map_.end(),
            [](const std::pair<BlockKey, CacheBlock>& a, const std::pair<BlockKey, CacheBlock>& b) {
                return a.second.frequency < b.second.frequency;
            }
    );
    if (it != cache_map_.end()) {
        if (it->second.dirty) {
            if (it->second.valid_bytes < BLOCK_SIZE) {
                memset(it->second.data + it->second.valid_bytes, 0, BLOCK_SIZE - it->second.valid_bytes);
            }
            writeBlockToDisk(it->first, it->second.data);
        }
        cache_map_.erase(it);
    }
}

int CacheManager::flushFile(int fd) {
    bool success = true;
    std::vector<BlockKey> blocks;
    for (auto& p : cache_map_) {
        if (p.first.fd == fd) {
            blocks.push_back(p.first);
        }
    }

    auto f_it = open_files_.find(fd);
    if (f_it == open_files_.end()) {
        return -1;
    }
    FileHandle& fh = f_it->second;

    int64_t max_offset = -1;

    for (auto& key : blocks) {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            continue;
        }
        CacheBlock& cb = it->second;
        if (cb.dirty) {
            if (cb.valid_bytes < BLOCK_SIZE) {
                memset(cb.data + cb.valid_bytes, 0, BLOCK_SIZE - cb.valid_bytes);
            }
            if (!writeBlockToDisk(key, cb.data)) {
                success = false;
            }
            cb.dirty = false;
        }
        int64_t block_start = key.block_number * BLOCK_SIZE;
        int64_t end_of_data = block_start + cb.valid_bytes;
        if (end_of_data > max_offset) {
            max_offset = end_of_data;
        }
    }

    if (max_offset > fh.known_size) {
        fh.known_size = max_offset;
    }

    if (max_offset >= 0 && success) {
        int64_t aligned_size = ((fh.known_size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
        LARGE_INTEGER li;
        li.QuadPart = aligned_size;
        if (!SetFilePointerEx(fh.handle, li, NULL, FILE_BEGIN)) {
            success = false;
        } else {
            if (!SetEndOfFile(fh.handle)) {
                success = false;
            }
        }
    }

    for (auto& key : blocks) {
        cache_map_.erase(key);
    }

    return success ? 0 : -1;
}



// Остальные методы, такие как getCacheBlock, evictBlock, readBlockFromDisk, и flushFile
// переносятся в этот файл из lab2.cpp
