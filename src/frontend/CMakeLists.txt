add_subdirectory(parse)
add_subdirectory(visitor)
add_subdirectory(ast)


add_library(Frontend 
  code_generator.cpp 
  VarType.cpp
)

target_link_libraries(Frontend PRIVATE 
  frontend_parse
  frontend_visitor
  frontend_ast

  LLVM 
)

