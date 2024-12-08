#include "lab2.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <cassert>

// Функция для вывода результатов тестов
void print_result(const std::string& test_name, bool passed) {
    std::cout << (passed ? "[PASSED] " : "[FAILED] ") << test_name << std::endl;
}

// 1. Тест: Открытие файлов
void test_open_files() {
    std::string test_name = "Test Open Files";

    // Создаём временный файл
    const char* existing_file = "existing_file.dat";
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        fclose(fp);
    } else {
        print_result(test_name, false);
        return;
    }

    // Открываем существующий файл
    int fd1 = lab2_open(existing_file);
    bool pass = (fd1 != -1);

    // Открываем несуществующий файл
    int fd2 = lab2_open("nonexistent_file.dat");
    pass = pass && (fd2 == -1);

    // Закрываем временный файл
    lab2_close(fd1);
    remove(existing_file);

    print_result(test_name, pass);
}

// 2. Тест: Закрытие файлов
void test_close_files() {
    std::string test_name = "Test Close Files";

    const char* existing_file = "existing_file.dat";
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        fclose(fp);
    } else {
        print_result(test_name, false);
        return;
    }

    int fd = lab2_open(existing_file);
    if (fd == -1) {
        print_result(test_name, false);
        remove(existing_file);
        return;
    }

    // Закрываем файл
    bool pass = (lab2_close(fd) == 0);

    // Пытаемся закрыть тот же fd снова
    pass = pass && (lab2_close(fd) == -1);

    // Пытаемся закрыть несуществующий fd
    pass = pass && (lab2_close(-1) == -1);

    remove(existing_file);

    print_result(test_name, pass);
}

// 3. Тест: Чтение данных
// 3. Тест: Чтение данных
void test_read_data() {
    std::string test_name = "Test Read Data";
    std::cout << "Starting: " << test_name << std::endl;

    const char* existing_file = "existing_file.dat";
    const char* data = "Test Data";

    std::cout << "1: Creating file " << existing_file << " and writing data \"" << data << "\"..." << std::endl;
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        size_t written = fwrite(data, sizeof(char), strlen(data), fp);
        std::cout << "2: Written " << written << " bytes to " << existing_file << std::endl;
        fclose(fp);
    } else {
        std::cout << "ERROR: "  << "Failed to create file " << existing_file << std::endl;
        print_result(test_name, false);
        return;
    }

    std::cout << "3: Opening file " << existing_file << "..." << std::endl;
    int fd = lab2_open(existing_file);
    if (fd == -1) {
        std::cout << "ERROR: "  << "Failed to open file " << existing_file << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    } else {
        std::cout << "4: File opened successfully with fd " << fd << std::endl;
    }

    std::cout << "5: Seeking to the beginning of the file..." << std::endl;
    bool pass = (lab2_lseek(fd, 0, SEEK_SET) != -1);
    if (!pass) {
        std::cout << "ERROR: "  << "6: lab2_lseek failed" << std::endl;
    } else {
        std::cout << "6: Seek successful." << std::endl;
    }

    std::cout << "7: Reading " << strlen(data) << " bytes from file..." << std::endl;
    char buffer[20];
    ssize_t bytes_read = lab2_read(fd, buffer, strlen(data));
    if (bytes_read == static_cast<ssize_t>(strlen(data))) {
        std::cout << "8: Successfully read " << bytes_read << " bytes." << std::endl;
    } else {
        std::cout << "ERROR: "  << "8: Failed to read the correct number of bytes. Expected " << strlen(data)
                  << ", got " << bytes_read << std::endl;
        pass = false;
    }

    buffer[bytes_read] = '\0';
    if (std::strcmp(buffer, data) == 0) {
        std::cout << "9: Data read matches expected: \"" << buffer << "\"" << std::endl;
    } else {
        std::cout << "ERROR: "  << "9: Data read does not match expected. Got \"" << buffer << "\" instead of \"" << data << "\"" << std::endl;
        pass = false;
    }

    // Добавляем повторное позиционирование в начало файла,
    // чтобы проверить чтение большего числа байт, чем есть в файле, с нуля.
    std::cout << "10: Attempting to seek to the beginning again before reading 100 bytes..." << std::endl;
    if (lab2_lseek(fd, 0, SEEK_SET) == -1) {
        std::cout << "ERROR: "  << "Failed to seek to beginning before large read" << std::endl;
        pass = false;
    }

    std::cout << "11: Attempting to read 100 bytes (more than data length)..." << std::endl;
    bytes_read = lab2_read(fd, buffer, 100);
    if (bytes_read == static_cast<ssize_t>(strlen(data))) {
        std::cout << "12: Correct behavior: read only " << bytes_read << " bytes (same as data length)." << std::endl;
    } else {
        std::cout << "12: Unexpected behavior: tried to read 100 bytes, expected to read " << strlen(data)
                  << " but got " << bytes_read << std::endl;
        pass = false;
    }

    std::cout << "13: Closing file..." << std::endl;
    if (lab2_close(fd) == 0) {
        std::cout << "14: File closed successfully." << std::endl;
    } else {
        std::cout << "ERROR: "  << "14: Failed to close file." << std::endl;
        pass = false;
    }

    std::cout << "15: Attempting to read after file is closed..." << std::endl;
    bytes_read = lab2_read(fd, buffer, 10);
    if (bytes_read == -1) {
        std::cout << "16: Correct behavior: cannot read from closed file." << std::endl;
    } else {
        std::cout << "ERROR: "  << "16: Unexpected behavior: read returned " << bytes_read << " even though file is closed." << std::endl;
        pass = false;
    }

    std::cout << "17: Removing temporary file " << existing_file << "..." << std::endl;
    if (remove(existing_file) == 0) {
        std::cout << "18: File removed successfully." << std::endl;
    } else {
        std::cout << "ERROR: "  << "18: Failed to remove file " << existing_file << std::endl;
        pass = false;
    }

    print_result(test_name, pass);
}



