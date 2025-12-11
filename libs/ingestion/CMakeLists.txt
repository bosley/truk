add_library(parser STATIC
    src/parser.cpp
)

add_subdirectory(tests)


target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(parser PUBLIC
    core
)

target_compile_features(parser PRIVATE cxx_std_20)
