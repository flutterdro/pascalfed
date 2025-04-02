
add_library(fed_representations)
add_library(fed::representations ALIAS fed_representations)

target_include_directories(fed_representations PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(fed_representations PUBLIC fmt::fmt cpptrace::cpptrace)

target_sources(fed_representations
    PRIVATE
        representations/ast/pretty-print.cpp
)



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
        representations/symbol-table.cpp
)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(cpptrace REQUIRED)

target_link_libraries(fed_frontend 
    PUBLIC 
        boost::boost 
        fmt::fmt 
        cpptrace::cpptrace
        fed::representations
)





