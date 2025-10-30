# Custom target for all compiler tests
add_custom_target(compiler_tests
  COMMENT "Build all compiler end-to-end tests"
)

# Function: add_e2e_tests
#
# Creates an end-to-end test that compiles .program files with the compiler
# and links them with a C++ test framework.
#
# Usage:
#   add_e2e_tests(<test_name> <test_framework.cpp> <source1.program> ...)
#
# Arguments:
#   test_name      - Name of the test (will be prefixed with "e2e_")
#   test_framework - C++ file containing the test framework (e.g., main())
#   source_files   - One or more .program files to compile
#
function(add_e2e_tests test_name test_framework)
  set(e2e_test_name "e2e_${test_name}")
  set(e2e_test_objects "")

  # Process remaining arguments as source files
  foreach(source_file IN LISTS ARGN)
    set(e2e_source_file "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
    set(e2e_object_file "${CMAKE_CURRENT_BINARY_DIR}/${source_file}.o")

    # Create custom command to compile .program file to .o file
    add_custom_command(
      OUTPUT ${e2e_object_file}
      COMMAND compiler -i ${e2e_source_file} -o ${e2e_object_file}
      DEPENDS ${e2e_source_file} compiler
      COMMENT "Compiling ${source_file} with compiler"
    )

    list(APPEND e2e_test_objects ${e2e_object_file})
  endforeach()

  # Create test executable
  add_executable(${e2e_test_name} EXCLUDE_FROM_ALL
    ${test_framework}
    ${e2e_test_objects}
  )

  set_target_properties(${e2e_test_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  # Ensure the test is built when 'compiler_tests' target is built
  add_dependencies(compiler_tests ${e2e_test_name})

  # Register with CTest
  add_test(NAME ${e2e_test_name} COMMAND ${e2e_test_name})
endfunction()

# Add e2e test subdirectory
add_subdirectory(e2e)
