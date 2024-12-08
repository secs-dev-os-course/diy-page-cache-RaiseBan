// tests/test_program.cpp

#include "lab2cache.h"
#include "block_cache.h" // Для доступа к BLOCK_SIZE
#include <iostream>
#include <cstring>

int main() {
    const char *file_path = "testfile.dat";

    // Открываем файл
    int fd = lab2_open(file_path);
    if (fd == -1) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return 1;
    }
    std::cout << "File opened with fd: " << fd << std::endl;

    // Читаем данные
    std::cout << "Attempting to read from file..." << std::endl;
    char buffer[BLOCK_SIZE];
    ssize_t bytes_read = lab2_read(fd, buffer, sizeof(buffer));
    std::cout << "lab2_read returned: " << bytes_read << std::endl;
    if (bytes_read == -1) {
        std::cerr << "Failed to read from file." << std::endl;
        lab2_close(fd);
        return 1;
    }
    std::cout << "Read " << bytes_read << " bytes from file." << std::endl;

    // Выводим прочитанные данные
    std::cout << "Data: " << std::string(buffer, bytes_read) << std::endl;

    // Записываем данные
    const char *write_data = "Additional data written to file.";
    std::cout << "Attempting to write to file..." << std::endl;
    ssize_t bytes_written = lab2_write(fd, write_data, strlen(write_data));
    std::cout << "lab2_write returned: " << bytes_written << std::endl;
    if (bytes_written == -1) {
        std::cerr << "Failed to write to file." << std::endl;
        lab2_close(fd);
        return 1;
    }
    std::cout << "Wrote " << bytes_written << " bytes to file." << std::endl;

    // Перемещаем указатель файла в конец
    std::cout << "Attempting to seek to end of file..." << std::endl;
    off_t new_pos = lab2_lseek(fd, 0, SEEK_END);
    std::cout << "lab2_lseek returned: " << new_pos << std::endl;
    if (new_pos == -1) {
        std::cerr << "Failed to seek to end of file." << std::endl;
        lab2_close(fd);
        return 1;
    }
    std::cout << "Seeked to position: " << new_pos << std::endl;

    // Записываем дополнительные данные в конец файла
    const char *additional_data = "More data at the end.";
    std::cout << "Attempting to write additional data to file..." << std::endl;
    bytes_written = lab2_write(fd, additional_data, strlen(additional_data));
    std::cout << "lab2_write returned: " << bytes_written << std::endl;
    if (bytes_written == -1) {
        std::cerr << "Failed to write additional data to file." << std::endl;
        lab2_close(fd);
        return 1;
    }
    std::cout << "Wrote " << bytes_written << " bytes to file." << std::endl;

    // Синхронизируем данные
    std::cout << "Attempting to synchronize file..." << std::endl;
    if (lab2_fsync(fd) != 0) {
        std::cerr << "Failed to fsync file." << std::endl;
        lab2_close(fd);
        return 1;
    }
    std::cout << "File synchronized successfully." << std::endl;

    // Закрываем файл
    std::cout << "Attempting to close file..." << std::endl;
    if (lab2_close(fd) != 0) {
        std::cerr << "Failed to close file with fd: " << fd << std::endl;
        return 1;
    }
    std::cout << "File closed successfully." << std::endl;

    return 0;
}
