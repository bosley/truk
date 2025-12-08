add_executable(test_example_buildtime test_example.c)

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_dependencies(test_example_buildtime example)
    target_link_libraries(test_example_buildtime PRIVATE $<TARGET_FILE:example>)
else()
    target_link_libraries(test_example_buildtime PRIVATE example)
endif()

target_include_directories(test_example_buildtime PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg
)

target_link_libraries(test_example_buildtime PRIVATE testlib)

add_test(NAME example_buildtime_test COMMAND test_example_buildtime)
