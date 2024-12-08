// src/create_testfile.cpp

#include <fstream>
#include <vector>
#include <iostream>

int main() {
    const char *file_path = "testfile.dat";

    std::ofstream outfile(file_path, std::ios::binary | std::ios::out);
    if (!outfile) {
        std::cerr << "Failed to create " << file_path << std::endl;
        return 1;
    }

    // Заполняем файл символами 'A'
    std::vector<char> data(4096, 'A'); // 4096 байт
    outfile.write(data.data(), data.size());

    if (!outfile) {
        std::cerr << "Failed to write to " << file_path << std::endl;
        return 1;
    }

    outfile.close();
    std::cout << "Created " << file_path << " with size " << data.size() << " bytes." << std::endl;

    return 0;
}
