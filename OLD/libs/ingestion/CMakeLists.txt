add_library(truk_ingestion STATIC
    src/tokenize.cpp
    src/parser.cpp
    src/file_utils.cpp
    src/import_resolver.cpp
)

target_include_directories(truk_ingestion PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(truk_ingestion PUBLIC
    truk_language
)

add_dependencies(truk_ingestion truk_language)

target_compile_features(truk_ingestion PRIVATE cxx_std_20)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
