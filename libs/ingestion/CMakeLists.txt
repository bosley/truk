add_library(ingestion STATIC
    src/tokenize.cpp
    src/parser.cpp
)

add_subdirectory(tests)

target_include_directories(ingestion PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ingestion PUBLIC
    truk_language
)

target_compile_features(ingestion PRIVATE cxx_std_20)
