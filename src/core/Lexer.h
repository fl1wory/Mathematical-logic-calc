#pragma once
#include <string>
#include <vector>
#include "Token.h"

class Lexer {
public:
    // Головна функція: приймає рядок, повертає список токенів
    static std::vector<Token> tokenize(const std::string& input);
};