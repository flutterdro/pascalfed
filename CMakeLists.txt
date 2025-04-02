
cmake_minimum_required(VERSION 3.28)
project(pascalfed VERSION 0.1.0.0)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin"
        AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64"
        AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"
        AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18)
    link_libraries(/opt/homebrew/opt/llvm/lib/unwind/libunwind.dylib)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/llvm/lib/c++  -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++")
endif()

option(FED_BUILD_TESTS "Build tests" ON)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_subdirectory(lib)

add_executable(exec src/main.cpp)

find_package(fmt REQUIRED)

target_link_libraries(exec PRIVATE fmt::fmt)

# if (FED_BUILD_TESTS)
    add_subdirectory(tests)
# endif()
