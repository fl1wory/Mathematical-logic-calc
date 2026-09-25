#include "Simplifier.h"
#include <set>
#include <map>
#include <algorithm>
#include <sstream>

std::string toBinaryString(int val, int numVars) {
    std::string res = "";
    for (int i = numVars - 1; i >= 0; --i) {
        res += ((val & (1 << i)) ? '1' : '0');
    }
    return res;
}

bool canMerge(const std::string& a, const std::string& b, std::string& merged) {
    int diffCount = 0;
    merged = a;
    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] != b[i]) {
            diffCount++;
            merged[i] = '-';
        }
    }
    return diffCount == 1;
}

bool coversMinterm(const std::string& pi, int minterm, int numVars) {
    std::string mStr = toBinaryString(minterm, numVars);
    for (size_t i = 0; i < pi.length(); ++i) {
        if (pi[i] != '-' && pi[i] != mStr[i]) return false;
    }
    return true;
}

std::string Simplifier::simplify(int numVars, const std::vector<std::string>& varNames, const std::vector<int>& minterms, std::string& processLog) {
    std::stringstream log;

    if (minterms.empty()) {
        log << "Формула тотожно хибна (немає мінтермів).\n";
        processLog = log.str();
        return "0"; 
    }
    if (minterms.size() == (size_t)(1 << numVars)) {
        log << "Формула тотожно істинна (всі мінтерми дають 1).\n";
        processLog = log.str();
        return "1";
    }

    log << "--- АЛГОРИТМ КУАЙНА-МАК-КЛАСКІ ---\n\n";
    log << "ЕТАП 1: Початкові мінтерми (двійковий вигляд):\n";

    std::set<std::string> currentTerms;
    for (int m : minterms) {
        std::string binStr = toBinaryString(m, numVars);
        currentTerms.insert(binStr);
        log << "  Мінтерм " << m << "\t-> " << binStr << "\n";
    }

    std::set<std::string> primeImplicants;
    bool mergedSomething = true;
    int iteration = 1;

    log << "\nЕТАП 2: Пошук простих імплікант (склеювання):\n";

    while (mergedSomething) {
        mergedSomething = false;
        std::set<std::string> nextTerms;
        std::set<std::string> mergedThisRound;

        std::vector<std::string> termList(currentTerms.begin(), currentTerms.end());
        bool iterHasMerges = false;

        for (size_t i = 0; i < termList.size(); ++i) {
            for (size_t j = i + 1; j < termList.size(); ++j) {
                std::string mergedStr;
                if (canMerge(termList[i], termList[j], mergedStr)) {
                    if (!iterHasMerges) {
                        log << "  Ітерація " << iteration << ":\n";
                        iterHasMerges = true;
                    }
                    if (nextTerms.find(mergedStr) == nextTerms.end()) {
                        log << "    Склеюємо " << termList[i] << " та " << termList[j] << " => " << mergedStr << "\n";
                    }
                    nextTerms.insert(mergedStr);
                    mergedThisRound.insert(termList[i]);
                    mergedThisRound.insert(termList[j]);
                    mergedSomething = true;
                }
            }
        }

        for (const auto& term : currentTerms) {
            if (mergedThisRound.find(term) == mergedThisRound.end()) {
                primeImplicants.insert(term);
            }
        }
        currentTerms = nextTerms;
        iteration++;
    }
    for (const auto& term : currentTerms) primeImplicants.insert(term);

    log << "\n  Знайдені прості імпліканти (ті, що більше не склеюються):\n    ";
    for (const auto& pi : primeImplicants) log << pi << "  ";
    log << "\n\nЕТАП 3: Вибір істотних імплікант та мінімального покриття:\n";

    std::map<int, std::vector<std::string>> covMap;
    for (int m : minterms) {
        for (const auto& pi : primeImplicants) {
            if (coversMinterm(pi, m, numVars)) {
                covMap[m].push_back(pi);
            }
        }
    }

    std::set<int> uncoveredMinterms(minterms.begin(), minterms.end());
    std::vector<std::string> selectedImplicants;
    std::set<std::string> selectedSet;

    // 1. Пошук істотних простих імплікант (EPI)
    for (int m : minterms) {
        if (covMap[m].size() == 1) {
            std::string epi = covMap[m][0];
            if (selectedSet.find(epi) == selectedSet.end()) {
                selectedSet.insert(epi);
                selectedImplicants.push_back(epi);
                log << "  Істотна імпліканта: " << epi << " (єдина покриває мінтерм " << m << ")\n";

                for (int m2 : minterms) {
                    if (coversMinterm(epi, m2, numVars)) {
                        uncoveredMinterms.erase(m2);
                    }
                }
            }
        }
    }

    // 2. Жадібне докриття для решти (якщо залишились)
    while (!uncoveredMinterms.empty()) {
        std::string bestImplicant;
        int maxCovered = -1;
        std::vector<int> bestCoveredMinterms;

        for (const auto& pi : primeImplicants) {
            if (selectedSet.find(pi) != selectedSet.end()) continue;

            std::vector<int> covered;
            for (int m : uncoveredMinterms) {
                if (coversMinterm(pi, m, numVars)) covered.push_back(m);
            }

            if ((int)covered.size() > maxCovered) {
                maxCovered = covered.size();
                bestImplicant = pi;
                bestCoveredMinterms = covered;
            }
        }

        selectedSet.insert(bestImplicant);
        selectedImplicants.push_back(bestImplicant);
        log << "  Додатково вибрано: " << bestImplicant << " (покриває мінтерми: ";
        for (size_t i = 0; i < bestCoveredMinterms.size(); ++i) {
            log << bestCoveredMinterms[i] << (i == bestCoveredMinterms.size() - 1 ? "" : ", ");
            uncoveredMinterms.erase(bestCoveredMinterms[i]);
        }
        log << ")\n";
    }

    log << "\nЕТАП 4: Перетворення назад у змінні:\n";

    std::string result = "";
    for (size_t i = 0; i < selectedImplicants.size(); ++i) {
        std::string term = "";
        const auto& pi = selectedImplicants[i];
        for (size_t j = 0; j < pi.length(); ++j) {
            if (pi[j] == '1') {
                if (!term.empty()) term += " & ";
                term += varNames[j];
            } else if (pi[j] == '0') {
                if (!term.empty()) term += " & ";
                term += "!" + varNames[j];
            }
        }
        
        if (selectedImplicants.size() > 1 && pi.find('-') != std::string::npos && std::count(pi.begin(), pi.end(), '-') < (int)pi.length() - 1) {
            term = "(" + term + ")";
        } 
        
        log << "  " << pi << " -> " << term << "\n";
        result += term;
        if (i < selectedImplicants.size() - 1) result += " | ";
    }

    processLog = log.str();
    return result;
}