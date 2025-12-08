add_executable(test_truk_buildtime test_truk.c)

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_dependencies(test_truk_buildtime example)
    target_link_libraries(test_truk_buildtime PRIVATE $<TARGET_FILE:example>)
else()
    target_link_libraries(test_truk_buildtime PRIVATE example)
endif()

target_include_directories(test_truk_buildtime PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg
)

add_test(NAME truk_buildtime_test COMMAND test_truk_buildtime)
