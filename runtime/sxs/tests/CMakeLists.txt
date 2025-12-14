add_executable(test_sxs_runtime test_runtime.cpp)

if(TARGET CppUTest)
  target_link_libraries(test_sxs_runtime PRIVATE sxs CppUTest CppUTestExt)
else()
  target_link_libraries(test_sxs_runtime PRIVATE sxs CppUTest::CppUTest
                                                 CppUTest::CppUTestExt)
endif()

target_compile_options(
  test_sxs_runtime PRIVATE -Wall -Wextra -Wpedantic
                           $<$<CONFIG:Debug>:-fsanitize=address>)

target_link_options(test_sxs_runtime PRIVATE
                    $<$<CONFIG:Debug>:-fsanitize=address>)

add_test(NAME sxs_runtime COMMAND test_sxs_runtime -v)

set_property(GLOBAL APPEND PROPERTY SXS_TEST_TARGETS test_sxs_runtime)
