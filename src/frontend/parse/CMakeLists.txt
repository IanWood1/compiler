

add_library(frontend_parse
  parser.cpp
)


target_link_libraries(frontend_parse PRIVATE LLVM taocpp::pegtl)
