

add_library(frontend_codegen
  code_generator.cpp
)


target_link_libraries(frontend_codegen PRIVATE
  frontend_ast
  frontend_types
  frontend_visitor
  LLVM
)
