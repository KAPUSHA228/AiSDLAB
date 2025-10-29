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
#include "../Lexer.h"  // Предполагаем, что у тебя есть Lexer

class PascalCodeGenerator {
private:
    std::ostringstream out;
    std::map<std::string, std::string> nodeContents;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> graph; // node -> [(target, label)]
    std::set<std::string> processed;
    std::string programName;
    int indentLevel = 0;
    std::vector<std::string> constants;
    std::vector<std::string> variables;

    void indent() {
        for (int i = 0; i < indentLevel; ++i) out << "    ";
    }

    std::string extractContent(const std::string& nodeValue) {
        // Извлекаем текст из ["text"], {text}, [[text]], [/text/]
        if (nodeValue.length() < 4) return nodeValue;

        size_t start = nodeValue.find_first_of("[\"({/");
        size_t end = nodeValue.find_last_of("]\")}/");

        if (start != std::string::npos && end != std::string::npos && end > start + 1) {
            std::string content = nodeValue.substr(start + 1, end - start - 1);
            // Убираем лишние кавычки если есть
            if (content.front() == '"' && content.back() == '"') {
                content = content.substr(1, content.length() - 2);
            }
            // Для ввода/вывода убираем слэши
            if (nodeValue[start] == '/' && nodeValue[end] == '/') {
                return content;
            }
            return content;
        }
        return nodeValue;
    }

    void parseFromTokens(const std::vector<Token>& tokens) {
        size_t i = 0;
        const size_t n = tokens.size();

        std::cout << "=== PARSING ONLY CONSTANTS ===" << std::endl;

        while (i < n) {
            const Token& token = tokens[i];

            // Ищем NODE_ID
            if (token.getType() == "NODE_ID") {
                std::cout << "Found NODE_ID: " << token.getValue() << std::endl;

                // Смотрим на следующий токен после NODE_ID
                if (i + 2 < n) {
                    const Token& nextToken = tokens[i +2];

                    // Если после NODE_ID идет PROCESS или START_END
                    if (tokens[i + 2].getType() == "QUOTED_STRING") {
                        std::string content = extractContent(nextToken.getValue());
                        std::cout << "Content: " << content << std::endl;

                        // Проверяем ТОЛЬКО на константу
                        if (isConstantDeclaration(content)) {
                            std::cout << ">>> IDENTIFIED AS CONSTANT <<<" << std::endl;
                            constants.push_back(content);
                        }
                        // Проверяем ТОЛЬКО на константу
                        if (isVariableDeclaration(content)) {
                            std::cout << ">>> IDENTIFIED AS VARIABLE <<<" << std::endl;
                            variables.push_back(content);
                        }
                        i += 2; // Пропускаем NODE_ID и его содержимое
                        continue;
                    }
                }
            }

            i++; // Переходим к следующему токену
        }

        std::cout << "=== CONSTANTS PARSING COMPLETE ===" << std::endl;
        std::cout << "Constants found: " << constants.size() << std::endl;
        for (const auto& c : constants) {
            std::cout << " - " << c << std::endl;
        }
        std::cout << "Variables found: " << variables.size() << std::endl;
        for (const auto& c : variables) {
            std::cout << " - " << c << std::endl;
        }

    }

    bool isConstantDeclaration(const std::string& content) {
        // Константа: содержит "=" и ":"
        // Пример: "PI : real = 3.1415926"
        bool hasAssign = content.find(":=") != std::string::npos;
        bool hasEquals = content.find('=') != std::string::npos;
        bool hasColon = content.find(':') != std::string::npos;

        return hasEquals && hasColon &&!hasAssign;
    }
    bool isVariableDeclaration(const std::string& content) {
        // Переменная: содержит ":" но НЕ содержит "="
        // Пример: "num1 , num2 , i : integer"
        bool hasEquals = content.find('=') != std::string::npos;
        bool hasColon = content.find(':') != std::string::npos;

        return hasColon && !hasEquals;
    }
    std::string convertToPascalStatement(const std::string& content) {
        if (content.find("Read") != std::string::npos) {
            std::regex readRegex(R"(Read\s*\(\s*([^)]+)\s*\))");
            std::smatch match;
            if (std::regex_search(content, match, readRegex)) {
                return "readln(" + match[1].str() + ")";
            }
        }
        else if (content.find("Writeln") != std::string::npos || content.find("Write") != std::string::npos) {
            std::regex writeRegex(R"((Write|Writeln)\s*\(\s*([^)]+)\s*\))");
            std::smatch match;
            if (std::regex_search(content, match, writeRegex)) {
                return match[1].str() + "(" + match[2].str() + ")";
            }
        }
        // Для присваиваний и других операторов просто возвращаем как есть
        return content;
    }

    void generateDeclarations() {
        std::set<std::string> functions;
        std::set<std::string> procedures;
        std::set<std::string> constants;
        std::set<std::string> variables;

        for (const auto& [id, content] : nodeContents) {
            std::string lowerContent = content;
            std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

            if (content.find("function") == 0) {
                functions.insert(content);
            }
            else if (content.find("procedure") == 0) {
                procedures.insert(content);
            }
            else if (content.find(":=") != std::string::npos && content.find(":") != std::string::npos) {
                // Константа: name : type = value
                constants.insert(content);
            }
            else if (content.find(":") != std::string::npos && content.find(":=") == std::string::npos) {
                // Переменная: name : type
                variables.insert(content);
            }
        }

        // Functions
        for (const auto& func : functions) {
            out << func << ";\n";
        }
        if (!functions.empty()) out << "\n";

        // Procedures
        for (const auto& proc : procedures) {
            out << proc << ";\n";
        }
        if (!procedures.empty()) out << "\n";

        // Constants
        if (!constants.empty()) {
            out << "const\n";
            for (const auto& constant : constants) {
                out << "  " << constant << ";\n";
            }
            out << "\n";
        }

        // Variables
        if (!variables.empty()) {
            out << "var\n";
            for (const auto& variable : variables) {
                out << "  " << variable << ";\n";
            }
            out << "\n";
        }
    }

