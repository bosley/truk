truk_add_test(
    NAME test_typecheck
    SOURCES
        test_typecheck.cpp
    DEPENDENCIES
        truk_validation
        ingestion
        truk_language
        truk_core
)

truk_add_test(
    NAME test_builtins
    SOURCES
        test_builtins.cpp
    DEPENDENCIES
        truk_validation
        ingestion
        truk_language
        truk_core
)
