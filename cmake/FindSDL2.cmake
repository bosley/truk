find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SDL2 QUIET sdl2)
endif()

find_path(SDL2_INCLUDE_DIR SDL.h
    HINTS
        ${PC_SDL2_INCLUDE_DIRS}
        ENV SDL2DIR
        ${SDL2_DIR}
    PATH_SUFFIXES SDL2 include/SDL2 include
)

find_library(SDL2_LIBRARY
    NAMES SDL2 SDL2-2.0
    HINTS
        ${PC_SDL2_LIBRARY_DIRS}
        ENV SDL2DIR
        ${SDL2_DIR}
    PATH_SUFFIXES lib
)

if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
    set(SDL2_FOUND TRUE)
endif()

if(SDL2_FOUND)
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)
