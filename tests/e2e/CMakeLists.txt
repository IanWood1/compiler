# E2E test with multiple test programs in one executable
add_e2e_tests(
  minitests
  minitests.cpp
  test1.program
  test2.program
  test3.program
  test4.program
  minitests.program
)

# Individual E2E tests - each test has its own executable and .program file
set(INDIVIDUAL_TESTS
  test1
  test2
  test3
  test4
  test_controlflow
  test_operators
  test_arrays
)

foreach(test_name IN LISTS INDIVIDUAL_TESTS)
  add_e2e_tests(
    ${test_name}
    ${test_name}.cpp
    ${test_name}.program
  )
endforeach()
