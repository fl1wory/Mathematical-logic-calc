#pragma once
#include <vector>
#include <string>
#include <set>
#include "AST.h"
#include "Token.h"

// Структура, що представляє готову таблицю істинності
struct TruthTable {
    std::vector<std::string> headers;         // Заголовки (змінні + кроки)
    std::vector<std::vector<bool>> rows;      // Рядки з нулями та одиничками
};

class Evaluator {
public:
    static std::vector<std::string> extractVariables(const std::vector<Token>& tokens);
    static std::vector<int> getMinterms(const ASTNode* ast, const std::vector<std::string>& variables);
    
    // Нова функція для генерації повної таблиці
    static TruthTable generateTruthTable(const ASTNode* ast, const std::vector<std::string>& variables);
};