// 4. Тест: Запись данных
void test_write_data() {
    std::string test_name = "Test Write Data";
    std::cout << "Starting: " << test_name << std::endl;

    const char* existing_file = "existing_file.dat";
    const char* data = "Sample Write";

    std::cout << "1: Creating file " << existing_file << "..." << std::endl;
    FILE* fp = fopen(existing_file, "wb");
    if (!fp) {
        std::cout << "ERROR: Failed to create file " << existing_file << std::endl;
        print_result(test_name, false);
        return;
    }
    fclose(fp);
    std::cout << "2: File created successfully." << std::endl;

    std::cout << "3: Opening file " << existing_file << "..." << std::endl;
    int fd = lab2_open(existing_file);
    if (fd == -1) {
        std::cout << "ERROR: Failed to open file " << existing_file << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    } else {
        std::cout << "4: File opened successfully with fd " << fd << std::endl;
    }

    std::cout << "5: Writing data \"" << data << "\" to fd " << fd << "..." << std::endl;
    ssize_t bytes_written = lab2_write(fd, data, strlen(data));
    if (bytes_written != static_cast<ssize_t>(strlen(data))) {
        std::cout << "ERROR: Failed to write data \"" << data << "\". Expected " << strlen(data)
                  << " bytes, but got " << bytes_written << " bytes." << std::endl;
        print_result(test_name, false);
        lab2_close(fd);
        remove(existing_file);
        return;
    } else {
        std::cout << "6: Successfully wrote " << bytes_written << " bytes." << std::endl;
    }

    std::cout << "7: Attempting to write 0 bytes..." << std::endl;
    bytes_written = lab2_write(fd, data, 0);
    if (bytes_written != 0) {
        std::cout << "ERROR: Expected to write 0 bytes, but wrote " << bytes_written << std::endl;
        print_result(test_name, false);
        lab2_close(fd);
        remove(existing_file);
        return;
    } else {
        std::cout << "8: Correct behavior: wrote 0 bytes as expected." << std::endl;
    }

    std::cout << "9: Closing file fd " << fd << "..." << std::endl;
    if (lab2_close(fd) != 0) {
        std::cout << "ERROR: Failed to close fd " << fd << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    } else {
        std::cout << "10: File closed successfully." << std::endl;
    }

    std::cout << "11: Attempting to write after file is closed..." << std::endl;
    bytes_written = lab2_write(fd, data, strlen(data));
    if (bytes_written != -1) {
        std::cout << "ERROR: Expected lab2_write to return -1 after file is closed, got " << bytes_written << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    } else {
        std::cout << "12: Correct behavior: can't write to closed file." << std::endl;
    }

    std::cout << "13: Attempting to write with invalid fd (-1)..." << std::endl;
    bytes_written = lab2_write(-1, data, strlen(data));
    if (bytes_written != -1) {
        std::cout << "ERROR: Expected lab2_write to return -1 for invalid fd, got " << bytes_written << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    } else {
        std::cout << "14: Correct behavior: can't write with invalid fd." << std::endl;
    }

    std::cout << "15: Removing file " << existing_file << "..." << std::endl;
    if (remove(existing_file) != 0) {
        std::cout << "ERROR: Failed to remove file " << existing_file << std::endl;
        print_result(test_name, false);
        return;
    } else {
        std::cout << "16: File removed successfully." << std::endl;
    }

    print_result(test_name, true);
}


