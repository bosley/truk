add_library(truk_tcc STATIC
    src/tcc.cpp
)

target_include_directories(truk_tcc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_path(TCC_INCLUDE_DIR libtcc.h
    PATHS /usr/local/include /opt/homebrew/include
)

find_library(TCC_LIBRARY NAMES tcc libtcc
    PATHS /usr/local/lib /opt/homebrew/lib
)

if(NOT TCC_INCLUDE_DIR OR NOT TCC_LIBRARY)
    message(FATAL_ERROR "TCC library not found. Please install TCC.")
endif()

target_include_directories(truk_tcc PRIVATE ${TCC_INCLUDE_DIR})
target_link_libraries(truk_tcc PRIVATE ${TCC_LIBRARY})

target_compile_options(truk_tcc PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
