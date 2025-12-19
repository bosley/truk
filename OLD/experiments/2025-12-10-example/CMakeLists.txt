add_executable(example_experiment main.cpp)

target_compile_options(example_experiment PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

if(ENABLE_ASAN)
    target_compile_options(example_experiment PRIVATE -fsanitize=address)
    target_link_options(example_experiment PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(example_experiment PRIVATE -fsanitize=undefined)
    target_link_options(example_experiment PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_TSAN)
    target_compile_options(example_experiment PRIVATE -fsanitize=thread)
    target_link_options(example_experiment PRIVATE -fsanitize=thread)
endif()

target_link_libraries(example_experiment PRIVATE truk_core fmt::fmt)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
