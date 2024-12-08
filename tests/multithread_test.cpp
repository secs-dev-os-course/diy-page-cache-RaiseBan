#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include "lab2.h"

static const size_t BLOCK_SIZE = 4096;
static const char* test_file = "multithread_test_file.dat";

void writer_thread(int fd, const char* data, size_t size) {
    ssize_t written = lab2_write(fd, data, size);
    if (written != (ssize_t)size) {
        std::cerr << "Writer: Failed to write expected size. Wrote: " << written << std::endl;
    }
}

void reader_thread(int fd, size_t size) {
    std::vector<char> buf(size);
    off_t pos = lab2_lseek(fd, 0, SEEK_SET);
    if (pos != 0) {
        std::cerr << "Reader: Failed to seek to start." << std::endl;
        return;
    }
    ssize_t read_bytes = lab2_read(fd, buf.data(), size);
    std::cout << "Reader: read " << read_bytes << " bytes." << std::endl;
}

void seeker_thread(int fd, int64_t offset) {
    off_t pos = lab2_lseek(fd, offset, SEEK_SET);
    std::cout << "Seeker: new pos=" << pos << std::endl;
}

int main() {
    // Создаем пустой файл
    {
        FILE* f = fopen(test_file, "wb");
        if (!f) {
            std::cerr << "Failed to create file." << std::endl;
            return 1;
        }
        fclose(f);
    }

    int fd = lab2_open(test_file);
    if (fd == -1) {
        std::cerr << "Failed to open file." << std::endl;
        remove(test_file);
        return 1;
    }

    // Подготовим тестовые данные
    std::string data1(BLOCK_SIZE * 2 + 100, 'A'); // Запишем много 'A'
    std::string data2("HelloMultithread"); // Еще данные

    // Запустим потоки: один пишет data1, другой потом читает, третий иногда двигает указатель
    std::thread t1(writer_thread, fd, data1.data(), data1.size());
    std::thread t2(seeker_thread, fd, 5);  // Сместим позицию на 5 от начала после записи
    std::thread t3(writer_thread, fd, data2.data(), data2.size());
    std::thread t4(seeker_thread, fd, 0);  // Вернемся в начало
    std::thread t5(reader_thread, fd, data1.size() + data2.size());

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    // Синхронизируем и закрываем
    if (lab2_fsync(fd) != 0) {
        std::cerr << "fsync failed." << std::endl;
    }

    if (lab2_close(fd) != 0) {
        std::cerr << "close failed." << std::endl;
    }

    remove(test_file);

    std::cout << "Multithread test completed." << std::endl;
    return 0;
}
