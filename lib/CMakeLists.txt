
add_library(frontend)

target_include_directories(frontend PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_sources(frontend 
    PRIVATE 
        scanner/lex.cpp
        parser/parser.cpp
)





