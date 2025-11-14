#ifndef PASCAL_CODE_GENERATOR_H
#define PASCAL_CODE_GENERATOR_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <regex>
#include <tuple>
#include <iostream>
#include "../Scripts/Lexer.h"

struct NodeInfo {
    int inDegree = 0;
    int outDegree = 0;
    bool hasBackEdge = false; // есть ли стрелка НАЗАД к узлу с меньшим ID
};

class PascalCodeGenerator {
private:
    std::ostringstream out;
    std::map<std::string, NodeInfo> inDegree; // счетчик входящих связей для каждого узла
    std::vector<std::string> nestingStack; // стек открытых блоков
    std::map<std::string, std::string> nodeContents;
    std::set<std::string> processed;
    std::string programName;
    int indentLevel = 0;
    std::string tmpId = "";
    std::vector<std::string> constants;
    std::vector<std::string> variables;
    std::vector<std::pair<std::string, int>> bodyElements;


    std::string extractContent(const std::string &nodeValue) {
        // Извлекаем текст из ["text"], {text}, [[text]], [/text/]
        if (nodeValue.length() < 4) return nodeValue;

        size_t start = nodeValue.find_first_of("[\"({/");
        size_t end = nodeValue.find_last_of("]\")}/");

        if (start != std::string::npos && end != std::string::npos && end > start + 1) {
            std::string content = nodeValue.substr(start + 1, end - start - 1);

            // Для ввода/вывода убираем слэши
            if (content.front() == '/' && content.back() == '/') {
                content = content.substr(1, content.length() - 2);
            }
            // Убираем лишние кавычки если есть
            if (content.front() == '"' && content.back() == '"') {
                content = content.substr(1, content.length() - 2);
            }
            return content;
        }
        return nodeValue;
    }

    bool isMergePoint(const std::string &nodeId) {
        // Узел с >1 входящих связей - merge point
        return (inDegree[nodeId].inDegree > 1) && (!inDegree[nodeId].hasBackEdge);
    }

    bool isCycle(const std::string &nodeId) {
        // Узел с >1 входящих связей - merge point
        return (inDegree[nodeId].inDegree > 1) && (inDegree[nodeId].hasBackEdge);
    }

