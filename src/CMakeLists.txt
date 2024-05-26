
# Add source to this project's executable.
add_subdirectory(frontend)
add_executable(${PROJECT_NAME} compiler.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(compiler 
  frontend_parse
  frontend_visitor
  frontend_ast
  frontend_codegen
  frontend_types
  Boost::program_options

  LLVM 
)

# Find the libraries that correspond to the LLVM components
# that we wish to use

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET compiler PROPERTY CXX_STANDARD 20)
endif ()
 
