
file(GLOB_RECURSE FRONTEND_SOURCES *)
add_library(Frontend ${FRONTEND_SOURCES}
        visitor/DumpAST.cpp
        visitor/ApplyTypesBuilder.cpp
)


target_link_libraries(Frontend LLVM)

