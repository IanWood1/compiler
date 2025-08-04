#!/usr/bin/env python3
"""
Simple test runner for the compiler project.
This script runs FileCheck-based tests to replace the dual .cpp/.program file approach.
"""

import os
import sys
import subprocess
import argparse
import tempfile
from pathlib import Path

def run_compiler(compiler_path, input_file, output_file=None, debug=False):
    """Run the compiler on an input file."""
    cmd = [compiler_path, "-i", input_file]
    if output_file:
        cmd.extend(["-o", output_file])
    else:
        # Create a temporary output file if none specified
        import tempfile
        temp_output = tempfile.NamedTemporaryFile(suffix='.o', delete=False)
        temp_output.close()
        cmd.extend(["-o", temp_output.name])
        output_file = temp_output.name
    if debug:
        cmd.append("-d")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        # Clean up temporary output file if we created one
        if not output_file.startswith('/tmp/'):
            try:
                os.unlink(output_file)
            except:
                pass
        return result
    except subprocess.TimeoutExpired:
        return None

def run_filecheck(filecheck_path, check_file, input_text):
    """Run FileCheck on input text with check patterns from check_file."""
    try:
        result = subprocess.run(
            [filecheck_path, check_file],
            input=input_text,
            capture_output=True,
            text=True,
            timeout=10
        )
        return result.returncode == 0, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return False, "", "FileCheck timeout"

def find_test_files(test_dir):
    """Find all test files in a directory."""
    test_files = []
    for root, dirs, files in os.walk(test_dir):
        for file in files:
            if file.endswith('.test'):
                test_files.append(os.path.join(root, file))
    return sorted(test_files)

def run_single_test(test_file, compiler_path, filecheck_path):
    """Run a single test file."""
    test_name = os.path.relpath(test_file)
    print(f"Running test: {test_name}")

    # Read the test file
    with open(test_file, 'r') as f:
        content = f.read()

    # For FileCheck-style tests, use the entire file
    # The compiler will ignore comment lines, and FileCheck will automatically extract CHECK patterns
    if '// CHECK:' in content or '// CHECK-' in content:
        # Run compiler directly on the test file - it will ignore CHECK comments
        temp_output = tempfile.NamedTemporaryFile(suffix='.o', delete=False)
        temp_output.close()
        result = run_compiler(compiler_path, test_file, temp_output.name, debug=True)

        if result is None:
            print(f"  TIMEOUT: {test_name}")
            return False

        if result.returncode != 0:
            print(f"  FAIL: {test_name} - Compiler error")
            print(f"    stderr: {result.stderr}")
            return False

        # Run FileCheck using the original test file - it will automatically extract CHECK patterns
        check_success, check_stdout, check_stderr = run_filecheck(
            filecheck_path, test_file, result.stdout + result.stderr
        )

        # Clean up temporary files
        try:
            os.unlink(temp_output.name)
        except:
            pass

        if check_success:
            print(f"  PASS: {test_name}")
            return True
        else:
            print(f"  FAIL: {test_name} - FileCheck failed")
            print(f"    FileCheck stderr: {check_stderr}")
            return False
    else:
        print(f"  SKIP: {test_name} - No CHECK patterns found")
        return True

def main():
    parser = argparse.ArgumentParser(description='Run compiler tests')
    parser.add_argument('--compiler', default='./build/bin/compiler',
                       help='Path to compiler binary')
    parser.add_argument('--filecheck', default='/usr/lib/llvm-17/bin/FileCheck',
                       help='Path to FileCheck binary')
    parser.add_argument('--test-dir', default='tests',
                       help='Directory containing tests')
    parser.add_argument('test_pattern', nargs='?', default='',
                       help='Pattern to match test names')

    args = parser.parse_args()

    # Check if tools exist
    if not os.path.exists(args.compiler):
        print(f"Error: Compiler not found at {args.compiler}")
        return 1

    if not os.path.exists(args.filecheck):
        print(f"Error: FileCheck not found at {args.filecheck}")
        return 1

    # Find and run tests
    test_files = find_test_files(args.test_dir)
    if args.test_pattern:
        test_files = [t for t in test_files if args.test_pattern in t]

    if not test_files:
        print("No test files found")
        return 0

    passed = 0
    total = len(test_files)

    for test_file in test_files:
        if run_single_test(test_file, args.compiler, args.filecheck):
            passed += 1

    print(f"\nResults: {passed}/{total} tests passed")
    return 0 if passed == total else 1

if __name__ == '__main__':
    sys.exit(main())
