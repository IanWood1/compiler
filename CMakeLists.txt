# CMake project for compiler
cmake_minimum_required(VERSION 3.15)

project("compiler")

# ============================================================================
# Build Configuration
# ============================================================================

# Prevent in-source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Generate compile_commands.json for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Compiler Cache Setup
# ============================================================================

find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)

if(CCACHE_PROGRAM)
  message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
elseif(SCCACHE_PROGRAM)
  message(STATUS "Found sccache: ${SCCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
else()
  message(STATUS "No compiler cache found (ccache or sccache)")
endif()

# ============================================================================
# Testing
# ============================================================================

include(CTest)

# ============================================================================
# External Dependencies
# ============================================================================

# PEGTL parsing library
add_subdirectory(external/PEGTL EXCLUDE_FROM_ALL)

# LLVM
set(LLVM_VERSION 17.0.6)
find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include directories: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

# Include LLVM headers as SYSTEM to suppress warnings from LLVM code
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# ============================================================================
# Project Include Directories
# ============================================================================

include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Subdirectories
# ============================================================================

add_subdirectory(src)
add_subdirectory(tests)
