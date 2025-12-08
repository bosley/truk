add_subdirectory(buildtime)
add_subdirectory(runtime)

add_custom_target(run_example_buildtime_test
    COMMAND $<TARGET_FILE:test_example_buildtime>
    DEPENDS test_example_buildtime example
    COMMENT "Running example buildtime test..."
)