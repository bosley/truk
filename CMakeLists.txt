cmake_minimum_required(VERSION 3.20)
project(truk VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)
option(BUILD_APPS "Build applications" ON)
option(BUILD_EXPERIMENTS "Build experiments" OFF)

include(GetGitHash)

if(BUILD_TESTS)
    enable_testing()
endif()

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

find_package(fmt QUIET)
if(NOT fmt_FOUND)
    message(STATUS "fmt not found locally, fetching from GitHub")
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
        GIT_SHALLOW TRUE
    )
    set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(fmt)
else()
    message(STATUS "Using system fmt - note: for standalone binary, build from source")
endif()

include(TrukHelpers)

if(BUILD_TESTS)
    find_package(CppUTest QUIET)
    if(NOT CppUTest_FOUND)
        message(STATUS "CppUTest not found locally, fetching from GitHub")
        FetchContent_Declare(
            CppUTest
            GIT_REPOSITORY https://github.com/cpputest/cpputest.git
            GIT_TAG v4.0
            GIT_SHALLOW TRUE
        )
        set(TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(CppUTest)
    endif()
endif()

add_subdirectory(runtime/sxs)

add_subdirectory(libs)

if(BUILD_APPS)
    add_subdirectory(apps)
endif()

if(BUILD_EXPERIMENTS)
    add_subdirectory(experiments)
endif()

if(BUILD_TESTS)
    get_property(SXS_TEST_TARGETS GLOBAL PROPERTY SXS_TEST_TARGETS)
    get_property(ALL_TEST_TARGETS GLOBAL PROPERTY TRUK_TEST_TARGETS)
    
    add_custom_target(run_sxs_tests
        COMMAND ${CMAKE_COMMAND} -E env --unset=MAKEFLAGS ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^sxs_"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running sxs runtime tests..."
        DEPENDS ${SXS_TEST_TARGETS}
        USES_TERMINAL
    )
    
    add_custom_target(run_tests ALL
        COMMAND ${CMAKE_COMMAND} -E env --unset=MAKEFLAGS ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all tests..."
        DEPENDS ${SXS_TEST_TARGETS} ${ALL_TEST_TARGETS} run_sxs_tests
        USES_TERMINAL
    )
endif()
