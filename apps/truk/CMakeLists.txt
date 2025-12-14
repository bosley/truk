add_executable(truk 
    main.cpp
    commands/compile.cpp
    commands/run.cpp
    commands/toc.cpp
    commands/tcc.cpp
    common/args.cpp
)

target_compile_options(truk PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

if(ENABLE_ASAN)
    target_compile_options(truk PRIVATE -fsanitize=address)
    target_link_options(truk PRIVATE -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(truk PRIVATE -fsanitize=undefined)
    target_link_options(truk PRIVATE -fsanitize=undefined)
endif()

if(ENABLE_TSAN)
    target_compile_options(truk PRIVATE -fsanitize=thread)
    target_link_options(truk PRIVATE -fsanitize=thread)
endif()

target_link_libraries(truk PRIVATE 
    truk_emitc
    truk_validation
    truk_tcc
    fmt::fmt
)

install(TARGETS truk
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
