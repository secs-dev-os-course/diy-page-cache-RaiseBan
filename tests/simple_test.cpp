#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include "lab2.h"

static const size_t BLOCK_SIZE = 4096;

int main() {
    const char* test_file = "test_file.dat";
    // Создаём тестовый файл
    {
        FILE* f = fopen(test_file, "wb");
        if (!f) {
            std::cerr << "Failed to create test file." << std::endl;
            return 1;
        }
        fclose(f);
    }

    // Открываем файл
    int fd = lab2_open(test_file);
    if (fd == -1) {
        std::cerr << "Failed to open test file via lab2_open." << std::endl;
        remove(test_file);
        return 1;
    }

    // Запись меньше блока
    const char* small_data = "Hello";
    ssize_t written = lab2_write(fd, small_data, strlen(small_data));
    if (written != (ssize_t)strlen(small_data)) {
        std::cerr << "Failed to write small_data. Written: " << written << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    // Возвращаемся в начало файла
    off_t pos = lab2_lseek(fd, 0, SEEK_SET);
    if (pos != 0) {
        std::cerr << "Failed to seek to start." << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    // Читаем и проверяем
    char buffer[100];
    memset(buffer, 0, sizeof(buffer));
    ssize_t read_bytes = lab2_read(fd, buffer, 100);
    if (read_bytes != (ssize_t)strlen(small_data)) {
        std::cerr << "Unexpected read size. Expected: " << strlen(small_data) << " got: " << read_bytes << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }
    if (strcmp(buffer, small_data) != 0) {
        std::cerr << "Data mismatch after read. Got: " << buffer << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }
    std::cout << "Small write/read test passed." << std::endl;

    // Проверка записи 0 байт
    ssize_t zero_write = lab2_write(fd, small_data, 0);
    if (zero_write != 0) {
        std::cerr << "Zero-byte write should return 0, got " << zero_write << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }
    std::cout << "Zero-byte write test passed." << std::endl;

    // Проверка записи больше одного блока
    // Запишем данные размером 2*BLOCK_SIZE + 100 байт
    std::string big_data(2 * BLOCK_SIZE + 100, 'A');
    // Переместимся в конец для записи (сейчас длина файла = 5)
    pos = lab2_lseek(fd, 0, SEEK_END);
    if (pos == -1) {
        std::cerr << "Failed to seek to end of file." << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    written = lab2_write(fd, big_data.data(), big_data.size());
    if (written != (ssize_t)big_data.size()) {
        std::cerr << "Failed to write big_data. Expected: " << big_data.size() << ", got: " << written << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    // Проверим запись: считаем всё с начала
    pos = lab2_lseek(fd, 0, SEEK_SET);
    if (pos != 0) {
        std::cerr << "Failed to seek to start before reading big data." << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    // Размер файла теперь 5 (первые данные) + (2*BLOCK_SIZE + 100) байт
    size_t total_size = strlen(small_data) + big_data.size();
    std::string read_buf(total_size, '\0');
    read_bytes = lab2_read(fd, &read_buf[0], total_size);
    if (read_bytes != (ssize_t)total_size) {
        std::cerr << "Failed to read all data back. Expected " << total_size << " got: " << read_bytes << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }

    // Проверяем, что первые 5 байт - small_data, а остальные - 'A'
    if (memcmp(read_buf.data(), small_data, strlen(small_data)) != 0) {
        std::cerr << "Data mismatch in first small_data part." << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }
    for (size_t i = strlen(small_data); i < total_size; i++) {
        if (read_buf[i] != 'A') {
            std::cerr << "Data mismatch in big_data part at index " << i << std::endl;
            lab2_close(fd);
            remove(test_file);
            return 1;
        }
    }
    std::cout << "Large write/read test passed." << std::endl;

    // Проверим fsync
    int sync_res = lab2_fsync(fd);
    if (sync_res != 0) {
        std::cerr << "fsync failed." << std::endl;
        lab2_close(fd);
        remove(test_file);
        return 1;
    }
    std::cout << "fsync test passed." << std::endl;

    // Закроем файл
    if (lab2_close(fd) != 0) {
        std::cerr << "Failed to close file." << std::endl;
        remove(test_file);
        return 1;
    }

    // Попытка чтения после закрытия должна вернуть -1
    read_bytes = lab2_read(fd, buffer, 10);
    if (read_bytes != -1) {
        std::cerr << "Reading after close should fail." << std::endl;
        remove(test_file);
        return 1;
    }
    std::cout << "Read after close test passed." << std::endl;

    // Удаляем временный файл
    if (remove(test_file) != 0) {
        std::cerr << "Failed to remove test file." << std::endl;
        return 1;
    }

    std::cout << "All tests passed successfully." << std::endl;
    return 0;
}
