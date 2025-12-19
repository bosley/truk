truk_add_test(
    NAME test_ingestion
    SOURCES
        test_ingestion.cpp
    DEPENDENCIES
        truk_ingestion
)

truk_add_test(
    NAME test_parser
    SOURCES
        test_parser.cpp
    DEPENDENCIES
        truk_ingestion
)
