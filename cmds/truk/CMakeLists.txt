add_executable(truk main.c)

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_dependencies(truk sxs)
    target_link_libraries(truk PRIVATE $<TARGET_FILE:sxs>)
else()
    target_link_libraries(truk PRIVATE sxs)
endif()

target_include_directories(truk PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg
)

add_subdirectory(tests)