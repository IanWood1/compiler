
file(GLOB ALL_PROGRAM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.program)
message(STATUS "ALL_PROGRAM_FILES: ${ALL_PROGRAM_FILES}")

set(TESTEE_FILES)
foreach (program_file ${ALL_PROGRAM_FILES})
  get_filename_component(PROGRAM_FILE_NAME ${program_file} NAME_WE)
  list(APPEND TESTEE_FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o)
  add_custom_command(
            OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o
            COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME} -i ${PROGRAM_FILE_NAME}.program -o ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_FILE_NAME}-testee.o
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM_FILE_NAME}.program compiler
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating object files from program files"
            VERBATIM
  )
endforeach (program_file)

message(STATUS "TESTEE_FILES: ${TESTEE_FILES}")

add_custom_target(
        build_tests ALL
        COMMENT tests
        DEPENDS ${TESTEE_FILES}
)


# Execute build custom target before building run-tests 
add_executable(run-tests "RunTests.cpp" ${TESTEE_FILES})
add_dependencies(run-tests build_tests)

add_test(mytest run-tests COMMAND run-tests)
