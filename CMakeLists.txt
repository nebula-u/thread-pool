cmake_minimum_required(VERSION 3.5.0)
# 指定C++标准为C++14
set(CMAKE_CXX_STANDARD 17)
# 确保使用指定的C++标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(thread-pool)

include_directories(./include)
aux_source_directory(./src SRCS_DIR)
aux_source_directory(./test TEST_SRCS_DIR)

add_executable(pool ${SRCS_DIR} ${TEST_SRCS_DIR})

# 添加pthread库
find_package(Threads REQUIRED)
target_link_libraries(pool Threads::Threads)
