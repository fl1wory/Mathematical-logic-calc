#pragma once
#include "AST.h"
#include <vector>
#include <string>

// Опис одного виконаного кроку
struct TransformStep {
    std::string ruleName;       // Назва правила (наприклад, "Закон де Моргана")
    std::string formulaBefore;  // Що було замінено
    std::string formulaAfter;   // На що замінено
    std::string fullExpression; // Вигляд усієї формули після перетворення
};

class StepSimplifier {
public:
    // Повністю спрощує вираз та повертає список усіх виконаних кроків
    static std::vector<TransformStep> simplify(std::unique_ptr<ASTNode>& root);

private:
    static bool applyOneRule(std::unique_ptr<ASTNode>& node, TransformStep& step);
};