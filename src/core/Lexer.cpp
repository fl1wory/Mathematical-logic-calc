#include "Lexer.h"
#include <cctype>
#include <stdexcept>

std::vector<Token> Lexer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length()) {
        char c = input[i];

        // Пропускаємо пробіли
        if (std::isspace(c)) {
            i++;
            continue;
        }

        // Змінні (великі та малі літери англійського алфавіту)
        if (std::isalpha(c)) {
            std::string varName(1, c);
            tokens.push_back({TokenType::VAR, varName});
            i++;
        }
        // Логічне І
        else if (c == '&') {
            tokens.push_back({TokenType::AND, "&"});
            i++;
        }
        // Логічне АБО
        else if (c == '|') {
            tokens.push_back({TokenType::OR, "|"});
            i++;
        }
        // Заперечення
        else if (c == '!') {
            tokens.push_back({TokenType::NOT, "!"});
            i++;
        }
        // Дужки
        else if (c == '(') {
            tokens.push_back({TokenType::LPAREN, "("});
            i++;
        }
        else if (c == ')') {
            tokens.push_back({TokenType::RPAREN, ")"});
            i++;
        }
        // Імплікація (->)
        else if (c == '-' && i + 1 < input.length() && input[i+1] == '>') {
            tokens.push_back({TokenType::IMPLIES, "->"});
            i += 2;
        }
        // Еквівалентність (<->)
        else if (c == '<' && i + 2 < input.length() && input[i+1] == '-' && input[i+2] == '>') {
            tokens.push_back({TokenType::EQUIV, "<->"});
            i += 3;
        }
        // Якщо символ невідомий - кидаємо помилку
        else {
            throw std::runtime_error(std::string("Невідомий символ: ") + c);
        }
    }
    
    tokens.push_back({TokenType::END, ""});
    return tokens;
}