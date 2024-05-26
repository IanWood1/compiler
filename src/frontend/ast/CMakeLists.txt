

add_library(frontend_ast
  ast.cpp
)


target_link_libraries(frontend_ast PRIVATE LLVM)
