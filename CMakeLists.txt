cmake_minimum_required(VERSION 3.10)
project(lab2_block_cache)

set(CMAKE_CXX_STANDARD 17)

# Создаём динамическую библиотеку
add_library(lab2 SHARED src/lab2.cpp)

# Указываем, что заголовочные файлы должны быть доступны для других целей
target_include_directories(lab2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Добавляем тестовую программу simple_test
add_executable(simple_test tests/simple_test.cpp)

# Линкуем simple_test с библиотекой lab2
target_link_libraries(simple_test PRIVATE lab2)

# Добавляем тестовый набор test_suite
add_executable(test_suite tests/test_suite.cpp)

# Линкуем test_suite с библиотекой lab2
target_link_libraries(test_suite PRIVATE lab2)

# Добавляем тестовый набор multithread_test
add_executable(multithread_test tests/multithread_test.cpp)

# Линкуем multithread_test с библиотекой lab2
target_link_libraries(multithread_test PRIVATE lab2)


# Добавляем тестовый набор substring_modified_test
add_executable(substring_modified_test tests/substring_modified_test.cpp)

# Линкуем substring_modified_test с библиотекой lab2
target_link_libraries(substring_modified_test PRIVATE lab2)




# Копируем DLL в директорию тестовых программ для Windows
if (WIN32)
    add_custom_command(TARGET simple_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2> $<TARGET_FILE_DIR:simple_test>
    )
    add_custom_command(TARGET test_suite POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2> $<TARGET_FILE_DIR:test_suite>
    )
    add_custom_command(TARGET multithread_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2> $<TARGET_FILE_DIR:multithread_test>
    )
    add_custom_command(TARGET substring_modified_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lab2> $<TARGET_FILE_DIR:substring_modified_test>
    )
endif()
