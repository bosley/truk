execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH_LONG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

if(NOT GIT_HASH_LONG)
    set(GIT_HASH_LONG "unknown")
endif()

if(NOT GIT_VERSION)
    set(GIT_VERSION "unknown")
endif()

if(NOT GIT_BRANCH)
    set(GIT_BRANCH "unknown")
endif()

message(STATUS "Git Hash: ${GIT_HASH}")
message(STATUS "Git Version: ${GIT_VERSION}")
message(STATUS "Git Branch: ${GIT_BRANCH}")

file(WRITE ${PROJECT_BINARY_DIR}/git-state.txt "${GIT_HASH}\n${GIT_HASH_LONG}\n${GIT_VERSION}\n${GIT_BRANCH}")

add_compile_definitions(
    TRUK_GIT_HASH="${GIT_HASH}"
    TRUK_GIT_HASH_LONG="${GIT_HASH_LONG}"
    TRUK_GIT_VERSION="${GIT_VERSION}"
    TRUK_GIT_BRANCH="${GIT_BRANCH}"
)