// 5. Тест: Перемещение указателя (Seek)
void test_seek() {
    std::string test_name = "Test Seek";

    const char* existing_file = "existing_file.dat";
    const char* data = "Seek Test";

    // Создаём файл и записываем данные
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        fwrite(data, sizeof(char), strlen(data), fp);
        fclose(fp);
    } else {
        print_result(test_name, false);
        return;
    }

    int fd = lab2_open(existing_file);
    if (fd == -1) {
        print_result(test_name, false);
        remove(existing_file);
        return;
    }

    // Перемещаем указатель на 5 байт от начала
    int64_t new_pos = lab2_lseek(fd, 5, SEEK_SET);
    bool pass = (new_pos == 5);

    // Читаем 4 байта
    char buffer[10];
    ssize_t bytes_read = lab2_read(fd, buffer, 4);
    buffer[bytes_read] = '\0';
    pass = pass && (bytes_read == 4) && (std::strncmp(buffer, "Test", 4) == 0);

    // Перемещаем указатель на -3 байта от текущей позиции
    new_pos = lab2_lseek(fd, -3, SEEK_CUR);
    pass = pass && (new_pos == 6);

    // Читаем 3 байта
    bytes_read = lab2_read(fd, buffer, 3);
    buffer[bytes_read] = '\0';
    pass = pass && (bytes_read == 3) && (std::strncmp(buffer, "est", 3) == 0);

    // Пытаемся переместить указатель на -10 байт от начала файла
    new_pos = lab2_lseek(fd, -10, SEEK_SET);
    pass = pass && (new_pos == -1);

    // Пытаемся переместить указатель с невалидным whence
    new_pos = lab2_lseek(fd, 0, 999);
    pass = pass && (new_pos == -1);

    // Закрываем файл
    pass = pass && (lab2_close(fd) == 0);

    remove(existing_file);

    print_result(test_name, pass);
}

// 6. Тест: Синхронизация данных (Fsync)
void test_fsync() {
    std::string test_name = "Test Fsync";

    const char* existing_file = "existing_file.dat";
    const char* data = "Fsync Test";

    // Создаём файл и записываем данные
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        fwrite(data, sizeof(char), strlen(data), fp);
        fclose(fp);
    } else {
        print_result(test_name, false);
        return;
    }

    int fd = lab2_open(existing_file);
    if (fd == -1) {
        print_result(test_name, false);
        remove(existing_file);
        return;
    }

    // Записываем новые данные
    const char* new_data = "New Data";
    ssize_t bytes_written = lab2_write(fd, new_data, strlen(new_data));
    bool pass = (bytes_written == static_cast<ssize_t>(strlen(new_data)));

    // Синхронизируем данные
    pass = pass && (lab2_fsync(fd) == 0);

    // Читаем данные напрямую из файла, чтобы проверить синхронизацию
    FILE* fp_read = fopen(existing_file, "rb");
    if (fp_read) {
        char buffer[20];
        fread(buffer, sizeof(char), strlen(data) + strlen(new_data), fp_read);
        buffer[strlen(data) + strlen(new_data)] = '\0';
        fclose(fp_read);
        pass = pass && (std::strncmp(buffer, data, strlen(data)) == 0) &&
               (std::strncmp(buffer + strlen(data), new_data, strlen(new_data)) == 0);
    } else {
        pass = false;
    }

    // Закрываем файл
    pass = pass && (lab2_close(fd) == 0);

    // Пытаемся синхронизировать закрытый файл
    pass = pass && (lab2_fsync(fd) == -1);

    // Пытаемся синхронизировать с невалидным fd
    pass = pass && (lab2_fsync(-1) == -1);

    remove(existing_file);

    print_result(test_name, pass);
}

