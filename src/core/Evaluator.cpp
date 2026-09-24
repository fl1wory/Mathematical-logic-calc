#include "Evaluator.h"

std::vector<std::string> Evaluator::extractVariables(const std::vector<Token>& tokens) {
    std::set<std::string> vars;
    // Шукаємо всі токени типу VAR і додаємо їх у set (він автоматично відкине дублікати і відсортує за алфавітом)
    for (const auto& t : tokens) {
        if (t.type == TokenType::VAR) {
            vars.insert(t.value);
        }
    }
    return std::vector<std::string>(vars.begin(), vars.end());
}

std::vector<int> Evaluator::getMinterms(const ASTNode* ast, const std::vector<std::string>& variables) {
    std::vector<int> minterms;
    int numVars = variables.size();
    
    // Кількість рядків у таблиці істинності це 2^N (використовуємо бітовий зсув 1 << N)
    int numRows = 1 << numVars; 

    for (int i = 0; i < numRows; ++i) {
        std::map<std::string, bool> varValues;
        
        // Заповнюємо значення змінних на основі бітів числа i
        for (int j = 0; j < numVars; ++j) {
            // Витягуємо j-тий біт (де перша змінна - це найстарший біт)
            bool bitValue = (i & (1 << (numVars - 1 - j))) != 0; 
            varValues[variables[j]] = bitValue;
        }

        // Якщо при цих значеннях формула дає 1 (true), запам'ятовуємо номер рядка (i)
        if (ast->evaluate(varValues)) {
            minterms.push_back(i);
        }
    }
    
    return minterms;
}

TruthTable Evaluator::generateTruthTable(const ASTNode* ast, const std::vector<std::string>& variables) {
    TruthTable table;
    
    // 1. Формуємо заголовки (спочатку A, B, C, потім проміжні дії)
    table.headers = variables;
    std::vector<std::string> stepHeaders;
    ast->getHeaders(stepHeaders);
    for (const auto& h : stepHeaders) {
        table.headers.push_back(h);
    }

    // 2. Генеруємо рядки таблиці
    int numVars = variables.size();
    int numRows = 1 << numVars;

    for (int i = 0; i < numRows; ++i) {
        std::map<std::string, bool> varValues;
        std::vector<bool> row;

        // Записуємо значення самих змінних (0 або 1)
        for (int j = 0; j < numVars; ++j) {
            bool val = (i & (1 << (numVars - 1 - j))) != 0;
            varValues[variables[j]] = val;
            row.push_back(val);
        }

        // Обчислюємо проміжні кроки (вони запишуться в map `stepResults`)
        std::map<std::string, bool> stepResults;
        ast->evaluateSteps(varValues, stepResults);

        // Додаємо результати проміжних кроків у наш рядок
        for (const auto& h : stepHeaders) {
            row.push_back(stepResults[h]);
        }

        table.rows.push_back(row);
    }

    return table;
}