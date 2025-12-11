add_library(truk_emitc STATIC
    src/emitter.cpp
)

target_include_directories(truk_emitc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(truk_emitc PUBLIC
    truk_core
    truk_language
    truk_ingestion
)

target_compile_features(truk_emitc PRIVATE cxx_std_20)

add_subdirectory(tests)
