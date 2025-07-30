# CMakeList.txt : CMake project for compiler, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Setting up ccache or sccache
find_program(CCACHE "ccache")
if(CCACHE)
  message("${CCACHE} has been found in the system!")
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  find_program(SCCACHE "sccache")
  if(SCCACHE)
    message("${SCCACHE} has been found in the system!")
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE})
  else()
    message("neither ccache or sccache have been found in the system!")
  endif(SCCACHE)
endif(CCACHE)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project("compiler")

include(CTest)

# Add source to this project's executable.
# External dependencies moved to individual modules

# Only allow LLVM version 17.0.0
set(LLVM_VERSION 17.0.6)

find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
# separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})
# add_library(LLVM INTERFACE)

include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS "ALL_PROGRAM_FILES: ${ALL_PROGRAM_FILES}")

add_subdirectory("src")
add_subdirectory("tests")
#add_executable (testprog test.cpp src/output.o)

target_compile_options(compiler PRIVATE -Wall)

message("flags ${CMAKE_CXX_FLAGS_DEBUG}")
