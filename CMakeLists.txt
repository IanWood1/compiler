# CMakeList.txt : CMake project for compiler, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)
# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()
set(CMAKE_CXX_STANDARD 20)


project("compiler")

# Add source to this project's executable. 
add_subdirectory("src")
add_subdirectory("tests")
#add_executable (testprog test.cpp src/output.o)

if (MSVC)
    message("using msvc")
    set(CMAKE_CXX_FLAGS "/W4" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/EHsc /MDd /O2 /Ob3" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/EHsc /MDd /ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
else ()
    message("using gcc/clang")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g  -O0" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall  -g  -O0" CACHE STRING "" FORCE)
endif ()

message("flags ${CMAKE_CXX_FLAGS_DEBUG}")
 
