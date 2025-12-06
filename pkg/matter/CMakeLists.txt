set(MATTER_SOURCES
    matter.cpp
)

set(MATTER_HEADERS
    matter.hpp
)

if(BUILD_SHARED_LIBS)
    add_library(matter SHARED ${MATTER_SOURCES} ${MATTER_HEADERS})
else()
    add_library(matter STATIC ${MATTER_SOURCES} ${MATTER_HEADERS})
endif()

target_include_directories(matter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(matter PUBLIC cxx_std_20)

add_subdirectory(tests)

