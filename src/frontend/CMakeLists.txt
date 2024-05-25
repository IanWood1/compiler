
file(GLOB_RECURSE FRONTEND_SOURCES *)
add_library(Frontend ${FRONTEND_SOURCES}
        visitor/DumpAST.cpp
        visitor/ApplyTypesBuilder.cpp
)


if (TARGET taocpp::pegtl)
    message(STATUS "PEGTL target is available.")
else()
    message(FATAL_ERROR "PEGTL target is not available. Ensure PEGTL is added as a subdirectory correctly.")
endif()

target_link_libraries(Frontend PRIVATE LLVM taocpp::pegtl)

