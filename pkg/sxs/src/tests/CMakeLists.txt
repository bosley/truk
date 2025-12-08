add_subdirectory(buildtime)
add_subdirectory(runtime)

add_custom_target(run_sxs_buildtime_test
    COMMAND $<TARGET_FILE:test_sxs_buildtime>
    DEPENDS test_sxs_buildtime sxs
    COMMENT "Running sxs buildtime test..."
)
