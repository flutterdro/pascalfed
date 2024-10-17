
add_library(fed_frontend)
add_library(fed::frontend ALIAS fed_frontend)

target_include_directories(fed_frontend PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(fed_frontend 
    PRIVATE 
        scanner/lex.cpp
        parser/parser-declarations.cpp
        parser/parser-statements.cpp
        parser/parser-expressions.cpp
        parser/parser.cpp
        parser/context.cpp
        diagnostics/buffer.cpp
        representations/raw-source.cpp
)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(fed_frontend PRIVATE boost::boost fmt::fmt)





