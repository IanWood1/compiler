#pragma once

#include <string>
#include <vector>
#include <memory>

namespace frontend {
namespace parser {
class RecursiveDescentParser;
}
namespace lexer {

enum class TokenType {
    // Keywords
    RETURN,
    WHILE,
    IF,
    BREAK,
    CONTINUE,
    STRUCT,
    PRINT,
    INPUT,
    
    // Identifiers and literals
    IDENTIFIER,
    NUMBER,
    
    // Operators
    ASSIGN,         // =
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    LESS_THAN,      // <
    GREATER_THAN,   // >
    LESS_EQUAL,     // <=
    GREATER_EQUAL,  // >=
    EQUAL,          // ==
    LEFT_SHIFT,     // <<
    RIGHT_SHIFT,    // >>
    AMPERSAND,      // &
    
    // Delimiters
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    COMMA,          // ,
    SEMICOLON,      // ;
    
    // Special
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
    size_t line;
    size_t column;
    
    Token(TokenType t, std::string txt, size_t ln, size_t col)
        : type(t), text(std::move(txt)), line(ln), column(col) {}
};

class Lexer {
public:
    explicit Lexer(const std::string& input);
    
    Token nextToken();
    Token peekToken();
    bool hasMoreTokens() const;
    
    // Allow parser to access position for lookahead
    friend class frontend::parser::RecursiveDescentParser;
    
private:
    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;
    std::unique_ptr<Token> peeked_token_;
    
    char currentChar() const;
    char peekChar(size_t offset = 1) const;
    void advance();
    void skipWhitespace();
    void skipComment();
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNum(char c) const;
    
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readOperator();
    TokenType getKeywordType(const std::string& text) const;
};

} // namespace lexer
} // namespace frontend