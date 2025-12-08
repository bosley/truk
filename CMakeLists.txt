cmake_minimum_required(VERSION 3.15)

project(truk VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CheckGit)

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -pedantic -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -Wall -Wextra -pedantic")

if(CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

include_directories(${CMAKE_SOURCE_DIR}/pkg)

enable_testing()

add_subdirectory(pkg)
add_subdirectory(cmds)
