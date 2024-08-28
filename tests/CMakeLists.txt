add_executable(frontend_tests)

find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(frontend_tests
    PRIVATE
        fmt::fmt
        Catch2::Catch2WithMain
        fed::frontend 
)

target_sources(frontend_tests
    PRIVATE
        frontend/lexing.cpp
)

include(CTest)
include(Catch)
catch_discover_tests(frontend_tests)
