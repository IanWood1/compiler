#pragma once

#include "frontend/parse/lexer.h"
#include "frontend/ast/ast.h"
#include <memory>
#include <vector>

namespace frontend {
namespace parser {

class RecursiveDescentParser {
public:
    explicit RecursiveDescentParser(lexer::Lexer& lexer);
    
    Program parseProgram();
    
private:
    lexer::Lexer& lexer_;
    mutable lexer::Token current_token_;
    std::vector<std::unique_ptr<ast::Function>>* current_functions_;
    
    // Store original lexer state for lookahead
    struct LexerState {
        size_t pos;
        size_t line;
        size_t column;
    };
    
    LexerState saveLexerState() const;
    void restoreLexerState(const LexerState& state) const;
    
    // Error handling
    void error(const std::string& message);
    void expect(lexer::TokenType expected);
    bool match(lexer::TokenType type);
    void advance() const;
    
    // Parsing methods
    std::unique_ptr<ast::Function> parseFunction();
    std::unique_ptr<ast::StructDecl> parseStruct();
    ConstVarTypePtr parseType();
    ConstVarTypePtr parseBasicType();
    ConstVarTypePtr parseArrayType();
    ConstVarTypePtr parseReferenceType();
    
    std::unique_ptr<ast::Scope> parseScope();
    std::unique_ptr<ast::Instruction> parseInstruction();
    std::unique_ptr<ast::InstructionReturn> parseReturn();
    std::unique_ptr<ast::InstructionAssignment> parseAssignment();
    std::unique_ptr<ast::InstructionDecl> parseVariableDeclaration();
    std::unique_ptr<ast::InstructionFunctionCall> parseFunctionCallInstruction();
    std::unique_ptr<ast::InstructionWhileLoop> parseWhile();
    std::unique_ptr<ast::InstructionIfStatement> parseIf();
    std::unique_ptr<ast::InstructionBreak> parseBreak();
    std::unique_ptr<ast::InstructionContinue> parseContinue();
    
    ast::ConstValuePtr parseExpression();
    ast::ConstValuePtr parseBinaryOperation();
    ast::ConstValuePtr parseSingleExpression();
    ast::ConstValuePtr parseVariable();
    ast::ConstValuePtr parseNumber();
    ast::ConstValuePtr parseFunctionCall();
    ast::ConstValuePtr parseArrayAccess();
    ast::ConstValuePtr parseArrayAllocate();
    
    std::vector<ast::ConstValuePtr> parseFunctionArguments();
    std::vector<ast::ConstValuePtr> parseFunctionDefinitionArguments();
    std::vector<ast::ValuePtr> parseVariableDeclarationList();
    
    ast::BinOpId parseBinaryOperator();
    
    // Helper methods
    bool isTypeToken() const;
    bool isBinaryOperator() const;
    bool isStartOfInstruction() const;
    bool isStartOfExpression() const;
    lexer::TokenType lookaheadAfterBrackets() const;
};

} // namespace parser
} // namespace frontend