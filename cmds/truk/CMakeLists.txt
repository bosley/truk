find_package(SDL2 REQUIRED)

add_executable(truk main.c)

if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
    add_dependencies(truk sxs screen)
    target_link_libraries(truk PRIVATE $<TARGET_FILE:sxs> $<TARGET_FILE:screen>)
else()
    target_link_libraries(truk PRIVATE sxs screen buffer SDL2::SDL2)
endif()

target_include_directories(truk PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg
)

add_subdirectory(tests)