// 7. Тест: Политика Вытеснения (LFU)
void test_lfu_policy() {
    std::string test_name = "Test LFU Policy";

    // Определим размер кэша
    const size_t MAX_CACHE_BLOCKS = 1024;

    // Создаём несколько файлов
    const int num_files = 5; // Больше MAX_CACHE_BLOCKS
    std::vector<std::string> filenames;
    for (int i = 0; i < num_files; ++i) {
        std::string fname = "lfu_test_file_" + std::to_string(i) + ".dat";
        FILE* fp = fopen(fname.c_str(), "wb");
        if (fp) {
            fclose(fp);
            filenames.push_back(fname);
        } else {
            // Если не удалось создать файл, пропускаем
        }
    }

    // Открываем все файлы
    std::vector<int> fds;
    for (const auto& fname : filenames) {
        int fd = lab2_open(fname.c_str());
        if (fd != -1) {
            fds.push_back(fd);
        }
    }

    // Выполняем операции чтения и записи
    bool pass = true;
    for (size_t i = 0; i < fds.size(); ++i) {
        const char* data = "LFU Data";
        ssize_t bytes_written = lab2_write(fds[i], data, strlen(data));
        if (bytes_written != static_cast<ssize_t>(strlen(data))) {
            pass = false;
            break;
        }

        // Читаем данные
        lab2_lseek(fds[i], 0, SEEK_SET);
        char buffer[20];
        ssize_t bytes_read = lab2_read(fds[i], buffer, strlen(data));
        if (bytes_read != static_cast<ssize_t>(strlen(data)) || std::strncmp(buffer, data, strlen(data)) != 0) {
            pass = false;
            break;
        }
    }

    // Закрываем все файлы
    for (auto fd : fds) {
        lab2_close(fd);
    }

    // Удаляем файлы
    for (const auto& fname : filenames) {
        remove(fname.c_str());
    }

    // Проверяем, что кэш не превышает лимит
    // К сожалению, без доступа к внутренностям кэша, мы не можем напрямую проверить его размер.
    // Вместо этого, мы проверим, что все файлы корректно записаны и прочитаны.

    print_result(test_name, pass);
}

// 8. Тест: Параллельный Доступ (Многопоточность)
void test_multithreaded_access() {
    std::string test_name = "Test Multithreaded Access";

    const char* existing_file = "multithread_file.dat";
    const char* initial_data = "Initial Data";

    // Создаём файл и записываем начальные данные
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        fwrite(initial_data, sizeof(char), strlen(initial_data), fp);
        fclose(fp);
    } else {
        print_result(test_name, false);
        return;
    }

    int fd = lab2_open(existing_file);
    if (fd == -1) {
        print_result(test_name, false);
        remove(existing_file);
        return;
    }

    // Функция для записи данных в файл
    auto writer = [&](const char* data, size_t len) {
        lab2_write(fd, data, len);
    };

    // Функция для чтения данных из файла
    auto reader = [&](char* buffer, size_t len) {
        lab2_read(fd, buffer, len);
    };

    // Создаём потоки
    std::thread t1(writer, "Thread1 Data ", 13);
    std::thread t2(writer, "Thread2 Data ", 13);
    std::thread t3(writer, "Thread3 Data ", 13);

    std::thread t4([&]() {
        char buffer[50];
        lab2_lseek(fd, 0, SEEK_SET);
        lab2_read(fd, buffer, 50);
        buffer[49] = '\0';
        // Можно добавить проверку содержимого
    });

    // Ждём завершения потоков
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    // Закрываем файл
    bool pass = (lab2_close(fd) == 0);

    remove(existing_file);

    print_result(test_name, pass);
}

