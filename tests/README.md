# Compiler Test Suite

This directory contains the test suite for the compiler project. The test infrastructure has been modernized to use FileCheck-based testing, eliminating the need for dual .cpp/.program files.

## Test Organization

Tests are organized into categories based on what they test:

### `syntax/`
Basic syntax and parsing tests that verify the compiler can correctly parse language constructs:
- Function declarations
- Variable declarations and assignments
- Array syntax
- Control flow statements

### `e2e/`
End-to-end tests that test complete compilation and execution:
- Simple arithmetic and function calls
- Complex programs with multiple functions
- Array operations and memory management
- Reference parameter passing semantics
- Control flow constructs (while loops, if statements)
- Nested function calls and complex expressions
- Real-world usage scenarios and integration of multiple language features

## Test Format

New tests use a single `.test` file format that includes a RUN line specifying the command and CHECK assertions for execution output validation:

```
// RUN: %compiler -i %s -o %t.o && %t.o
// Test description
int64 my_function(int64 x){
  return x + 1
}

int64 main(){
  return my_function(5)
}

// CHECK: 6
```

The test runner uses the RUN line to compile and execute the program, then validates the output against CHECK patterns.

## Running Tests

### All Tests
```bash
# From build directory
make all_tests
```

### Category-Specific Tests
```bash
make filecheck_tests_syntax
make filecheck_tests_e2e
```

### Legacy Tests
```bash
make compiler_tests
```

### Individual Tests
```bash
# From tests directory
./run_test.py --test-dir syntax
./run_test.py syntax/basic_function.test
```

### Using CTest
```bash
ctest -R filecheck
ctest -R filecheck_syntax
ctest -R filecheck_e2e
```

## Adding New Tests

1. Choose the appropriate category directory
2. Create a `.test` file with descriptive name
3. Include source code and CHECK assertions
4. Verify the test works: `./run_test.py path/to/your.test`

## Test Naming Convention

- Use descriptive names: `basic_function.test`, `while_loop.test`
- Group related tests: `array_basic.test`, `array_references.test`
- Include edge cases: `array_bounds.test`, `empty_function.test`

## FileCheck Patterns

For e2e tests, CHECK patterns validate execution output:

- Return values: `// CHECK: 42` (exact value)
- Multiple outputs: `// CHECK: 10` followed by `// CHECK: 20`
- String patterns: `// CHECK: {{[0-9]+}}` (regex for numbers)

For syntax tests, CHECK patterns validate AST structure:

- Function definitions: `// CHECK: define int64`
- Type information: `// CHECK: type: int64`
- Control structures: `// CHECK: while`, `// CHECK: if`

See [FileCheck documentation](https://llvm.org/docs/CommandGuide/FileCheck.html) for full pattern syntax.
