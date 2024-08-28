
cmake_minimum_required(VERSION 3.28)
project(pascalfed VERSION 0.1.0.0)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

option(FED_BUILD_TESTS "Build tests" ON)

add_subdirectory(lib)

add_executable(exec src/main.cpp)

find_package(fmt REQUIRED)

target_link_libraries(exec PRIVATE fmt::fmt)

# if (FED_BUILD_TESTS)
    add_subdirectory(tests)
# endif()
