truk_add_test(
    NAME test_emitter
    SOURCES
        test_emitter.cpp
    DEPENDENCIES
        truk_emitc
        truk_ingestion
        truk_language
        truk_core
)
