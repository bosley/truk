function(truk_add_library)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES HEADERS DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${ARG_NAME} ${ARG_SOURCES} ${ARG_HEADERS})
    
    target_compile_options(${ARG_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )

    if(ENABLE_ASAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=address)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=address)
    endif()

    if(ENABLE_UBSAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=undefined)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=undefined)
    endif()

    if(ENABLE_TSAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=thread)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=thread)
    endif()

    if(ARG_DEPENDENCIES)
        target_link_libraries(${ARG_NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()
endfunction()

function(truk_add_test)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${ARG_NAME} ${ARG_SOURCES})
    
    target_compile_options(${ARG_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )

    if(ENABLE_ASAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=address)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=address)
    endif()

    if(ENABLE_UBSAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=undefined)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=undefined)
    endif()

    if(ENABLE_TSAN)
        target_compile_options(${ARG_NAME} PRIVATE -fsanitize=thread)
        target_link_options(${ARG_NAME} PRIVATE -fsanitize=thread)
    endif()

    target_link_libraries(${ARG_NAME} PRIVATE CppUTest CppUTestExt)
    
    if(ARG_DEPENDENCIES)
        target_link_libraries(${ARG_NAME} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    add_test(NAME ${ARG_NAME} COMMAND $<TARGET_FILE:${ARG_NAME}>)
    
    set_property(GLOBAL APPEND PROPERTY TRUK_TEST_TARGETS ${ARG_NAME})
endfunction()
