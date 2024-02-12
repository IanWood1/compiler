set(${MY_COMPILER_EXE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
#set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)


find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
add_library(LLVM INTERFACE)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source to this project's executable.
add_subdirectory(frontend)
add_executable(compiler compiler.cpp)
target_link_libraries(compiler Frontend)
add_dependencies(compiler Frontend)

# Find the libraries that correspond to the LLVM components
# that we wish to use
target_include_directories(LLVM SYSTEM INTERFACE ${LLVM_INCLUDE_DIRS})
target_link_libraries(LLVM INTERFACE ${LLVM_AVAILABLE_LIBS})
target_compile_definitions(LLVM INTERFACE ${LLVM_DEFINITIONS} -DNOMINMAX)


# Find the libraries that correspond to the LLVM components
# that we wish to use

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET compiler PROPERTY CXX_STANDARD 20)
endif ()
 


