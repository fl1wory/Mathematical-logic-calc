#include "LogicController.h"
#include "../core/Lexer.h"
#include "../core/Parser.h"
#include "../core/Evaluator.h"
#include "../core/Simplifier.h"
#include "../core/StepSimplifier.h"
#include <sstream>
#include <iomanip>

LogicController::LogicController(QObject *parent) : QObject(parent) {}

QString LogicController::solveStepByStep(const QString& formula) {
    if (formula.trimmed().isEmpty()) return "Введіть формулу!";

    try {
        auto tokens = Lexer::tokenize(formula.toStdString());
        Parser parser(tokens);
        auto ast = parser.parse();

        auto steps = StepSimplifier::simplify(ast);

        std::stringstream ss;
        if (steps.empty()) {
            ss << "Формула вже максимально спрощена або не підпадає під базові правила.\n";
        } else {
            int i = 1;
            for (const auto& s : steps) {
                ss << "Крок " << i++ << ":\n";
                ss << "  • Правило: " << s.ruleName << "\n";
                ss << "  • Заміна:  " << s.formulaBefore << "  ===>  " << s.formulaAfter << "\n";
                ss << "  • Вигляд:  " << s.fullExpression << "\n";
                ss << "--------------------------------------------------\n";
            }
        }
        ss << "\nФінальний результат: " << ast->toString();
        return QString::fromStdString(ss.str());
    } catch (const std::exception& e) {
        return QString("Помилка: %1").arg(e.what());
    }
}

QString LogicController::solveTruthTable(const QString& formula) {
    if (formula.trimmed().isEmpty()) return "Введіть формулу!";

    try {
        auto tokens = Lexer::tokenize(formula.toStdString());
        Parser parser(tokens);
        auto ast = parser.parse();
        auto variables = Evaluator::extractVariables(tokens);

        TruthTable table = Evaluator::generateTruthTable(ast.get(), variables);

        std::stringstream ss;
        for (const auto& h : table.headers) {
            ss << std::left << std::setw(h.length()) << h << " | ";
        }
        ss << "\n";

        for (const auto& h : table.headers) {
            ss << std::string(h.length() + 3, '-');
        }
        ss << "\n";

        for (const auto& row : table.rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                int width = table.headers[i].length();
                ss << std::left << std::setw(width) << (row[i] ? "1" : "0") << " | ";
            }
            ss << "\n";
        }
        return QString::fromStdString(ss.str());
    } catch (const std::exception& e) {
        return QString("Помилка: %1").arg(e.what());
    }
}

QString LogicController::solveQuine(const QString& formula) {
    if (formula.trimmed().isEmpty()) return "Введіть формулу!";

    try {
        auto tokens = Lexer::tokenize(formula.toStdString());
        Parser parser(tokens);
        auto ast = parser.parse();
        auto variables = Evaluator::extractVariables(tokens);
        auto minterms = Evaluator::getMinterms(ast.get(), variables);

        std::string log;
        std::string simplified = Simplifier::simplify(variables.size(), variables, minterms, log);

        std::stringstream ss;
        ss << log << "\n";
        ss << "====================================\n";
        ss << "Мінімальна ДНФ: " << simplified;
        return QString::fromStdString(ss.str());
    } catch (const std::exception& e) {
        return QString("Помилка: %1").arg(e.what());
    }
}