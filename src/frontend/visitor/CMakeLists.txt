

add_library(frontend_visitor

  ApplyTypesBuilder.cpp
  DumpAST.cpp
  IRInstructionGen.cpp
  IRValueGen.cpp

)



target_link_libraries(frontend_visitor PRIVATE LLVM)
