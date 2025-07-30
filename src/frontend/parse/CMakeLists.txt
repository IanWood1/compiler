

add_library(frontend_parse
  lexer.cpp
  new_parser.cpp
  parser_new.cpp
)

target_link_libraries(frontend_parse PRIVATE LLVM)
