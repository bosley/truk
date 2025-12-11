add_executable(test_parser
    test_parser.cpp
)

target_link_libraries(test_parser
    PRIVATE
    GTest::gtest_main
    parser
)

include(GoogleTest)
gtest_discover_tests(test_parser)
