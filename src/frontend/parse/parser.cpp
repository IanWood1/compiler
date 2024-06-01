#include "frontend/parse/parser.h"
#include <cassert>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "frontend/ast.h"
#include "frontend/diagnostic/debug.h"
#include "frontend/visitor/DumpAST.h"

namespace frontend {

#define TokenTypeMacro(Enum) Enum,
#define OperatorMacro(Enum, _) Enum,
#define SymbolMacro(Enum, _) Enum,
enum class TokenType {
#include "TokenTypes.def"
  NumTokenTypes
};

#define TokenTypeMacro(Enum) #Enum,
#define OperatorMacro(Enum, _) #Enum,
#define SymbolMacro(Enum, _) #Enum,
const char* tokenTypeStrings[] = {
#include "TokenTypes.def"
};

// Generate the string mapping
#define OperatorMacro(name, str) {str, TokenType::name},
std::map<std::string, TokenType> operatorMap = {
#include "TokenTypes.def"
};

static bool isOperator(const std::string& str) {
  return operatorMap.find(str) != operatorMap.end();
}

static TokenType getOperator(const std::string& str) {
  assert(isOperator(str) && "expected valid operator string");
  return operatorMap[str];
}

// Function to get the string representation of the enum
std::string tokenTypeToString(TokenType type) {
  return tokenTypeStrings[static_cast<int>(type)];
}

#define SymbolMacro(name, charr) {charr, TokenType::name},
std::map<char, TokenType> symbolMap = {
#include "TokenTypes.def"
};

static bool isSymbol(char c) {
  return symbolMap.find(c) != symbolMap.end();
}

static TokenType getSymbol(char c) {
  assert(isSymbol(c) && "expected valid symbol character");
  return symbolMap[c];
}

class Reader {
 public:
  // Constructor that initializes the reader with a string
  explicit Reader(std::string input) : input_(std::move(input)) {}

  // Method to get the next character without consuming it
  [[nodiscard]] char next() const {
    assert(position_ < input_.length());
    return input_[position_];
  }

  // Method to get the k-th character from the current position without consuming it
  [[nodiscard]] char next(size_t k) const {
    size_t targetPos = position_ + k;
    assert(position_ < input_.length());
    return input_[targetPos];
  }

  // Method to consume the next character and return it
  char consume() {
    assert(position_ < input_.length());
    char c = input_[position_++];
    if (c == '\n') {
      line_++;
      col_ = 1;
    }
    return c;
  }

  // Method to consume the next k characters and return them as a string
  std::string consume(size_t k) {
    assert(position_ + k < input_.length());
    std::string result = input_.substr(position_, k);
    for (char c : result) {
      if (c == '\n') {
        line_++;
        col_ = 1;
      } else {
        col_++;
      }
    }
    position_ += k;
    return result;
  }

  // Method to get the current position in the input
  size_t getPosition() const { return position_; }
  size_t getLine() const { return line_; }
  size_t getCol() const { return col_; }
  bool isEOF() const { return position_ >= input_.size(); }

 private:
  std::string input_;
  size_t position_{0};
  size_t line_{1};
  size_t col_{1};
};

class Token {
 public:
  Token(TokenType type, Reader& reader, size_t length) : type_(type) {
    assert(length > 0);
    line_ = reader.getLine();
    column_ = reader.getCol();
    value_ = reader.consume(length);
    assert(value_.find(' ') == value_.npos &&
           "token shouldnt contain whitespace");
  }

  explicit Token(TokenType type, const Reader& reader)
      : type_(type),
        value_("$_EOF"),
        line_(reader.getLine()),
        column_(reader.getCol()) {}

  // Accessors
  TokenType getType() const { return type_; }
  const std::string& getValue() const { return value_; }
  int getLine() const { return line_; }
  int getColumn() const { return column_; }

  // Other usefull
  bool isEOF() const noexcept { return type_ == TokenType::EndOfFile; }

  // Debugging/Logging Method
  void print() const {
    std::cout << "Token(Type: " << tokenTypeToString(type_)
              << ", Value: " << value_ << ", Line: " << line_
              << ", Column: " << column_ << ")\n";
  }

 private:
  TokenType type_;
  std::string value_;
  size_t line_;
  size_t column_;
};

class Tokenizer {
 public:
  explicit Tokenizer(Reader reader) : reader_(std::move(reader)) {}

  Token get() {
    skipSpaces();

    if (reader_.isEOF()) {
      return Token(TokenType::EndOfFile, reader_);
    }

    auto curr = reader_.next();
    assert(curr != ' ');

    if (isalpha(curr))
      return getIdentifier();
    if (isalnum(curr))
      return getIntegerLiteral();
    if (isSymbol(curr))
      return Token(getSymbol(curr), reader_, 1);

    std::string op = {curr};
    if (isOperator(op))
      return Token(getOperator(op), reader_, 1);
    op += reader_.next();
    if (isOperator(op))
      return Token(getOperator(op), reader_, 2);

    FRONTEND_ERROR("no matching token found");
  }

