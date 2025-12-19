add_library(truk_language STATIC
    src/keywords.cpp
    src/node.cpp
    src/builtins.cpp
)

target_include_directories(truk_language
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(truk_language PRIVATE cxx_std_20)
