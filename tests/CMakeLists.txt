add_custom_target(compiler_tests COMMENT "target to build tests")

function(add_e2e_tests test_name test_framework)
  set(e2e_test_name "test_e2e_${test_name}")
  set(e2e_test_objects "")

  # Shift the first two arguments and process the remaining ones as source files
  cmake_parse_arguments(ARG "" "" "source_files" ${ARGN})

  # Compile each source file into an object file
  foreach(source_file IN LISTS ARG_UNPARSED_ARGUMENTS)
    set(e2e_source_file "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
    set(e2e_object_file "${CMAKE_CURRENT_BINARY_DIR}/${source_file}.o")
    set(e2e_test "${CMAKE_CURRENT_BINARY_DIR}/${e2e_test_name}")
    add_custom_command(
      OUTPUT ${e2e_object_file}
      COMMAND compiler -i ${e2e_source_file} -o ${e2e_object_file}
      DEPENDS ${e2e_source_file}
      COMMENT "Generating object file for ${source_file}")
    list(APPEND e2e_test_objects ${e2e_object_file})
  endforeach()

  # Create the executable by linking the test framework and object files
  add_executable(${e2e_test_name} EXCLUDE_FROM_ALL ${test_framework}
                                                   ${e2e_test_objects})
  set_target_properties(${e2e_test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                    ${CMAKE_CURRENT_BINARY_DIR})
  add_dependencies(compiler_tests ${e2e_test_name})

  # Add the test to CTest
  add_test(NAME ${e2e_test_name} COMMAND ${e2e_test})
endfunction()