 private:
  Token getIdentifier() {
    size_t start = reader_.getPosition();
    size_t length = readWhile([](const char c) {
      return (static_cast<bool>(std::isalnum(c))) || c == '_';
    });
    assert(length != 0 && "identifier must have at least length 1");
    return Token(TokenType::Identifier, reader_, length);
  }

  Token getIntegerLiteral() {
    size_t start = reader_.getPosition();
    size_t length = readWhile([](const char c) {
      return (static_cast<bool>(std::isalnum(c)) &&
              !static_cast<bool>(std::isalpha(c)));
    });
    assert(length != 0 && "integer literal must have at least length 1");
    return Token(TokenType::IntegerLiteral, reader_, length);
  }

  //==-----------------------------==//
  // Helpers
  //==-----------------------------==//
  void skipSpaces() {
    while (!reader_.isEOF() &&
           (isspace(reader_.next()) || reader_.next() == '\n')) {
      reader_.consume();
    }
  }

  // Returns the number of chars read and evaluated to true
  size_t readWhile(const std::function<bool(char)>& pred) {
    size_t curr = 0;
    while (!reader_.isEOF() && pred(reader_.next(curr++))) {}
    return curr - 1;
  }

  Reader reader_;
};

class Context {
 public:
  explicit Context(Tokenizer& tokenizer, std::vector<Token>& tokBuffer,
                   size_t& startPos)
      : tokenizer_(tokenizer),
        tokBuffer_(tokBuffer),
        startPos_(startPos),
        currPos_(startPos) {}

  ~Context() {
    assert(notified_ && "context destructed without notifying first");
  }

  Token next() {
    if (currPos_ >= tokBuffer_.size()) {
      tokBuffer_.push_back(tokenizer_.get());
    }
    tokBuffer_[currPos_].print();
    return tokBuffer_[currPos_++];
  }

  bool expectNext(TokenType type) { return next().getType() == type; }

  std::nullopt_t notifyFailure() {
    assert(!notified_);
    notified_ = true;
    currPos_ = startPos_;
    return std::nullopt;
  }

  std::nullopt_t notifyFailure(const std::string& msg) {
    assert(!notified_ && currPos_ != startPos_);
    std::cout << "Local Parse Failure: " << msg << "\n";
    notified_ = true;
    currPos_ = startPos_;
    return std::nullopt;
  }

  template <typename T>
  T notifySuccess(T t) {
    assert(!notified_ && currPos_ != startPos_);
    std::cout << "success!\n";
    notified_ = true;
    return t;
  }

 private:
  Tokenizer& tokenizer_;
  std::vector<Token>& tokBuffer_;
  size_t startPos_;
  size_t& currPos_;
  bool notified_ = false;
};

class Parser {
 public:
  explicit Parser(Reader reader) : tokenizer_(std::move(reader)) {}

  Program parse() {
    Program prog;
    bool found = true;
    while (found) {
      found = false;
      if (auto astFunc = getFuncDef()) {
        found = true;
        prog.functions.push_back(std::move(*astFunc));
      } else if (auto astStruct = getStructDef()) {
        found = true;
        prog.structs.push_back(std::move(*astStruct));
      }
    }
    return prog;
  }

 private:
  std::optional<ast::FunctionPtr> getFuncDef() {
    Context context = getContext();
    Token typeTok = context.next();
    if (typeTok.getType() != TokenType::Identifier) {
      return context.notifyFailure("expected function type");
    }

    Token nameTok = context.next();
    if (nameTok.getType() != TokenType::Identifier) {
      return context.notifyFailure("expected function name");
    }

    if (!context.expectNext(TokenType::OpenParen)) {
      return context.notifyFailure("expected open paren after func name");
    }

    for (Token tok = context.next(); tok.getType() != TokenType::CloseParen;
         tok = context.next()) {
      if (tok.isEOF()) {
        return context.notifyFailure("got EOF before close paren");
      }
    }

    return context.notifySuccess(std::make_unique<ast::Function>());
  }

  std::optional<ast::StructDeclPtr> getStructDef() {
    Context context = getContext();
    context.next();
    return context.notifyFailure("not implemented getStructDef()");
  }

  Context getContext() {
    static std::vector<Token> tokenStack;
    static size_t pos{0};
    return Context(tokenizer_, tokenStack, pos);
  }

  Tokenizer tokenizer_;
  std::vector<std::string> messages_;
};

Program parse_file(const char* file_name) {
  /*
   * Check the grammar for some possible issues.
   */

  /*
   * Parse.
   */

  std::ifstream file(file_name);
  if (!file) {
    std::cout << "no file found!\n";
    exit(1);
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  std::cout << "starting...\n";
  Reader reader(ss.str());
  Parser parser(reader);
  Program p = parser.parse();

  DumpAST dump;
  dump.dump_program(p);
  std::cout << "hello?\n";

  exit(1);

  (void)file_name;

  return p;
}
}  // namespace frontend
