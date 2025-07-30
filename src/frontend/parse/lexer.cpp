#include "frontend/parse/lexer.h"
#include <cctype>
#include <unordered_map>

namespace frontend {
namespace lexer {

Lexer::Lexer(const std::string& input) 
    : input_(input), pos_(0), line_(1), column_(1), peeked_token_(nullptr) {}

char Lexer::currentChar() const {
    if (pos_ >= input_.size()) {
        return '\0';
    }
    return input_[pos_];
}

char Lexer::peekChar(size_t offset) const {
    size_t peek_pos = pos_ + offset;
    if (peek_pos >= input_.size()) {
        return '\0';
    }
    return input_[peek_pos];
}

void Lexer::advance() {
    if (pos_ < input_.size()) {
        if (input_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

void Lexer::skipWhitespace() {
    while (std::isspace(currentChar())) {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar() == '/' && peekChar() == '/') {
        // Skip until end of line
        while (currentChar() != '\n' && currentChar() != '\0') {
            advance();
        }
    }
}

bool Lexer::isAlpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Lexer::isAlphaNum(char c) const {
    return isAlpha(c) || isDigit(c);
}

TokenType Lexer::getKeywordType(const std::string& text) const {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"return", TokenType::RETURN},
        {"while", TokenType::WHILE},
        {"if", TokenType::IF},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"struct", TokenType::STRUCT},
        {"print", TokenType::PRINT},
        {"input", TokenType::INPUT}
    };
    
    auto it = keywords.find(text);
    return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
}

Token Lexer::readIdentifierOrKeyword() {
    size_t start_line = line_;
    size_t start_column = column_;
    std::string text;
    
    while (isAlphaNum(currentChar())) {
        text += currentChar();
        advance();
    }
    
    TokenType type = getKeywordType(text);
    return Token(type, text, start_line, start_column);
}

Token Lexer::readNumber() {
    size_t start_line = line_;
    size_t start_column = column_;
    std::string text;
    
    // Handle optional sign
    if (currentChar() == '+' || currentChar() == '-') {
        text += currentChar();
        advance();
    }
    
    while (isDigit(currentChar())) {
        text += currentChar();
        advance();
    }
    
    return Token(TokenType::NUMBER, text, start_line, start_column);
}

Token Lexer::readOperator() {
    size_t start_line = line_;
    size_t start_column = column_;
    char first = currentChar();
    char second = peekChar();
    
    // Two-character operators
    if (first == '<' && second == '=') {
        advance();
        advance();
        return Token(TokenType::LESS_EQUAL, "<=", start_line, start_column);
    }
    if (first == '>' && second == '=') {
        advance();
        advance();
        return Token(TokenType::GREATER_EQUAL, ">=", start_line, start_column);
    }
    if (first == '=' && second == '=') {
        advance();
        advance();
        return Token(TokenType::EQUAL, "==", start_line, start_column);
    }
    if (first == '<' && second == '<') {
        advance();
        advance();
        return Token(TokenType::LEFT_SHIFT, "<<", start_line, start_column);
    }
    if (first == '>' && second == '>') {
        advance();
        advance();
        return Token(TokenType::RIGHT_SHIFT, ">>", start_line, start_column);
    }
    
    // Single-character operators and delimiters
    advance();
    switch (first) {
        case '=': return Token(TokenType::ASSIGN, "=", start_line, start_column);
        case '+': return Token(TokenType::PLUS, "+", start_line, start_column);
        case '-': return Token(TokenType::MINUS, "-", start_line, start_column);
        case '*': return Token(TokenType::MULTIPLY, "*", start_line, start_column);
        case '<': return Token(TokenType::LESS_THAN, "<", start_line, start_column);
        case '>': return Token(TokenType::GREATER_THAN, ">", start_line, start_column);
        case '&': return Token(TokenType::AMPERSAND, "&", start_line, start_column);
        case '(': return Token(TokenType::LEFT_PAREN, "(", start_line, start_column);
        case ')': return Token(TokenType::RIGHT_PAREN, ")", start_line, start_column);
        case '{': return Token(TokenType::LEFT_BRACE, "{", start_line, start_column);
        case '}': return Token(TokenType::RIGHT_BRACE, "}", start_line, start_column);
        case '[': return Token(TokenType::LEFT_BRACKET, "[", start_line, start_column);
        case ']': return Token(TokenType::RIGHT_BRACKET, "]", start_line, start_column);
        case ',': return Token(TokenType::COMMA, ",", start_line, start_column);
        case ';': return Token(TokenType::SEMICOLON, ";", start_line, start_column);
        default:
            return Token(TokenType::UNKNOWN, std::string(1, first), start_line, start_column);
    }
}

Token Lexer::nextToken() {
    if (peeked_token_) {
        auto token = *peeked_token_;
        peeked_token_.reset();
        return token;
    }
    
    skipWhitespace();
    
    // Skip comments
    while (currentChar() == '/' && peekChar() == '/') {
        skipComment();
        skipWhitespace();
    }
    
    if (currentChar() == '\0') {
        return Token(TokenType::END_OF_FILE, "", line_, column_);
    }
    
    if (isAlpha(currentChar())) {
        return readIdentifierOrKeyword();
    }
    
    if (isDigit(currentChar()) || 
        ((currentChar() == '+' || currentChar() == '-') && isDigit(peekChar()))) {
        return readNumber();
    }
    
    return readOperator();
}

Token Lexer::peekToken() {
    if (!peeked_token_) {
        peeked_token_ = std::make_unique<Token>(nextToken());
    }
    return *peeked_token_;
}

bool Lexer::hasMoreTokens() const {
    return pos_ < input_.size();
}

} // namespace lexer
} // namespace frontend