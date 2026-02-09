cmake_minimum_required(VERSION 3.20)

project(memory_pool)

set(CMAKE_CXX_STANDARD 17)

add_library(memory_pool STATIC src/PoolAllocator.cpp)

target_include_directories(memory_pool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
