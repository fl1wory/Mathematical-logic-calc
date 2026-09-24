#pragma once
#include <vector>
#include <memory>
#include "Token.h"
#include "AST.h"

class Parser {
    std::vector<Token> tokens;
    size_t pos;

    const Token& current() const;
    void advance();
    void match(TokenType type);

    // Функції для кожного рівня пріоритету (від найнижчого до найвищого)
    std::unique_ptr<ASTNode> parseEquiv();
    std::unique_ptr<ASTNode> parseImplies();
    std::unique_ptr<ASTNode> parseOr();
    std::unique_ptr<ASTNode> parseAnd();
    std::unique_ptr<ASTNode> parseNot();
    std::unique_ptr<ASTNode> parsePrimary();

public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ASTNode> parse();
};