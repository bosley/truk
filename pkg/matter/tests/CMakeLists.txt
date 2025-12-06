add_executable(matter_test test_matter.cpp)

target_link_libraries(matter_test PRIVATE matter)

add_test(NAME matter_test COMMAND matter_test)

set_tests_properties(matter_test PROPERTIES
    TIMEOUT 30
)

