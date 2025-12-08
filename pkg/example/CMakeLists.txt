if(WIN32)
    set(EXAMPLE_IMPL_SRC src/win/impl.c)
else()
    set(EXAMPLE_IMPL_SRC src/nix/impl.c)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_library(example SHARED
        ${EXAMPLE_IMPL_SRC}
    )
else()
    add_library(example STATIC
        ${EXAMPLE_IMPL_SRC}
    )
endif()

target_include_directories(example PUBLIC
    ${CMAKE_SOURCE_DIR}/pkg
)

add_subdirectory(tests)

add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_example_buildtime_test
    COMMENT "Running example buildtime test..."
)
