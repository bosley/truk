add_executable(test_sxs_runtime test_runtime.cpp)
add_executable(test_sxs_map test_map.cpp)

if(TARGET CppUTest)
  target_link_libraries(test_sxs_runtime PRIVATE sxs CppUTest CppUTestExt)
  target_link_libraries(test_sxs_map PRIVATE sxs CppUTest CppUTestExt)
else()
  target_link_libraries(test_sxs_runtime PRIVATE sxs CppUTest::CppUTest
                                                 CppUTest::CppUTestExt)
  target_link_libraries(test_sxs_map PRIVATE sxs CppUTest::CppUTest
                                             CppUTest::CppUTestExt)
endif()

target_compile_options(
  test_sxs_runtime PRIVATE -Wall -Wextra -Wpedantic
                           $<$<CONFIG:Debug>:-fsanitize=address>)
target_compile_options(
  test_sxs_map PRIVATE -Wall -Wextra -Wpedantic
                       $<$<CONFIG:Debug>:-fsanitize=address>)

target_link_options(test_sxs_runtime PRIVATE
                    $<$<CONFIG:Debug>:-fsanitize=address>)
target_link_options(test_sxs_map PRIVATE
                    $<$<CONFIG:Debug>:-fsanitize=address>)

add_test(NAME sxs_runtime COMMAND test_sxs_runtime -v)
add_test(NAME sxs_map COMMAND test_sxs_map -v)

set_property(GLOBAL APPEND PROPERTY SXS_TEST_TARGETS test_sxs_runtime test_sxs_map)
