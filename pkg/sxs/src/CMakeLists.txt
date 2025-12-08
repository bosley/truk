set(SXS_IMPL_SRC sxs.c)

add_library(sxs STATIC
    ${SXS_IMPL_SRC}
)

target_include_directories(sxs PUBLIC
    ${CMAKE_SOURCE_DIR}/pkg
)

add_subdirectory(tests)

add_custom_command(TARGET sxs POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_sxs_buildtime_test
    COMMENT "Running sxs buildtime test..."
)
