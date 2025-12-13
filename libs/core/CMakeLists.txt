truk_add_library(
    NAME truk_core
    SOURCES
        src/core.cpp
        src/environment.cpp
        src/rll.cpp
    HEADERS
        include/truk/core/core.hpp
        include/truk/core/memory.hpp
        include/truk/core/environment.hpp
        include/truk/core/resource.hpp
        include/truk/core/exceptions.hpp
        include/truk/core/host.hpp
        include/truk/core/rll.hpp
    DEPENDENCIES
        fmt::fmt
)

target_include_directories(truk_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(UNIX AND NOT APPLE)
    target_link_libraries(truk_core PRIVATE ${CMAKE_DL_LIBS})
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
