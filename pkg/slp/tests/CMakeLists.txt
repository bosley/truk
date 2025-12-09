file(GLOB TEST_SOURCES "*.c")

set(SLP_TEST_TARGETS "")
set(SLP_TEST_COMMANDS "")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    target_link_libraries(${TEST_NAME} PRIVATE slp testlib)
    
    set_target_properties(${TEST_NAME} PROPERTIES
        BUILD_RPATH "${CMAKE_BINARY_DIR}/pkg/slp"
        INSTALL_RPATH "@executable_path/../lib"
    )
    
    list(APPEND SLP_TEST_TARGETS ${TEST_NAME})
    list(APPEND SLP_TEST_COMMANDS COMMAND $<TARGET_FILE:${TEST_NAME}>)
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    install(TARGETS ${TEST_NAME}
        RUNTIME DESTINATION libs/slp/tests
    )
endforeach()

add_custom_target(run_slp_buildtime_test ALL
    ${SLP_TEST_COMMANDS}
    DEPENDS ${SLP_TEST_TARGETS} slp
    COMMENT "Running slp tests..."
)