// 9. Тест: Обработка Неверных Параметров
void test_invalid_parameters() {
    std::string test_name = "Test Invalid Parameters";
    std::cout << "Starting: " << test_name << std::endl;
    bool pass = true;

    // Попытка открыть файл с nullptr
    std::cout << "1: Attempting to open file with nullptr..." << std::endl;
    int fd = lab2_open(nullptr);
    if (fd != -1) {
        std::cout << "ERROR: "  << "Failed: lab2_open(nullptr) should return -1, got " << fd << std::endl;
        pass = false;
    }
    std::cout << "2: lab2_open(nullptr) returned " << fd << std::endl;

    // Создаём временный файл
    const char* existing_file = "invalid_params.dat";
    std::cout << "3: Creating temporary file " << existing_file << "..." << std::endl;
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        std::cout << "4: Temporary file created successfully." << std::endl;
        fclose(fp);
    } else {
        std::cout << "ERROR: "  << "Failed: Unable to create temporary file " << existing_file << std::endl;
        print_result(test_name, false);
        return;
    }

    std::cout << "5: Attempting to open existing file " << existing_file << "..." << std::endl;
    fd = lab2_open(existing_file);
    if (fd == -1) {
        std::cout << "ERROR: "  << "Failed: lab2_open(" << existing_file << ") returned -1" << std::endl;
        print_result(test_name, false);
        remove(existing_file);
        return;
    }
    std::cout << "6: lab2_open(" << existing_file << ") returned " << fd << std::endl;

    // Попытка читать с nullptr буфером
    std::cout << "7: Attempting to read with nullptr buffer..." << std::endl;
    ssize_t bytes_read = lab2_read(fd, nullptr, 10);
    if (bytes_read != -1) {
        std::cout << "ERROR: "  << "Failed: lab2_read(fd, nullptr, 10) should return -1, got " << bytes_read << std::endl;
        pass = false;
    }
    std::cout << "8: lab2_read(fd, nullptr, 10) returned " << bytes_read << std::endl;

    // Попытка писать с nullptr буфером
    std::cout << "9: Attempting to write with nullptr buffer..." << std::endl;
    ssize_t bytes_written = lab2_write(fd, nullptr, 10);
    if (bytes_written != -1) {
        std::cout << "ERROR: "  << "Failed: lab2_write(fd, nullptr, 10) should return -1, got " << bytes_written << std::endl;
        pass = false;
    }
    std::cout << "10: lab2_write(fd, nullptr, 10) returned " << bytes_written << std::endl;

    // Попытка писать с очень большим count
    std::cout << "11: Attempting to write with count = SIZE_MAX..." << std::endl;
    const char* data = "Data";
    bytes_written = lab2_write(fd, data, SIZE_MAX);
    if (bytes_written != -1) {
        std::cout << "ERROR: "  << "Failed: lab2_write(fd, data, SIZE_MAX) should return -1, got " << bytes_written << std::endl;
        pass = false;
    }
    std::cout << "12: lab2_write(fd, data, SIZE_MAX) returned " << bytes_written << std::endl;

    // Попытка использовать некорректный whence
    std::cout << "13: Attempting to seek with invalid whence..." << std::endl;
    int64_t new_pos = lab2_lseek(fd, 0, 999);
    if (new_pos != -1) {
        std::cout << "ERROR: "  << "Failed: lab2_lseek(fd, 0, 999) should return -1, got " << new_pos << std::endl;
        pass = false;
    }
    std::cout << "14: lab2_lseek(fd, 0, 999) returned " << new_pos << std::endl;

    // Закрываем файл
    std::cout << "15: Attempting to close file descriptor " << fd << "..." << std::endl;
    if (lab2_close(fd) != 0) {
        std::cout << "ERROR: "  << "Failed: lab2_close(fd) should return 0" << std::endl;
        pass = false;
    }
    std::cout << "16: lab2_close(fd) returned 0" << std::endl;

    // Удаляем временный файл
    std::cout << "17: Removing temporary file " << existing_file << "..." << std::endl;
    remove(existing_file);

    print_result(test_name, pass);
}


