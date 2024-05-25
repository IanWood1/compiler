# CMakeList.txt : CMake project for compiler, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("compiler")


include(CTest)

# Add source to this project's executable. 
add_subdirectory(external/PEGTL EXCLUDE_FROM_ALL)
message(STATUS "ALL_PROGRAM_FILES: ${ALL_PROGRAM_FILES}")

add_subdirectory("src")
add_subdirectory("tests")
#add_executable (testprog test.cpp src/output.o)

target_compile_options(compiler PRIVATE -Wall)

message("flags ${CMAKE_CXX_FLAGS_DEBUG}")
