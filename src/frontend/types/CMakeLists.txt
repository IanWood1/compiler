


add_library(frontend_types
  VarType.cpp
)


target_link_libraries(frontend_types PRIVATE
  LLVM
)
