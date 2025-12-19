find_package(Git QUIET)

set(TRUK_BUILD_HASH "unknown")

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    if(GIT_HASH)
        set(TRUK_BUILD_HASH ${GIT_HASH})
    endif()
endif()

message(STATUS "Build hash: ${TRUK_BUILD_HASH}")

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/build_info.hpp.in
    ${CMAKE_BINARY_DIR}/generated/truk/build_info.hpp
    @ONLY
)

include_directories(${CMAKE_BINARY_DIR}/generated)
