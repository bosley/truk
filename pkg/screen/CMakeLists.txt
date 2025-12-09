find_package(SDL2 REQUIRED)

add_library(screen SHARED screen.c)

target_link_libraries(screen PRIVATE SDL2::SDL2)

target_include_directories(screen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS screen
    LIBRARY DESTINATION libs/screen/lib
    ARCHIVE DESTINATION libs/screen/lib
)

install(FILES screen.h
    DESTINATION libs/screen/include
)

add_subdirectory(tests)
