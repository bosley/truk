add_executable(meta main.cpp)

target_compile_options(meta PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

if(ENABLE_ASAN)
    target_compile_options(meta PRIVATE -fsanitize=address)
    target_link_options(meta PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(meta PRIVATE -fsanitize=undefined)
    target_link_options(meta PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_TSAN)
    target_compile_options(meta PRIVATE -fsanitize=thread)
    target_link_options(meta PRIVATE -fsanitize=thread)
endif()

target_link_libraries(meta PRIVATE 
    truk_core 
    truk_emitc
    truk_ingestion
    truk_validation
    fmt::fmt
)
