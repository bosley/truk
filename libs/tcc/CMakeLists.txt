add_library(truk_tcc STATIC
    src/tcc.cpp
)

target_include_directories(truk_tcc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_path(TCC_INCLUDE_DIR libtcc.h)
find_library(TCC_LIBRARY NAMES tcc libtcc)

if(NOT TCC_INCLUDE_DIR OR NOT TCC_LIBRARY)
    message(STATUS "TCC library not found locally, fetching and building from source")
    
    include(ExternalProject)
    
    ExternalProject_Add(
        tcc_external
        GIT_REPOSITORY https://repo.or.cz/tinycc.git
        GIT_TAG release_0_9_27
        GIT_SHALLOW TRUE
        PREFIX ${CMAKE_BINARY_DIR}/tcc-prefix
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 0
    )
    
    set(TCC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/tcc-prefix/include)
    set(TCC_LIBRARY ${CMAKE_BINARY_DIR}/tcc-prefix/lib/libtcc.a)
    
    add_dependencies(truk_tcc tcc_external)
endif()

target_include_directories(truk_tcc PRIVATE ${TCC_INCLUDE_DIR})
target_link_libraries(truk_tcc PRIVATE ${TCC_LIBRARY})

target_compile_options(truk_tcc PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
