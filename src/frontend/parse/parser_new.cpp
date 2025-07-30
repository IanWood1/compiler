#include "frontend/parse/parser.h"
#include "frontend/parse/lexer.h"
#include "frontend/parse/new_parser.h"
#include "frontend/diagnostic/debug.h"
#include <fstream>
#include <sstream>

namespace frontend {

Program parseFile(const char* file_name) {
    // Read file contents
    std::ifstream file(file_name);
    if (!file.is_open()) {
        FRONTEND_ERROR("Could not open file: " + std::string(file_name));
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();
    file.close();
    
    // Create lexer and parser
    lexer::Lexer lexer(input);
    parser::RecursiveDescentParser parser(lexer);
    
    // Parse the program
    return parser.parseProgram();
}

}  // namespace frontend