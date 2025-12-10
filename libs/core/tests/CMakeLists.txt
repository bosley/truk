truk_add_test(
    NAME test_core
    SOURCES
        test_core.cpp
    DEPENDENCIES
        truk_core
)

truk_add_test(
    NAME test_memory
    SOURCES
        test_memory.cpp
    DEPENDENCIES
        truk_core
)
