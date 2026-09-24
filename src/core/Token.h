#pragma once
#include <string>

// Всі можливі типи символів у дискретній математиці
enum class TokenType {
    VAR,        // Змінна (A, B, C...)
    AND,        // & (Кон'юнкція)
    OR,         // | (Диз'юнкція)
    NOT,        // ! (Заперечення)
    IMPLIES,    // -> (Імплікація)
    EQUIV,      // <-> (Еквівалентність)
    LPAREN,     // (
    RPAREN,     // )
    END         // Кінець рядка
};

// Структура, що зберігає тип і сам текст токена
struct Token {
    TokenType type;
    std::string value;
};