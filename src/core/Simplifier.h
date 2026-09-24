#pragma once
#include <vector>
#include <string>

class Simplifier {
public:
    // Додали параметр processLog для запису проміжних кроків
    static std::string simplify(int numVars, 
                                const std::vector<std::string>& varNames, 
                                const std::vector<int>& minterms, 
                                std::string& processLog);
};