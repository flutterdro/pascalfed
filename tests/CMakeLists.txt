add_executable(frontend_tests)

find_package(Catch2 REQUIRED)


target_link_libraries(frontend_tests
    PRIVATE
        Catch2::Catch2WithMain
        fed::frontend 
)

target_sources(frontend_tests
    PRIVATE
        frontend/lexing.cpp
)
