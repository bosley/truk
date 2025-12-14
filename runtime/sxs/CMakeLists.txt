cmake_minimum_required(VERSION 3.20)

add_library(sxs STATIC src/runtime.c)

target_include_directories(sxs PUBLIC include)

target_compile_options(sxs PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