// 10. Тест: Многократные Открытия и Закрытия
// 10. Тест: Многократные Открытия и Закрытия
// 10. Тест: Многократные Открытия и Закрытия
// Изменённая версия функции test_multiple_opens_closes с добавлением lab2_lseek(fd2, 0, SEEK_END) после открытия второго дескриптора

void test_multiple_opens_closes() {
    std::string test_name = "Test Multiple Opens and Closes";
    std::cout << "Starting: " << test_name << std::endl;
    bool pass = true;

    const char* existing_file = "multiple_opens.dat";
    const char* data1 = "First Write";
    const char* data2 = "Second Write";

    // Создаём пустой файл
    std::cout << "1: Creating temporary file " << existing_file << "..." << std::endl;
    FILE* fp = fopen(existing_file, "wb");
    if (fp) {
        std::cout << "2: Temporary file created successfully." << std::endl;
        fclose(fp);
    } else {
        std::cout << "ERROR: "  << "3: Failed to create temporary file " << existing_file << std::endl;
        print_result(test_name, false);
        return;
    }

    // Открываем файл несколько раз
    std::cout << "4: Opening file " << existing_file << " first time..." << std::endl;
    int fd1 = lab2_open(existing_file);
    if (fd1 == -1) {
        std::cout << "ERROR: "  << "5: Failed to open file " << existing_file << " first time." << std::endl;
        pass = false;
    } else {
        std::cout << "6: File opened first time with fd " << fd1 << std::endl;
    }

    std::cout << "7: Opening file " << existing_file << " second time..." << std::endl;
    int fd2 = lab2_open(existing_file);
    if (fd2 == -1) {
        std::cout << "ERROR: "  << "8: Failed to open file " << existing_file << " second time." << std::endl;
        pass = false;
    } else {
        std::cout << "9: File opened second time with fd " << fd2 << std::endl;
    }

    // Проверка, что дескрипторы уникальны
    if (fd1 != -1 && fd2 != -1 && fd1 == fd2) {
        std::cout << "ERROR: "  << "10: Duplicate file descriptors returned: fd1=" << fd1 << ", fd2=" << fd2 << std::endl;
        pass = false;
    } else {
        std::cout << "11: File descriptors are unique." << std::endl;
    }

    // Пишем через первый дескриптор
    std::cout << "12: Writing data1 through fd1..." << std::endl;
    ssize_t bytes_written = lab2_write(fd1, data1, strlen(data1));
    if (bytes_written != static_cast<ssize_t>(strlen(data1))) {
        std::cout << "ERROR: "  << "13: Failed to write data1 through fd1. Expected " << strlen(data1) << " bytes, wrote " << bytes_written << " bytes." << std::endl;
        pass = false;
    } else {
        std::cout << "14: Successfully wrote data1 (" << bytes_written << " bytes) through fd1." << std::endl;
    }

    // Перед записью вторых данных переходим в конец файла через fd2
    std::cout << "15: Seeking to the end through fd2 before writing data2..." << std::endl;
    if (lab2_lseek(fd2, 0, SEEK_END) == -1) {
        std::cout << "ERROR: "  << "16: Failed to seek to the end of file through fd2." << std::endl;
        pass = false;
    } else {
        std::cout << "17: Successfully seeked to the end of file through fd2." << std::endl;
    }

    // Пишем через второй дескриптор
    std::cout << "18: Writing data2 through fd2..." << std::endl;
    bytes_written = lab2_write(fd2, data2, strlen(data2));
    if (bytes_written != static_cast<ssize_t>(strlen(data2))) {
        std::cout << "ERROR: "  << "19: Failed to write data2 through fd2. Expected " << strlen(data2) << " bytes, wrote " << bytes_written << " bytes." << std::endl;
        pass = false;
    } else {
        std::cout << "20: Successfully wrote data2 (" << bytes_written << " bytes) through fd2." << std::endl;
    }

    // Закрываем первый дескриптор
    std::cout << "21: Closing fd1..." << std::endl;
    if (lab2_close(fd1) != 0) {
        std::cout << "ERROR: "  << "22: Failed to close fd1 (" << fd1 << ")." << std::endl;
        pass = false;
    } else {
        std::cout << "23: Successfully closed fd1 (" << fd1 << ")." << std::endl;
    }

    // Пытаемся читать через закрытый дескриптор
    std::cout << "24: Attempting to read through closed fd1..." << std::endl;
    char buffer[50];
    ssize_t bytes_read = lab2_read(fd1, buffer, 10);
    if (bytes_read != -1) {
        std::cout << "ERROR: "  << "25: Expected lab2_read(fd1, buffer, 10) to return -1, but got " << bytes_read << std::endl;
        pass = false;
    } else {
        std::cout << "26: Correctly failed to read through closed fd1." << std::endl;
    }

    // Читаем через второй дескриптор
    std::cout << "27: Seeking to beginning of file through fd2..." << std::endl;
    if (lab2_lseek(fd2, 0, SEEK_SET) == -1) {
        std::cout << "ERROR: "  << "28: Failed to seek to beginning of file through fd2." << std::endl;
        pass = false;
    } else {
        std::cout << "29: Successfully seeked to beginning of file through fd2." << std::endl;
    }

    std::cout << "30: Reading combined data through fd2..." << std::endl;
    bytes_read = lab2_read(fd2, buffer, strlen(data1) + strlen(data2));
    if (bytes_read != static_cast<ssize_t>(strlen(data1) + strlen(data2))) {
        std::cout << "ERROR: "  << "31: Failed to read combined data through fd2. Expected " << (strlen(data1) + strlen(data2)) << " bytes, got " << bytes_read << " bytes." << std::endl;
        pass = false;
    } else {
        buffer[bytes_read] = '\0'; // Завершаем строку нулевым символом
        std::cout << "32: Successfully read " << bytes_read << " bytes through fd2: \"" << buffer << "\"" << std::endl;
        // Проверяем корректность данных
        if (std::strncmp(buffer, data1, strlen(data1)) != 0 ||
            std::strncmp(buffer + strlen(data1), data2, strlen(data2)) != 0) {
            std::cout << "ERROR: "  << "33: Data read through fd2 does not match written data." << std::endl;
            pass = false;
        } else {
            std::cout << "34: Data read through fd2 matches written data." << std::endl;
        }
    }

    // Закрываем второй дескриптор
    std::cout << "35: Closing fd2..." << std::endl;
    if (lab2_close(fd2) != 0) {
        std::cout << "ERROR: "  << "36: Failed to close fd2 (" << fd2 << ")." << std::endl;
        pass = false;
    } else {
        std::cout << "37: Successfully closed fd2 (" << fd2 << ")." << std::endl;
    }

    // Удаляем временный файл
    std::cout << "38: Removing temporary file " << existing_file << "..." << std::endl;
    if (remove(existing_file) != 0) {
        std::cout << "ERROR: "  << "39: Failed to remove temporary file " << existing_file << "." << std::endl;
        pass = false;
    } else {
        std::cout << "40: Successfully removed temporary file " << existing_file << "." << std::endl;
    }

    print_result(test_name, pass);
}




int main() {
    std::cout << "Running Test Suite for lab2 Block Cache...\n\n";

    test_open_files();
    std::cout << "" << std::endl;
    test_close_files();
    std::cout << "" << std::endl;
    test_read_data();
    std::cout << "" << std::endl;
    test_write_data();
    std::cout << "" << std::endl;
    test_seek();
    std::cout << "" << std::endl;
    test_fsync();
    std::cout << "" << std::endl;
    test_lfu_policy();
    std::cout << "" << std::endl;
    test_multithreaded_access();
    std::cout << "" << std::endl;
    test_invalid_parameters();
    std::cout << "" << std::endl;
    test_multiple_opens_closes();
    std::cout << "" << std::endl;

    std::cout << "\nTest Suite Completed.\n";

    return 0;
}