    void parseFromTokens(const std::vector<Token> &tokens) {
        size_t i = 0;
        const size_t n = tokens.size();

        // Проходим по всем стрелкам и считаем входящие связи
        for (size_t i = 1; i < tokens.size() - 1; i++) {
            if ((tokens[i].getType() == "ARROW_SOLID" || tokens[i].getType() == "CONDITION_TRUE" ||
                 tokens[i].getType() == "CONDITION_FALSE" || tokens[i].getType() == "CONDITION_SWITCH")) {
                std::string to = tokens[i + 1].getValue();
                inDegree[to].inDegree++;
                std::string from = tokens[i - 1].getValue();
                inDegree[from].outDegree++;
                if (extractId(tokens[i - 1]) > extractId(tokens[i + 1])) {
                    inDegree[tokens[i + 1].getValue()].hasBackEdge = true;
                }
            }
        }
        std::cout << "MAPA\n";
        for (auto item: inDegree) {
            std::cout << item.first << " " << item.second.inDegree << " " << item.second.outDegree << " "
                      << item.second.hasBackEdge << std::endl;
        }
        std::cout << "=== PARSING ONLY STATEMENTS ===" << std::endl;

        while (i < n) {
            const Token &token = tokens[i];
            // Ищем NODE_ID
            if (token.getType() == "NODE_ID") {
                std::cout << "Found NODE_ID: " << token.getValue() << std::endl;
                const Token &nextToken = tokens[i + 1];
                if (isMergePoint(token.getValue()) &&
                    ((nextToken.getType() == "NEWFOR") || (nextToken.getType() == "JUSTBLOCK") ||
                     (nextToken.getType() == "INPUT_OUTPUT") || (nextToken.getType() == "DECISION"))) {
                    // Закрываем последний открытый блок
                    indentLevel--;
                    bodyElements.emplace_back("end;", indentLevel);
                    if (tmpId == "else") { //подчищаем за switch
                        indentLevel--;
                        bodyElements.emplace_back("end;", indentLevel);
                        tmpId = "";
                    }
                }
                if(isCycle(token.getValue())){
                    indentLevel--;
                }
                // Смотрим на следующий токен после NODE_ID
                if (i + 1 < n) {
//                    if (nextToken.getType() == "ARROW_SOLID" && (i+2<n)) {
//                        const Token &nextNextToken = tokens[i + 2];
//                    }
                    if (nextToken.getType() == "SUBROUTINE") {
                        std::string content = extractContent(nextToken.getValue());
                        content = content.substr(2, nextToken.getValue().length() - 5 - 1);
                        std::cout << "Content: " << content << std::endl;
                        std::cout << ">>> IDENTIFIED AS FUNCTION/PROCEDURE <<<" << std::endl;
                        constants.emplace_back(content + ";", indentLevel);
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    if (nextToken.getType() == "DECISION") {
                        std::string content = extractContent(nextToken.getValue());
                        std::cout << "Content: " << content << std::endl;
                        std::cout << ">>> IDENTIFIED AS CONDITION <<<" << std::endl;
                        bodyElements.emplace_back(content + " begin", indentLevel);
                        indentLevel++;
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    if (nextToken.getType() == "CONDITION_SWITCH") {
                        std::string content = nextToken.getValue().substr(4, nextToken.getValue().length() - 4 - 1);
                        std::cout << ">>> IDENTIFIED AS BRAIN SWITCH <<<" << std::endl;
                        if (!tmpId.empty()) {
                            indentLevel--;
                            bodyElements.emplace_back("end;", indentLevel);
                        }
                        tmpId = content;
                        std::replace(content.begin(), content.end(), ' ', ',');
                        content += ":";
                        bodyElements.emplace_back(content, indentLevel);
                        bodyElements.emplace_back("begin", indentLevel);
                        indentLevel++;

                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    if (nextToken.getType() == "CONDITION_FALSE") {
                        std::cout << ">>> IDENTIFIED AS UNCONDITION <<<" << std::endl;
                        indentLevel--;
                        bodyElements.emplace_back("else begin", indentLevel);
                        indentLevel++;
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    if (nextToken.getType() == "NEWFOR") {
                        std::string content = nextToken.getValue().substr(3, nextToken.getValue().length() - 5 - 1);;
                        std::cout << "Content: " << content << std::endl;
                        std::cout << ">>> IDENTIFIED AS FOR CYCLE <<<" << std::endl;
                        bodyElements.emplace_back(content, indentLevel);
                        bodyElements.emplace_back("begin", indentLevel);
                        indentLevel++;
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    if (nextToken.getType() == "INPUT_OUTPUT") {
                        std::string content = extractContent(nextToken.getValue());
                        std::cout << "Content: " << content << std::endl;
                        std::cout << ">>> IDENTIFIED AS INPUT/OUTPUT <<<" << std::endl;
                        bodyElements.emplace_back(content + ";", indentLevel);
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                    // Если после NODE_ID идет PROCESS или START_END
                    if (nextToken.getType() == "JUSTBLOCK") {
                        std::string content = extractContent(nextToken.getValue());
                        std::cout << "Content: " << content << std::endl;

                        // Проверяем ТОЛЬКО на константу
                        if (isConstantDeclaration(content)) {
                            std::cout << ">>> IDENTIFIED AS CONSTANT <<<" << std::endl;
                            constants.push_back(content + ";");
                        } else {
                            // Проверяем ТОЛЬКО на объявление
                            if (isVariableDeclaration(content)) {
                                std::cout << ">>> IDENTIFIED AS VARIABLE <<<" << std::endl;
                                variables.push_back(content + ";");
                            } else {
                                //осталось присвоение
                                std::cout << ">>> IDENTIFIED AS ASSIGN <<<" << std::endl;
                                bodyElements.emplace_back(content + ";", indentLevel);
                            }
                        }
                        i++; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                }
            }
            i++; // Переходим к следующему токену
        }

        std::cout << "=== STATEMENTS PARSING COMPLETE ===" << std::endl;
        std::cout << "Constants found: " << constants.size() << std::endl;
        for (const auto &c: constants) {
            std::cout << " - " << c << std::endl;
        }
        std::cout << "Variables found: " << variables.size() << std::endl;
        for (const auto &c: variables) {
            std::cout << " - " << c << std::endl;
        }
        std::cout << "Body elements found: " << bodyElements.size() << std::endl;
        for (const auto &c: bodyElements) {
            for (int i = 0; i < c.second; i++) {
                std::cout << "    ";
            }
            std::cout << " - " << c.first << std::endl;
        }

    }

    static int extractId(const Token &token) {
        return std::stoi(token.getValue().substr(1, token.getValue().length()));
    }

    static bool isConstantDeclaration(const std::string &content) {
        // Константа: содержит "=" и ":"
        // Пример: "PI : real = 3.1415926"
        bool hasAssign = content.find(":=") != std::string::npos;
        bool hasEquals = content.find('=') != std::string::npos;
        bool hasColon = content.find(':') != std::string::npos;

        return hasEquals && hasColon && !hasAssign;
    }

    static bool isVariableDeclaration(const std::string &content) {
        // Переменная: содержит ":" но НЕ содержит "="
        // Пример: "num1 , num2 , i : integer"
        bool hasEquals = content.find('=') != std::string::npos;
        bool hasColon = content.find(':') != std::string::npos;

        return hasColon && !hasEquals;
    }

    std::string convertToPascalStatement(const std::string &content) {
        if (content.find("Read") != std::string::npos) {
            std::regex readRegex(R"(Read\s*\(\s*([^)]+)\s*\))");
            std::smatch match;
            if (std::regex_search(content, match, readRegex)) {
                return "readln(" + match[1].str() + ")";
            }
        } else if (content.find("Writeln") != std::string::npos || content.find("Write") != std::string::npos) {
            std::regex writeRegex(R"((Write|Writeln)\s*\(\s*([^)]+)\s*\))");
            std::smatch match;
            if (std::regex_search(content, match, writeRegex)) {
                return match[1].str() + "(" + match[2].str() + ")";
            }
        }
        // Для присваиваний и других операторов просто возвращаем как есть
        return content;
    }

public:
    PascalCodeGenerator() = default;


    std::string generatePascal(const std::string &code, const std::string &progName) {
        Lexer lexer(code, MERMAID);
        std::vector<Token> tokens = lexer.getTokenList();
        //lexer.printTokenList();
        programName = progName;
        out.str("");
        out.clear();
        nodeContents.clear();
        processed.clear();


        // Заголовок программы
        out << programName << ";\n\n";


        // Парсим структуру из токенов
        parseFromTokens(tokens);


        return out.str();
    }
};

#endif // PASCAL_CODE_GENERATOR_H