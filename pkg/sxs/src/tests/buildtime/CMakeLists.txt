add_executable(test_sxs_buildtime test_example.c)

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_dependencies(test_sxs_buildtime sxs)
    target_link_libraries(test_sxs_buildtime PRIVATE $<TARGET_FILE:sxs>)
else()
    target_link_libraries(test_sxs_buildtime PRIVATE sxs)
endif()

target_include_directories(test_sxs_buildtime PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg
)

add_test(NAME sxs_buildtime_test COMMAND test_sxs_buildtime)

add_custom_command(TARGET test_sxs_buildtime POST_BUILD
    COMMAND $<TARGET_FILE:test_sxs_buildtime>
    COMMENT "Running sxs buildtime test..."
)
