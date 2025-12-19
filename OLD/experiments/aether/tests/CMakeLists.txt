truk_add_test(
    NAME test_forms
    SOURCES
        test_forms.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_bool
    SOURCES
        test_bool.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_arrays
    SOURCES
        test_arrays.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_structs
    SOURCES
        test_structs.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_struct_arrays
    SOURCES
        test_struct_arrays.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_bounds
    SOURCES
        test_bounds.cpp
    DEPENDENCIES
        truk_aether
)

truk_add_test(
    NAME test_string
    SOURCES
        test_string.cpp
    DEPENDENCIES
        truk_aether
)
