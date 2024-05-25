
file(GLOB ALL_PROGRAM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.program)
message(STATUS "ALL_PROGRAM_FILES: ${ALL_PROGRAM_FILES}")

set(TESTEE_FILES)
foreach (PROGRAM_FILE ${ALL_PROGRAM_FILES})
    get_filename_component(PROGRAM_FILE_NAME ${PROGRAM_FILE} NAME_WE)
    list(APPEND TESTEE_FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o)
    add_custom_command(
            OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o
            COMMAND ${PROJECT_BINARY_DIR}/src/compiler ${PROGRAM_FILE_NAME}.program ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM_FILE_NAME}.program
            DEPENDS compiler
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating object files from program files"
            VERBATIM
    )
endforeach (PROGRAM_FILE)
 
message(STATUS "TESTEE_FILES: ${TESTEE_FILES}")

add_custom_target(
        build_tests ALL
        DEPENDS ${TESTEE_FILES}
)


# Execute build custom target before building RunTests

add_executable(RunTests "RunTests.cpp" ${TESTEE_FILES} "TestFramework.h")
add_dependencies(RunTests build_tests)
target_link_libraries(RunTests Frontend)


add_test(mytest RunTests COMMAND RunTests)
