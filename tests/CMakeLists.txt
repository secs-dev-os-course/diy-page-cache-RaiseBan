# tests/CMakeLists.txt

# Указываем путь к заголовочным файлам
include_directories(${PROJECT_SOURCE_DIR}/include)

# Создаем исполняемый файл для тестовой программы
add_executable(test_program test_program.cpp)

# Линкуем тестовую программу с библиотекой lab2cache
target_link_libraries(test_program lab2cache)
