#include "frontend/parse/new_parser.h"
#include "frontend/diagnostic/debug.h"
#include "frontend/types/VarType.h"
#include <sstream>

namespace frontend {
namespace parser {

RecursiveDescentParser::RecursiveDescentParser(lexer::Lexer& lexer)
    : lexer_(lexer), current_token_(lexer_.nextToken()), current_functions_(nullptr) {}

void RecursiveDescentParser::error(const std::string& message) {
    std::stringstream ss;
    ss << "Parser error at line " << current_token_.line 
       << ", column " << current_token_.column << ": " << message;
    FRONTEND_ERROR(ss.str());
}

void RecursiveDescentParser::expect(lexer::TokenType expected) {
    if (current_token_.type != expected) {
        std::stringstream ss;
        ss << "Expected token type " << static_cast<int>(expected) 
           << " but got " << static_cast<int>(current_token_.type);
        error(ss.str());
    }
    advance();
}

bool RecursiveDescentParser::match(lexer::TokenType type) {
    if (current_token_.type == type) {
        advance();
        return true;
    }
    return false;
}

void RecursiveDescentParser::advance() {
    current_token_ = lexer_.nextToken();
}

RecursiveDescentParser::LexerState RecursiveDescentParser::saveLexerState() {
    return {lexer_.pos_, lexer_.line_, lexer_.column_};
}

void RecursiveDescentParser::restoreLexerState(const LexerState& state) {
    lexer_.pos_ = state.pos;
    lexer_.line_ = state.line;
    lexer_.column_ = state.column;
    current_token_ = lexer_.nextToken();
}

Program RecursiveDescentParser::parseProgram() {
    Program program;
    current_functions_ = &program.functions;
    
    while (current_token_.type != lexer::TokenType::END_OF_FILE) {
        if (current_token_.type == lexer::TokenType::STRUCT) {
            program.structs.push_back(parseStruct());
        } else if (isTypeToken()) {
            program.functions.push_back(parseFunction());
        } else {
            error("Expected struct or function definition");
        }
    }
    
    return program;
}

bool RecursiveDescentParser::isTypeToken() const {
    return current_token_.type == lexer::TokenType::IDENTIFIER;
}

std::unique_ptr<ast::Function> RecursiveDescentParser::parseFunction() {
    auto function = std::make_unique<ast::Function>();
    
    // Parse return type
    function->type = parseType();
    
    // Parse function name
    expect(lexer::TokenType::IDENTIFIER);
    function->name = current_token_.text;
    
    // Parse parameters
    expect(lexer::TokenType::LEFT_PAREN);
    if (current_token_.type != lexer::TokenType::RIGHT_PAREN) {
        function->args = parseFunctionDefinitionArguments();
    }
    expect(lexer::TokenType::RIGHT_PAREN);
    
    // Parse function body
    function->scope = parseScope();
    
    return function;
}

std::unique_ptr<ast::StructDecl> RecursiveDescentParser::parseStruct() {
    auto struct_decl = std::make_unique<ast::StructDecl>();
    
    expect(lexer::TokenType::STRUCT);
    expect(lexer::TokenType::IDENTIFIER);
    struct_decl->name = current_token_.text;
    
    expect(lexer::TokenType::LEFT_BRACE);
    
    int member_index = 0;
    while (current_token_.type != lexer::TokenType::RIGHT_BRACE) {
        ConstVarTypePtr member_type = parseType();
        expect(lexer::TokenType::IDENTIFIER);
        std::string member_name = current_token_.text;
        
        struct_decl->member_types.push_back(member_type);
        struct_decl->member_name_to_index[member_name] = member_index++;
    }
    
    expect(lexer::TokenType::RIGHT_BRACE);
    
    struct_decl->type = VarType::getStructType(
        struct_decl->name, 
        struct_decl->member_types,
        struct_decl->member_name_to_index
    );
    
    return struct_decl;
}

ConstVarTypePtr RecursiveDescentParser::parseType() {
    ConstVarTypePtr base_type = parseBasicType();
    
    // Check for array type
    if (current_token_.type == lexer::TokenType::LEFT_BRACKET) {
        return parseArrayType();
    }
    
    // Check for reference type
    if (current_token_.type == lexer::TokenType::AMPERSAND) {
        advance();
        return base_type->getRefTypeFrom();
    }
    
    return base_type;
}

ConstVarTypePtr RecursiveDescentParser::parseBasicType() {
    expect(lexer::TokenType::IDENTIFIER);
    std::string type_name = current_token_.text;
    
    // Handle template types like int64<type>
    while (current_token_.type == lexer::TokenType::LESS_THAN) {
        advance();
        ConstVarTypePtr template_arg = parseType();
        expect(lexer::TokenType::GREATER_THAN);
        type_name += "<" + template_arg->getTypeName() + ">";
    }
    
    return VarType::findTypeByName(type_name);
}

ConstVarTypePtr RecursiveDescentParser::parseArrayType() {
    ConstVarTypePtr elem_type = parseBasicType();
    
    expect(lexer::TokenType::LEFT_BRACKET);
    auto size_expr = parseExpression();
    expect(lexer::TokenType::RIGHT_BRACKET);
    
    // Extract size from Integer literal
    auto* size_int = dynamic_cast<const ast::Integer*>(size_expr.get());
    if (!size_int) {
        error("Array size must be a constant integer");
    }
    
    std::string type_name = elem_type->getTypeName() + "[" + std::to_string(size_int->value) + "]";
    return VarType::getArrayType(type_name, 1, size_int->value, elem_type);
}

std::unique_ptr<ast::Scope> RecursiveDescentParser::parseScope() {
    auto scope = std::make_unique<ast::Scope>();
    
    expect(lexer::TokenType::LEFT_BRACE);
    
    while (current_token_.type != lexer::TokenType::RIGHT_BRACE) {
        scope->instructions.push_back(parseInstruction());
    }
    
    expect(lexer::TokenType::RIGHT_BRACE);
    return scope;
}

std::unique_ptr<ast::Instruction> RecursiveDescentParser::parseInstruction() {
    switch (current_token_.type) {
        case lexer::TokenType::RETURN:
            return parseReturn();
        case lexer::TokenType::WHILE:
            return parseWhile();
        case lexer::TokenType::IF:
            return parseIf();
        case lexer::TokenType::BREAK:
            return parseBreak();
        case lexer::TokenType::CONTINUE:
            return parseContinue();
        case lexer::TokenType::LEFT_BRACE:
            return parseScope();
        case lexer::TokenType::IDENTIFIER:
            {
                // Look ahead to see if this is a type (variable declaration), assignment, or function call
                auto next = lexer_.peekToken();
                
                // Check for function call first (identifier followed by '(')
                if (next.type == lexer::TokenType::LEFT_PAREN) {
                    return parseFunctionCallInstruction();
                }
                
                // Check for assignment (identifier followed by '=' or '[')
                if (next.type == lexer::TokenType::ASSIGN || next.type == lexer::TokenType::LEFT_BRACKET) {
                    return parseAssignment();
                }
                
                // Must be a variable declaration if it's a type name followed by identifier
                // This is a simple heuristic - assume it's a type if followed by another identifier
                if (next.type == lexer::TokenType::IDENTIFIER) {
                    return parseVariableDeclaration();
                }
                
                error("Unexpected identifier in instruction context");
                return nullptr;
            }
        default:
            error("Unexpected token in instruction");
            return nullptr;
    }
}

std::unique_ptr<ast::InstructionReturn> RecursiveDescentParser::parseReturn() {
    expect(lexer::TokenType::RETURN);
    
    if (current_token_.type == lexer::TokenType::RIGHT_BRACE ||
        current_token_.type == lexer::TokenType::LEFT_BRACE) {
        // Return void
        return std::make_unique<ast::InstructionReturn>();
    }
    
    auto expr = parseExpression();
    return std::make_unique<ast::InstructionReturn>(std::move(expr));
}

std::unique_ptr<ast::InstructionAssignment> RecursiveDescentParser::parseAssignment() {
    auto lhs = parseExpression(); // This handles variable or array access
    expect(lexer::TokenType::ASSIGN);
    auto rhs = parseExpression();
    return std::make_unique<ast::InstructionAssignment>(std::move(lhs), std::move(rhs));
}

std::unique_ptr<ast::InstructionDecl> RecursiveDescentParser::parseVariableDeclaration() {
    ConstVarTypePtr var_type = parseType();
    auto variables = parseVariableDeclarationList();
    
    // Set type for all variables
    for (auto& var : variables) {
        auto* var_ptr = dynamic_cast<ast::Variable*>(var.get());
        if (var_ptr) {
            var_ptr->type = var_type->getLValueFrom();
        }
    }
    
    return std::make_unique<ast::InstructionDecl>(std::move(variables));
}

std::unique_ptr<ast::InstructionFunctionCall> RecursiveDescentParser::parseFunctionCallInstruction() {
    auto func_call = parseFunctionCall();
    return std::make_unique<ast::InstructionFunctionCall>(std::move(func_call));
}

std::unique_ptr<ast::InstructionWhileLoop> RecursiveDescentParser::parseWhile() {
    expect(lexer::TokenType::WHILE);
    expect(lexer::TokenType::LEFT_PAREN);
    auto condition = parseExpression();
    expect(lexer::TokenType::RIGHT_PAREN);
    auto body = parseScope();
    return std::make_unique<ast::InstructionWhileLoop>(std::move(condition), std::move(body));
}

std::unique_ptr<ast::InstructionIfStatement> RecursiveDescentParser::parseIf() {
    expect(lexer::TokenType::IF);
    expect(lexer::TokenType::LEFT_PAREN);
    auto condition = parseExpression();
    expect(lexer::TokenType::RIGHT_PAREN);
    auto true_scope = parseScope();
    return std::make_unique<ast::InstructionIfStatement>(std::move(condition), std::move(true_scope));
}

std::unique_ptr<ast::InstructionBreak> RecursiveDescentParser::parseBreak() {
    expect(lexer::TokenType::BREAK);
    return std::make_unique<ast::InstructionBreak>();
}

std::unique_ptr<ast::InstructionContinue> RecursiveDescentParser::parseContinue() {
    expect(lexer::TokenType::CONTINUE);
    return std::make_unique<ast::InstructionContinue>();
}

ast::ConstValuePtr RecursiveDescentParser::parseExpression() {
    return parseBinaryOperation();
}

ast::ConstValuePtr RecursiveDescentParser::parseBinaryOperation() {
    auto left = parseSingleExpression();
    
    while (isBinaryOperator()) {
        ast::BinOpId op = parseBinaryOperator();
        auto right = parseSingleExpression();
        left = std::make_shared<ast::BinaryOperation>(op, std::move(left), std::move(right));
    }
    
    return left;
}

ast::ConstValuePtr RecursiveDescentParser::parseSingleExpression() {
    switch (current_token_.type) {
        case lexer::TokenType::NUMBER:
            return parseNumber();
        case lexer::TokenType::IDENTIFIER:
            // Look ahead to determine if it's variable, function call, or array access
            {
                auto next = lexer_.peekToken();
                if (next.type == lexer::TokenType::LEFT_PAREN) {
                    return parseFunctionCall();
                } else if (next.type == lexer::TokenType::LEFT_BRACKET) {
                    return parseArrayAccess();
                } else {
                    return parseVariable();
                }
            }
        case lexer::TokenType::PRINT:
        case lexer::TokenType::INPUT:
            return parseFunctionCall();
        case lexer::TokenType::LEFT_BRACKET:
            return parseArrayAllocate();
        default:
            error("Expected expression");
            return nullptr;
    }
}

ast::ConstValuePtr RecursiveDescentParser::parseVariable() {
    expect(lexer::TokenType::IDENTIFIER);
    std::string name = current_token_.text;
    
    // Create variable with null function context for simplicity - this will be fixed up later
    auto var = ast::Variable::get(name, nullptr);
    var->name = name;
    return std::move(var);
}

ast::ConstValuePtr RecursiveDescentParser::parseNumber() {
    expect(lexer::TokenType::NUMBER);
    std::string text = current_token_.text;
    int64_t value = std::stoll(text);
    return std::make_shared<ast::Integer>(value);
}

ast::ConstValuePtr RecursiveDescentParser::parseFunctionCall() {
    ast::ConstValuePtr function;
    
    if (current_token_.type == lexer::TokenType::PRINT || 
        current_token_.type == lexer::TokenType::INPUT) {
        std::string func_name = current_token_.text;
        advance();
        function = std::make_shared<ast::FunctionName>(std::move(func_name), nullptr);
    } else {
        expect(lexer::TokenType::IDENTIFIER);
        std::string func_name = current_token_.text;
        
        // Find function in current program
        ConstVarTypePtr func_type = nullptr;
        if (current_functions_) {
            for (const auto& f : *current_functions_) {
                if (f->name == func_name) {
                    func_type = f->type;
                    break;
                }
            }
        }
        
        function = std::make_shared<ast::FunctionName>(std::move(func_name), func_type);
    }
    
    expect(lexer::TokenType::LEFT_PAREN);
    auto args = parseFunctionArguments();
    expect(lexer::TokenType::RIGHT_PAREN);
    
    auto func_call = std::make_shared<ast::FunctionCall>(std::move(function), std::move(args));
    
    // Set arg types from function definition
    auto* func_name = dynamic_cast<const ast::FunctionName*>(func_call->function.get());
    if (func_name && current_functions_) {
        for (const auto& f : *current_functions_) {
            if (f->name == func_name->name) {
                for (const auto& param : f->args) {
                    func_call->arg_types.push_back(param->type);
                }
                break;
            }
        }
    }
    
    return func_call;
}

ast::ConstValuePtr RecursiveDescentParser::parseArrayAccess() {
    auto var = parseVariable();
    std::vector<ast::ConstValuePtr> indices;
    
    while (current_token_.type == lexer::TokenType::LEFT_BRACKET) {
        advance();
        indices.push_back(parseExpression());
        expect(lexer::TokenType::RIGHT_BRACKET);
    }
    
    return std::make_shared<ast::ArrayAccess>(std::move(var), std::move(indices), current_token_.line);
}

ast::ConstValuePtr RecursiveDescentParser::parseArrayAllocate() {
    expect(lexer::TokenType::LEFT_BRACKET);
    auto init_value = parseExpression();
    expect(lexer::TokenType::SEMICOLON);
    auto length = parseExpression();
    expect(lexer::TokenType::RIGHT_BRACKET);
    
    return std::make_shared<ast::ArrayAllocate>(std::move(length), std::move(init_value));
}

std::vector<ast::ConstValuePtr> RecursiveDescentParser::parseFunctionArguments() {
    std::vector<ast::ConstValuePtr> args;
    
    if (current_token_.type != lexer::TokenType::RIGHT_PAREN) {
        args.push_back(parseExpression());
        
        while (current_token_.type == lexer::TokenType::COMMA) {
            advance();
            args.push_back(parseExpression());
        }
    }
    
    return args;
}

std::vector<ast::ConstValuePtr> RecursiveDescentParser::parseFunctionDefinitionArguments() {
    std::vector<ast::ConstValuePtr> args;
    
    do {
        ConstVarTypePtr arg_type = parseType();
        expect(lexer::TokenType::IDENTIFIER);
        std::string arg_name = current_token_.text;
        
        auto var = ast::Variable::get(arg_name, nullptr);
        var->name = arg_name;
        var->type = std::move(arg_type);
        args.push_back(std::move(var));
        
    } while (match(lexer::TokenType::COMMA));
    
    return args;
}

std::vector<ast::ValuePtr> RecursiveDescentParser::parseVariableDeclarationList() {
    std::vector<ast::ValuePtr> vars;
    
    do {
        expect(lexer::TokenType::IDENTIFIER);
        std::string var_name = current_token_.text;
        
        auto var = ast::Variable::get(var_name, nullptr);
        var->name = var_name;
        vars.push_back(std::move(var));
        
    } while (match(lexer::TokenType::COMMA));
    
    return vars;
}

ast::BinOpId RecursiveDescentParser::parseBinaryOperator() {
    switch (current_token_.type) {
        case lexer::TokenType::PLUS:
            advance();
            return ast::BinOpId::ADD;
        case lexer::TokenType::MINUS:
            advance();
            return ast::BinOpId::SUB;
        case lexer::TokenType::MULTIPLY:
            advance();
            return ast::BinOpId::MUL;
        case lexer::TokenType::AMPERSAND:
            advance();
            return ast::BinOpId::AND;
        case lexer::TokenType::LEFT_SHIFT:
            advance();
            return ast::BinOpId::SHL;
        case lexer::TokenType::RIGHT_SHIFT:
            advance();
            return ast::BinOpId::SHR;
        case lexer::TokenType::LESS_THAN:
            advance();
            return ast::BinOpId::LT;
        case lexer::TokenType::GREATER_THAN:
            advance();
            return ast::BinOpId::GT;
        case lexer::TokenType::LESS_EQUAL:
            advance();
            return ast::BinOpId::LEQ;
        case lexer::TokenType::GREATER_EQUAL:
            advance();
            return ast::BinOpId::GEQ;
        case lexer::TokenType::EQUAL:
            advance();
            return ast::BinOpId::EQ;
        default:
            error("Expected binary operator");
            return ast::BinOpId::NONE;
    }
}

bool RecursiveDescentParser::isBinaryOperator() const {
    return current_token_.type == lexer::TokenType::PLUS ||
           current_token_.type == lexer::TokenType::MINUS ||
           current_token_.type == lexer::TokenType::MULTIPLY ||
           current_token_.type == lexer::TokenType::AMPERSAND ||
           current_token_.type == lexer::TokenType::LEFT_SHIFT ||
           current_token_.type == lexer::TokenType::RIGHT_SHIFT ||
           current_token_.type == lexer::TokenType::LESS_THAN ||
           current_token_.type == lexer::TokenType::GREATER_THAN ||
           current_token_.type == lexer::TokenType::LESS_EQUAL ||
           current_token_.type == lexer::TokenType::GREATER_EQUAL ||
           current_token_.type == lexer::TokenType::EQUAL;
}

bool RecursiveDescentParser::isStartOfInstruction() const {
    return current_token_.type == lexer::TokenType::RETURN ||
           current_token_.type == lexer::TokenType::WHILE ||
           current_token_.type == lexer::TokenType::IF ||
           current_token_.type == lexer::TokenType::BREAK ||
           current_token_.type == lexer::TokenType::CONTINUE ||
           current_token_.type == lexer::TokenType::LEFT_BRACE ||
           current_token_.type == lexer::TokenType::IDENTIFIER;
}

bool RecursiveDescentParser::isStartOfExpression() const {
    return current_token_.type == lexer::TokenType::NUMBER ||
           current_token_.type == lexer::TokenType::IDENTIFIER ||
           current_token_.type == lexer::TokenType::PRINT ||
           current_token_.type == lexer::TokenType::INPUT ||
           current_token_.type == lexer::TokenType::LEFT_BRACKET;
}

} // namespace parser
} // namespace frontend