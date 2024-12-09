cmake_minimum_required(VERSION 3.10)
project(lab2_block_cache)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)

# Включаем экспорт символов для Windows DLL
if (WIN32)
    add_definitions(-DLAB2_EXPORTS)
endif()

# Указываем исходники для библиотеки
set(LAB2_SOURCES
        src/lab2.cpp
        src/CacheManager.cpp
        src/CacheBlock.cpp
        src/FileHandle.cpp
)

# Создаём динамическую библиотеку
add_library(lab2 SHARED ${LAB2_SOURCES})

# Указываем заголовочные файлы
target_include_directories(lab2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Тестовые программы
add_executable(simple_test tests/simple_test.cpp)
target_link_libraries(simple_test PRIVATE lab2)


add_executable(multithread_test tests/multithread_test.cpp)
target_link_libraries(multithread_test PRIVATE lab2)

add_executable(substring_modified_test tests/substring_modified_test.cpp)
target_link_libraries(substring_modified_test PRIVATE lab2)

# Копирование DLL в папку с тестами (только для Windows)
if (WIN32)
    foreach(target simple_test multithread_test substring_modified_test)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2> $<TARGET_FILE_DIR:${target}>
        )
    endforeach()
endif()
