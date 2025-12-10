truk_add_library(
    NAME truk_core
    SOURCES
        src/core.cpp
        src/environment.cpp
    HEADERS
        include/truk/core/core.hpp
        include/truk/core/memory.hpp
        include/truk/core/environment.hpp
        include/truk/core/resource.hpp
    DEPENDENCIES
        fmt::fmt
)

target_include_directories(truk_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
