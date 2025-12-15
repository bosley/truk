include(EmbedRuntime)

set(EMBEDDED_RUNTIME_FILE "${CMAKE_BINARY_DIR}/generated/embedded_runtime.hpp")

get_sxs_runtime_files(SXS_RUNTIME_RELATIVE)
set(SXS_RUNTIME_SOURCES "")
foreach(rel_file ${SXS_RUNTIME_RELATIVE})
    list(APPEND SXS_RUNTIME_SOURCES "${CMAKE_SOURCE_DIR}/runtime/sxs/${rel_file}")
endforeach()

add_custom_command(
    OUTPUT "${EMBEDDED_RUNTIME_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
    COMMAND ${CMAKE_COMMAND}
        -DSXS_ROOT=${CMAKE_SOURCE_DIR}/runtime/sxs
        -DOUTPUT_FILE=${EMBEDDED_RUNTIME_FILE}
        -P ${CMAKE_SOURCE_DIR}/cmake/GenerateEmbeddedRuntime.cmake
    DEPENDS ${SXS_RUNTIME_SOURCES}
    COMMENT "Generating embedded runtime header"
    VERBATIM
)

set_source_files_properties("${EMBEDDED_RUNTIME_FILE}" PROPERTIES GENERATED TRUE)

add_library(truk_emitc STATIC src/emitter.cpp)

set_source_files_properties(src/emitter.cpp PROPERTIES 
    OBJECT_DEPENDS "${EMBEDDED_RUNTIME_FILE}"
)

add_dependencies(truk_emitc sxs truk_language truk_ingestion)

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
