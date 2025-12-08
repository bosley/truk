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

target_link_libraries(test_sxs_buildtime PRIVATE testlib)

add_test(NAME sxs_buildtime_test COMMAND test_sxs_buildtime)
