#include "Parser.h"

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

const Token& Parser::current() const {
    return tokens[pos];
}

void Parser::advance() {
    if (pos < tokens.size() - 1) pos++;
}

void Parser::match(TokenType type) {
    if (current().type == type) {
        advance();
    } else {
        throw std::runtime_error("Синтаксична помилка: неочікуваний токен '" + current().value + "'");
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    return parseEquiv();
}

std::unique_ptr<ASTNode> Parser::parseEquiv() {
    auto node = parseImplies();
    while (current().type == TokenType::EQUIV) {
        advance();
        node = std::make_unique<EquivNode>(std::move(node), parseImplies());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseImplies() {
    auto node = parseOr();
    while (current().type == TokenType::IMPLIES) {
        advance();
        node = std::make_unique<ImpliesNode>(std::move(node), parseOr());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseOr() {
    auto node = parseAnd();
    while (current().type == TokenType::OR) {
        advance();
        node = std::make_unique<OrNode>(std::move(node), parseAnd());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseAnd() {
    auto node = parseNot();
    while (current().type == TokenType::AND) {
        advance();
        node = std::make_unique<AndNode>(std::move(node), parseNot());
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parseNot() {
    if (current().type == TokenType::NOT) {
        advance();
        return std::make_unique<NotNode>(parseNot());
    }
    return parsePrimary();
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (current().type == TokenType::VAR) {
        auto node = std::make_unique<VarNode>(current().value);
        advance();
        return node;
    } 
    if (current().type == TokenType::LPAREN) {
        advance();
        auto node = parse(); // Рекурсивно парсимо те, що всередині дужок
        match(TokenType::RPAREN);
        return node;
    }
    throw std::runtime_error("Синтаксична помилка: очікувалась змінна або '('");
}