# Modern test infrastructure for the compiler project
# This replaces the dual .cpp/.program file approach with FileCheck-based testing

# Legacy function for backwards compatibility with e2e tests
add_custom_target(compiler_tests COMMENT "target to build tests")

function(add_e2e_tests test_name test_framework)
  set(e2e_test_name "e2e_${test_name}")
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

# Find Python3 for running test scripts
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Find FileCheck
find_program(FILECHECK_EXECUTABLE
    NAMES FileCheck
    PATHS /usr/lib/llvm-17/bin /usr/lib/llvm-16/bin /usr/lib/llvm-18/bin
    DOC "Path to FileCheck executable")

if(NOT FILECHECK_EXECUTABLE)
    message(WARNING "FileCheck not found - some tests will be skipped")
endif()

# Custom target for all new tests
add_custom_target(compiler_filecheck_tests
    COMMENT "Run FileCheck-based compiler tests")

# Function to add FileCheck-based tests
function(add_filecheck_tests test_category)
    if(NOT FILECHECK_EXECUTABLE)
        message(STATUS "Skipping ${test_category} tests - FileCheck not found")
        return()
    endif()

    # Create test target for this category
    set(test_target "filecheck_tests_${test_category}")
    add_custom_target(${test_target}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
                --compiler $<TARGET_FILE:compiler>
                --filecheck ${FILECHECK_EXECUTABLE}
                --test-dir ${CMAKE_CURRENT_SOURCE_DIR}/${test_category}
        DEPENDS compiler
        COMMENT "Running ${test_category} tests with FileCheck"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(compiler_filecheck_tests ${test_target})
    # Add to CTest
    add_test(NAME filecheck_${test_category}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
                --compiler $<TARGET_FILE:compiler>
                --filecheck ${FILECHECK_EXECUTABLE}
                --test-dir ${CMAKE_CURRENT_SOURCE_DIR}/${test_category}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

# Add tests for each category
add_filecheck_tests(syntax)
add_filecheck_tests(e2e)

# Add legacy e2e tests (keep for backwards compatibility until they can be migrated)
add_subdirectory(e2e)

# Add a target to run all tests (new and legacy)
add_custom_target(all_tests
    DEPENDS compiler_filecheck_tests compiler_tests
    COMMENT "Run all compiler tests")

# Print summary of test infrastructure
if(FILECHECK_EXECUTABLE)
    message(STATUS "FileCheck found at: ${FILECHECK_EXECUTABLE}")
    message(STATUS "New FileCheck-based tests will be available")
else()
    message(STATUS "FileCheck not found - only legacy e2e tests available")
endif()
