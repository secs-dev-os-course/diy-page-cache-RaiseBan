cmake_minimum_required(VERSION 3.10)
project(Lab2Cache)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем, что мы собираем DLL
add_definitions(-DLAB2CACHE_EXPORTS)

# Указываем путь к заголовочным файлам
include_directories(${PROJECT_SOURCE_DIR}/include)

# Создаем динамическую библиотеку
add_library(lab2cache SHARED
        src/lab2cache.cpp
        src/block_cache.cpp
        src/platform_win.cpp
)

# Указываем, что библиотека должна экспортировать все символы
set_target_properties(lab2cache PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# После сборки lab2cache, копируем DLL в директорию tests
add_custom_command(TARGET lab2cache POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2cache> ${CMAKE_BINARY_DIR}/tests/
)

# Добавляем директорию tests
add_subdirectory(tests)

# Добавляем исполняемый файл для создания тестового файла
add_executable(create_testfile create_testfile.cpp)
