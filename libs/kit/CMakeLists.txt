truk_add_library(
    NAME truk_kit
    SOURCES
        src/kit.cpp
        src/parser.cpp
        src/resolver.cpp
        src/utils.cpp
    HEADERS
        include/truk/kit/kit.hpp
    DEPENDENCIES
        fmt::fmt
)

target_include_directories(truk_kit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(UNIX AND NOT APPLE)
    target_link_libraries(truk_kit PRIVATE ${CMAKE_DL_LIBS})
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
