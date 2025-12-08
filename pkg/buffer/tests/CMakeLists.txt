file(GLOB TEST_SOURCES "*.c")

set(BUFFER_TEST_TARGETS "")
set(BUFFER_TEST_COMMANDS "")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    target_link_libraries(${TEST_NAME} PRIVATE buffer testlib)
    
    set_target_properties(${TEST_NAME} PROPERTIES
        BUILD_RPATH "${CMAKE_BINARY_DIR}/pkg/buffer"
        INSTALL_RPATH "@executable_path/../lib"
    )
    
    list(APPEND BUFFER_TEST_TARGETS ${TEST_NAME})
    list(APPEND BUFFER_TEST_COMMANDS COMMAND $<TARGET_FILE:${TEST_NAME}>)
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    install(TARGETS ${TEST_NAME}
        RUNTIME DESTINATION libs/buffer/tests
    )
endforeach()

add_custom_target(run_buffer_buildtime_test ALL
    ${BUFFER_TEST_COMMANDS}
    DEPENDS ${BUFFER_TEST_TARGETS} buffer
    COMMENT "Running buffer tests..."
)
