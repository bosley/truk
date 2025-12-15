include(EmbedRuntime)

set(EMBEDDED_RUNTIME_FILE "${CMAKE_BINARY_DIR}/generated/embedded_runtime.hpp")
embed_sxs_runtime("${EMBEDDED_RUNTIME_FILE}")

add_library(truk_emitc STATIC src/emitter.cpp)

add_dependencies(truk_emitc sxs)

target_include_directories(truk_emitc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(truk_emitc PUBLIC
    truk_core
    truk_language
    truk_ingestion
)

target_compile_features(truk_emitc PRIVATE cxx_std_20)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
