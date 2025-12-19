add_library(truk_aether INTERFACE)

target_include_directories(truk_aether INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(truk_aether INTERFACE cxx_std_20)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