    void generateCodeFromNode(const std::string& nodeId, int currentIndent) {
        if (processed.count(nodeId)) return;
        processed.insert(nodeId);

        if (nodeContents.find(nodeId) == nodeContents.end()) return;

        std::string content = nodeContents[nodeId];
        std::string indentStr(currentIndent * 4, ' ');

        // Определяем тип узла по содержимому
        std::string lowerContent = content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);

        if (content.find("program") == 0) {
            // Пропускаем, уже обработали
        }
        else if (content.find("function") == 0 || content.find("procedure") == 0) {
            // Пропускаем объявления
        }
        else if (content.find("if") == 0) {
            generateIfStatement(nodeId, currentIndent);
            return;
        }
        else if (content.find("case") == 0) {
            generateCaseStatement(nodeId, currentIndent);
            return;
        }
        else if (content.find("for") == 0 || content.find("while") == 0) {
            generateLoopStatement(nodeId, currentIndent);
            return;
        }
        else if (content.find("until") == 0) {
            generateRepeatStatement(nodeId, currentIndent);
            return;
        }
        else {
            // Простые операторы
            out << indentStr << convertToPascalStatement(content) << ";\n";
        }

        // Переходим к следующим узлам
        if (graph.find(nodeId) != graph.end()) {
            for (const auto& [nextNode, label] : graph[nodeId]) {
                generateCodeFromNode(nextNode, currentIndent);
            }
        }
    }

    void generateIfStatement(const std::string& nodeId, int currentIndent) {
        std::string indentStr(currentIndent * 4, ' ');
        std::string content = nodeContents[nodeId];

        out << indentStr << content << "\n";
        out << indentStr << "begin\n";

        // Ищем ветку true
        for (const auto& [nextNode, label] : graph[nodeId]) {
            if (label == "true" || label.empty()) {
                generateCodeFromNode(nextNode, currentIndent + 1);
                break;
            }
        }

        out << indentStr << "end";

        // Проверяем есть ли ветка false
        bool hasFalse = false;
        for (const auto& [nextNode, label] : graph[nodeId]) {
            if (label == "false") {
                hasFalse = true;
                break;
            }
        }

        if (hasFalse) {
            out << " else\n";
            out << indentStr << "begin\n";

            for (const auto& [nextNode, label] : graph[nodeId]) {
                if (label == "false") {
                    generateCodeFromNode(nextNode, currentIndent + 1);
                    break;
                }
            }

            out << indentStr << "end";
        }

        out << ";\n";
        processed.insert(nodeId);
    }

    void generateCaseStatement(const std::string& nodeId, int currentIndent) {
        std::string indentStr(currentIndent * 4, ' ');
        std::string content = nodeContents[nodeId];

        out << indentStr << content << "\n";
        out << indentStr << "begin\n";

        // Собираем все ветки case
        std::vector<std::pair<std::string, std::string>> branches;
        for (const auto& [nextNode, label] : graph[nodeId]) {
            branches.push_back({nextNode, label});
        }

        // Генерируем ветки
        for (const auto& [branchNode, label] : branches) {
            if (label == "else") {
                out << indentStr << "  else:\n";
            } else {
                out << indentStr << "  " << label << ":\n";
            }
            out << indentStr << "    begin\n";
            generateCodeFromNode(branchNode, currentIndent + 2);
            out << indentStr << "    end;\n";
        }

        out << indentStr << "end;\n";
        processed.insert(nodeId);
    }

    void generateLoopStatement(const std::string& nodeId, int currentIndent) {
        std::string indentStr(currentIndent * 4, ' ');
        std::string content = nodeContents[nodeId];

        out << indentStr << content << "\n";
        out << indentStr << "begin\n";

        // Тело цикла (первая ветка)
        if (!graph[nodeId].empty()) {
            generateCodeFromNode(graph[nodeId][0].first, currentIndent + 1);
        }

        out << indentStr << "end;\n";
        processed.insert(nodeId);
    }

    void generateRepeatStatement(const std::string& nodeId, int currentIndent) {
        std::string indentStr(currentIndent * 4, ' ');

        out << indentStr << "repeat\n";
        out << indentStr << "begin\n";

        // Тело repeat (все узлы до условия until)
        if (!graph[nodeId].empty()) {
            generateCodeFromNode(graph[nodeId][0].first, currentIndent + 1);
        }

        out << indentStr << "end;\n";

        // until условие
        std::string content = nodeContents[nodeId];
        if (content.find("until ") == 0) {
            content = content.substr(6);
        }
        out << indentStr << "until " << content << ";\n";

        processed.insert(nodeId);
    }
    void generateBody() {

    }
public:
    PascalCodeGenerator() = default;



    std::string generatePascal(std::string code, const std::string& progName) {
        Lexer lexer(code,MERMAID);
        std::vector<Token> tokens= lexer.getTokenList();
        lexer.printTokenList();
        programName = progName;
        out.str("");
        out.clear();
        nodeContents.clear();
        graph.clear();
        processed.clear();

        // Парсим структуру из токенов
        parseFromTokens(tokens);

        // Заголовок программы
        out << "program " << programName << ";\n\n";

        // Объявления
        generateDeclarations();

        // Begin section
        out << "begin\n";

        generateBody();

        out << "end.\n";

        return out.str();
    }
};

#endif // PASCAL_CODE_GENERATOR_H