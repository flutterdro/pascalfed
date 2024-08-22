
cmake_minimum_required(VERSION 3.28)
project(fgba VERSION 0.1.0.0)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

option(FED_BUILD_TESTS "Build tests" ON)

if (FED_BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(lib)

add_executable(exec dummy.cpp)

target_sources(exec PUBLIC dummy2.cpp)
