# Frontend library modules
add_subdirectory(frontend)

# Compiler executable
add_executable(${PROJECT_NAME} compiler.cpp)

# Set C++ standard (already set globally in root CMakeLists.txt)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
  -Werror
  -Wall
  -Wextra
  -Wpedantic
)

# Link against frontend libraries and LLVM
target_link_libraries(${PROJECT_NAME} PRIVATE
  frontend_parse
  frontend_visitor
  frontend_ast
  frontend_codegen
  frontend_types
  LLVM
)
