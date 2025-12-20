//
// Created by пк on 19.11.2025.
//

#ifndef AISDLAB_CPPPARSERTOAST_H
#define AISDLAB_CPPPARSERTOAST_H

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include "../CPPAst/CppAst.h"
#include "../CPPAst/CppExpr.h"
#include "../CPPAst/CppStmt.h"
#include "../CPPAst/CppDecl.h"
#include "../CPPAst/CppVisitor.h"
#include "../CPPAst/CppExprAcceptImpl.h"
#include "../Scripts/Lexer.h"
#include "../Scripts/Token.h"

class CppParserToAST {
public:
    std::unique_ptr<CppProgram> parse(const std::string &code) {
        Lexer lexer(code, types::CPP);
        tokens = lexer.getTokenList();
        lexer.printTokenList();

        // Фильтруем пробелы и комментарии
        std::vector<Token> filtered;
        filtered.reserve(tokens.size());
        for (const auto &token: tokens) {
            if (token.getType() == "SPACE" || token.getType() == "COMMENT") {
                continue;
            }
            filtered.push_back(token);
        }
        tokens.swap(filtered);
        current = 0;

        auto program = std::make_unique<CppProgram>();
        program->name = "test_program_by_CPP";

        if (tokens.empty()) {
            program->body = std::make_unique<CppCompoundStmt>();
            return program;
        }

        program->body = parseTranslationUnit();
        return program;
    }

private:
    std::vector<Token> tokens;
    size_t current{0};
    // Контекст парсера
    std::vector<std::string> currentNamespace;
    std::vector<CppTemplateParameter> currentTemplateParams;
    CppClassDecl *currentClass{nullptr};
    bool inTemplate{false};
    std::unordered_set<std::string> knownClasses;
    std::unordered_set<std::string> knownTypedefs;
    std::unordered_set<std::string> currentAccessScope;

    // ============ ОСНОВНОЙ ПАРСИНГ ============

    std::unique_ptr<CppCompoundStmt> parseTranslationUnit() {
        auto block = std::make_unique<CppCompoundStmt>();

        while (!isAtEnd()) {
            std::string tokenType = peekType();

            std::cout << "DEBUG: TranslationUnit at: " << tokenType
                      << " [" << peek().getValue() << "]"
                      << " (current=" << current << "/" << tokens.size() << ")" << std::endl;



            // 🔥 ВНЕШНЯЯ ИНИЦИАЛИЗАЦИЯ СТАТИЧЕСКИХ ПОЛЕЙ И ПЕРЕМЕННЫХ ПОЛЬЗОВАТЕЛЬСКИХ ТИПОВ
            if (check("IDENTIFIER") && knownClasses.count(peek().getValue()) > 0) {
                std::string className = advance().getValue();

                if (match("SCOPE")) {
                    // Это Circle::circleCount = 0;
                    if (check("IDENTIFIER")) {
                        std::string memberName = advance().getValue();

                        auto var = std::make_unique<CppVarDecl>();
                        var->typeName = ""; // Тип не нужен
                        var->name = className + "::" + memberName;

                        if (match("ASSIGN")) {
                            var->initializer = parseExpression();
                        }

                        if (match("SEMICOLON")) {
                            block->statements.push_back(std::move(var));
                            std::cout << "DEBUG: Added static field initialization: "
                                      << var->name << std::endl;
                            continue;
                        }
                    }
                } else {
                    // 🔥 ЭТО ВАЖНО: Data data; - объявление переменной пользовательского типа
                    // Вернемся назад и попробуем распарсить как обычную переменную
                    current -= 1; // вернуться к Data

                    // Пробуем распарсить как глобальную переменную
                    if (isGlobalVarStart()) {
                        std::cout << "DEBUG: Trying to parse " << className << " as user type variable" << std::endl;
                        try {
                            auto var = parseGlobalVar();
                            if (var) {
                                block->statements.push_back(std::move(var));
                                continue;
                            }
                        } catch (const std::exception &e) {
                            std::cout << "DEBUG: Failed to parse user type variable: " << e.what() << std::endl;
                        }
                    }
                }
            }

            // Пропускаем проблемные токены в начале
            if (!isDeclarationStart(tokenType) && !isPreprocessor(tokenType)) {
                std::cout << "Skipping non-declaration token: " << tokenType
                          << " [" << peek().getValue() << "]" << std::endl;
                advance();
                continue;
            }

            // Препроцессор
            if (isPreprocessor(tokenType)) {
                auto pp = parsePreprocessor();
                if (pp) block->statements.push_back(std::move(pp));
                continue;
            }

            // Пространства имен
            if (match("NAMESPACE")) {
                auto ns = parseNamespace();
                if (ns) block->statements.push_back(std::move(ns));
                continue;
            }

            // Шаблоны - пропускаем для упрощения
            if (match("TEMPLATE")) {
                std::cout << "Skipping template" << std::endl;
                skipUntilSemicolonOrBrace();
                continue;
            }

            // Классы и структуры
            if (match("CLASS") || match("STRUCT")) {
                std::cout << "DEBUG: Starting to parse class/struct at position " << current << std::endl;
                auto classDecl = parseClass();
                if (classDecl) {
                    std::string className = classDecl->name;
                    block->statements.push_back(std::move(classDecl));
                    knownClasses.insert(className);
                    std::cout << "DEBUG: Successfully added class to AST" << std::endl;
                } else {
                    std::cout << "DEBUG: Failed to parse class/struct" << std::endl;
                }
                continue;
            }

            // Перечисления
            if (match("ENUM")) {
                auto enumDecl = parseEnum();
                if (enumDecl) block->statements.push_back(std::move(enumDecl));
                continue;
            }

            if (match("UNION")) {
                auto unionDecl = parseUnion();
                if (unionDecl) block->statements.push_back(std::move(unionDecl));
                continue;
            }

            // typedef и using
            if (match("TYPEDEF")) {
                auto typedefDecl = parseTypedef();
                if (typedefDecl) block->statements.push_back(std::move(typedefDecl));
                continue;
            }

            if (match("USING")) {
                auto usingDecl = parseUsing();
                if (usingDecl) block->statements.push_back(std::move(usingDecl));
                continue;
            }

            // Глобальные переменные
            if (isGlobalVarStart()) {
                std::cout << "DEBUG: Found global var start: " << peek().getValue() << std::endl;
                auto var = parseGlobalVar();
                if (var) {
                    block->statements.push_back(std::move(var));
                    std::cout << "DEBUG: Successfully parsed global var" << std::endl;
                } else {
                    std::cout << "DEBUG: Failed to parse global var, skipping..." << std::endl;
                    skipUntilSemicolon();
                }
                continue;
            }

            // Функции
            if (isFunctionStart()) {
                auto func = parseFunction();
                if (func) block->statements.push_back(std::move(func));
                continue;
            }

            // Если ничего не распарсилось, пропускаем токен
            std::cout << "Skipping token in translation unit: " << tokenType
                      << " [" << peek().getValue() << "]" << std::endl;
            advance();
        }


        std::cout << "DEBUG: Finished parsing translation unit successfully!" << std::endl;
        return block;
    }

    void skipUntilSemicolon() {
        int braceLevel = 0;
        int parenLevel = 0;

        std::cout << "DEBUG skipUntilSemicolon: starting at " << peek().getValue() << std::endl;

        while (!isAtEnd()) {
            std::string type = peekType();

            if (type == "OPENCURLY") braceLevel++;
            else if (type == "CLOSECURLY") {
                if (braceLevel == 0) {
                    // Достигли закрывающей скобки класса - ВЫХОДИМ!
                    std::cout << "DEBUG: Reached class closing brace, stopping skip" << std::endl;
                    return;
                }
                braceLevel--;
            } else if (type == "OPENPARENTHESES") parenLevel++;
            else if (type == "CLOSEPARENTHESES") parenLevel--;
            else if (type == "SEMICOLON" && braceLevel == 0 && parenLevel == 0) {
                // Нашли точку с запятой на верхнем уровне
                advance();
                std::cout << "DEBUG: Found semicolon, stopping skip" << std::endl;
                return;
            }

            std::cout << "DEBUG skipUntilSemicolon: skipping " << peek().getValue()
                      << " (braceLevel=" << braceLevel << ", parenLevel=" << parenLevel << ")" << std::endl;
            advance();
        }
    }

    void skipUntilSemicolonOrBrace() {
        int braceLevel = 0;
        int parenLevel = 0;

        while (!isAtEnd()) {
            if (check("OPENCURLY")) {
                braceLevel++;
            } else if (check("CLOSECURLY")) {
                if (braceLevel == 0) {
                    std::cout << "DEBUG: Reached closing brace, stopping skip" << std::endl;
                    return; // НЕ потребляем закрывающую скобку!
                }
                braceLevel--;
            } else if (check("OPENPARENTHESES")) {
                parenLevel++;
            } else if (check("CLOSEPARENTHESES")) {
                if (parenLevel > 0) parenLevel--;
            } else if (check("SEMICOLON") && braceLevel == 0 && parenLevel == 0) {
                std::cout << "DEBUG: Reached semicolon, stopping skip" << std::endl;
                advance(); // пропускаем точку с запятой
                return;
            }
            std::cout << "Skipping in skipUntilSemicolonOrBrace: " << peek().getValue() << std::endl;
            advance();
        }
    }

    Token lookAhead(size_t offset = 1) {
        size_t pos = current + offset;
        return pos < tokens.size() ? tokens[pos] : Token();
    }

    std::unique_ptr<CppDecl> parseDeclaration() {
        if (isAtEnd()) return nullptr;

        // Препроцессор
        if (isPreprocessor(peekType())) {
            return parsePreprocessor();
        }

        // Пространства имен
        if (match("NAMESPACE")) {
            return parseNamespace();
        }

        // Шаблоны
        if (match("TEMPLATE")) {
            return parseTemplate();
        }

        // Классы и структуры
        if (match("CLASS") || match("STRUCT")) {
            return parseClass();
        }

        // Перечисления
        if (match("ENUM")) {
            return parseEnum();
        }

        // typedef и using
        if (match("TYPEDEF")) {
            return parseTypedef();
        }

        if (match("USING")) {
            return parseUsing();
        }

        // Глобальные переменные
        if (isGlobalVarStart()) {
            return parseGlobalVarAsDecl();
        }


        // Функции
        if (isFunctionStart()) {
            return parseFunction();
        }

        // Если ничего не распарсилось, пробуем как выражение
        try {
            auto expr = parseExpression();
            if (expr && match("SEMICOLON")) {
                // Для выражений возвращаем nullptr, так как они не являются объявлениями
                return nullptr;
            }
        } catch (...) {
            // Игнорируем ошибки выражений
        }

        // Пропускаем проблемный токен
        std::cout << "Skipping token in declaration: " << peek().getValue() << std::endl;
        advance();
        return nullptr;
    }

    std::unique_ptr<CppDecl> parseGlobalVarAsDecl() {
        try {
            auto decl = std::make_unique<CppVarDecl>();
            decl->typeName = parseTypeName();

            auto nameTok = consume("IDENTIFIER", "Ожидалось имя переменной");
            decl->name = nameTok.getValue();

            // Обработка массивов
            if (match("OPENBRACKET")) {
                decl->typeName += "[]";
                consume("CLOSEBRACKET", "Ожидался ']' после размера массива");
            }

            if (match("ASSIGN")) {
                decl->initializer = parseExpression();
            }

            consume("SEMICOLON", "Ожидался ';' после объявления");

            return std::unique_ptr<CppDecl>(dynamic_cast<CppDecl *>(decl.release()));

        } catch (const std::exception &e) {
            std::cerr << "Error in parseGlobalVarAsDecl: " << e.what() << std::endl;
            return nullptr;
        }
    }

    std::unique_ptr<CppTypedefDecl> parseTypedef() {
        auto typedefDecl = std::make_unique<CppTypedefDecl>();
        // Упрощенная реализация
        typedefDecl->typeName = parseTypeName();
        if (check("IDENTIFIER")) {
            typedefDecl->alias = advance().getValue();
        }
        consume("SEMICOLON", "Expected ';' after typedef");
        return typedefDecl;
    }

    std::unique_ptr<CppUsingDecl> parseUsing() {
        auto usingDecl = std::make_unique<CppUsingDecl>();
        // Упрощенная реализация
        if (match("NAMESPACE")) {
            usingDecl->isNamespace = true;
            if (check("IDENTIFIER")) {
                usingDecl->target = advance().getValue();
            }
        } else {
            usingDecl->name = parseTypeName();
        }
        consume("SEMICOLON", "Expected ';' after using");
        return usingDecl;
    }

    // ============ ПРЕПРОЦЕССОР ============

    std::unique_ptr<CppPreprocessorDirective> parsePreprocessor() {
        auto pp = std::make_unique<CppPreprocessorDirective>();
        Token directive = advance();
        pp->directive = directive.getValue();

        std::string content;
        while (!isAtEnd() && !isPreprocessor(peekType()) &&
               !isDeclarationStart(peekType())) {
            if (!content.empty()) content += " ";
            content += advance().getValue();
        }
        pp->value = content;

        return pp;
    }

    // ============ ОБЪЯВЛЕНИЯ ============

    std::unique_ptr<CppNamespaceDecl> parseNamespace() {
        auto ns = std::make_unique<CppNamespaceDecl>();

        if (check("IDENTIFIER")) {
            ns->name = advance().getValue();
        } else {
            ns->name = ""; // anonymous namespace
        }

        consume("OPENCURLY", "Expected '{' after namespace");
        ns->body = std::unique_ptr<CppCompoundStmt>(static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        return ns;
    }

    std::unique_ptr<CppTemplateDecl> parseTemplate() {
        auto templ = std::make_unique<CppTemplateDecl>();

        consume("JL", "Expected '<' after template");

        // Упрощенный парсинг параметров шаблона
        while (!isAtEnd() && !check("JG")) {
            if (match("TYPENAME") || match("CLASS")) {
                CppTemplateParameter param;
                param.kind = "type";
                if (check("IDENTIFIER")) {
                    param.name = advance().getValue();
                }
                templ->parameters.push_back(param);
            }

            if (match("COMMA")) continue;
            if (check("JG")) break;
            advance(); // пропускаем неизвестные токены
        }

        if (check("CLOSEANGLE")) {
            advance();
        }

        consume("JG", "Expected '>' after template parameters");

        // Парсим объявление после шаблона
        if (isDeclarationStart(peekType())) {
            templ->declaration = parseDeclaration();
        }

        return templ;
    }

    void skipUntil(const std::string &targetType) {
        while (!isAtEnd() && !check(targetType)) {
            advance();
        }
    }

    std::unique_ptr<CppDecl> parseFriendDecl() {
        size_t save = current;

        try {
            // Пытаемся распарсить как функцию
            if (isFunctionStart()) {
                auto func = parseFunction();
                if (func) {
                    // Можете добавить флаг friend или специальный тип
                    return func;
                }
            }
        } catch (...) {
            current = save;
        }

        // Пропускаем friend объявление
        std::cout << "DEBUG: Skipping friend declaration" << std::endl;
        skipUntilSemicolon();
        return nullptr;
    }

    void skipComplexMember() {
        if (isAtEnd()) return;

        std::cout << "DEBUG: Skipping complex class member: " << peek().getValue() << std::endl;

        int braceLevel = 0;
        int parenLevel = 0;


        while (!isAtEnd()) {

            std::string tokenType = peekType();
            std::string tokenValue = peek().getValue();

            // 🔥 КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: останавливаемся на ЛЮБОЙ закрывающей скобке на верхнем уровне
            if (tokenType == "CLOSECURLY" && braceLevel == 0 && parenLevel == 0) {
                std::cout << "DEBUG: Reached class closing brace, stopping skip (IMPORTANT!)" << std::endl;
                return; // НЕ потребляем закрывающую скобку!
            }
            // 🔥 ИЛИ если видим ";" на верхнем уровне - СТОП
            if (tokenType == "SEMICOLON" && braceLevel == 0 && parenLevel == 0) {
                std::cout << "DEBUG: SIMPLE skip - reached semicolon" << std::endl;
                advance();
                return;
            }


            // Отслеживаем уровни вложенности
            if (tokenType == "OPENPARENTHESES") {
                parenLevel++;
            } else if (tokenType == "CLOSEPARENTHESES") {
                if (parenLevel > 0) parenLevel--;
            } else if (tokenType == "OPENCURLY") {
                braceLevel++;
            } else if (tokenType == "CLOSECURLY") {
                if (braceLevel > 0) braceLevel--;
            } else if (tokenType == "SEMICOLON" && braceLevel == 0 && parenLevel == 0) {
                std::cout << "DEBUG: Reached semicolon, stopping skip" << std::endl;
                advance();
                return;
            }

            std::cout << "Skipping: " << tokenValue << " (braceLevel=" << braceLevel
                      << ", parenLevel=" << parenLevel << ")" << std::endl;
            advance();
        }
    }

    std::unique_ptr<CppClassDecl> parseClass() {
        std::cout << "DEBUG: parseClass() called" << std::endl;

        auto classDecl = std::make_unique<CppClassDecl>();
        classDecl->isStruct = previous().getType() == "STRUCT";

        if (check("IDENTIFIER")) {
            classDecl->name = advance().getValue();
            knownClasses.insert(classDecl->name);
            std::cout << "DEBUG: Parsing class: " << classDecl->name << std::endl;
        }

        // Наследование
        if (match("COLON")) {
            // Парсим список базовых классов
            bool first = true;
            while (!isAtEnd() && !check("OPENCURLY")) {
                if (!first && !match("COMMA")) {
                    break;
                }

                // Спецификатор доступа
                CppAccessSpecifier access = CppAccessSpecifier::Private;
                if (match("PUBLIC")) {
                    access = CppAccessSpecifier::Public;
                } else if (match("PROTECTED")) {
                    access = CppAccessSpecifier::Protected;
                } else if (match("PRIVATE")) {
                    access = CppAccessSpecifier::Private;
                }

                // Имя базового класса
                if (check("IDENTIFIER")) {
                    classDecl->baseClasses.push_back(advance().getValue());
                    classDecl->baseAccess.push_back(access);
                }

                first = false;
            }
        }

        if (!match("OPENCURLY")) {
            throw std::runtime_error("Expected '{'");
        }

        CppClassDecl *previousClass = currentClass;
        currentClass = classDecl.get();
        CppAccessSpecifier currentAccess = classDecl->isStruct ?
                                           CppAccessSpecifier::Public : CppAccessSpecifier::Private;

        // 🔥 УПРОЩЕННЫЙ ПАРСИНГ ТОЛЬКО ПОЛЕЙ
        while (!isAtEnd() && !check("CLOSECURLY")) {
            std::cout << "DEBUG class loop: " << peek().getValue()
                      << " (" << peek().getType() << ")" << std::endl;

            // ТОЛЬКО спецификаторы доступа и поля
            if (match("PUBLIC") || match("PROTECTED") || match("PRIVATE")) {
                if (previous().getType() == "PUBLIC") currentAccess = CppAccessSpecifier::Public;
                else if (previous().getType() == "PROTECTED") currentAccess = CppAccessSpecifier::Protected;
                else currentAccess = CppAccessSpecifier::Private;

                if (!match("COLON")) {
                    // Если нет двоеточия, это может быть что-то другое
                    continue;
                }
                continue;
            }
            // 🔥 УПРОЩЕННАЯ ОБРАБОТКА ВИРТУАЛЬНЫХ МЕТОДОВ
            if (match("VIRTUAL")) {
                std::cout << "DEBUG class: Found 'virtual'" << std::endl;

                // Проверяем, не деструктор ли это
                if (check("BITNOT")) {
                    std::cout << "DEBUG class: Virtual destructor" << std::endl;
                    // Обрабатываем в отдельном блоке для деструкторов
                } else {
                    // Виртуальный обычный метод
                    size_t save = current;
                    try {
                        auto method = parseMethod();
                        if (method) {
                            method->isVirtual = true;
                            method->access = currentAccess;
                            classDecl->members.push_back(std::move(method));
                            std::cout << "DEBUG class: Added virtual method" << std::endl;
                            continue;
                        }
                    } catch (const std::exception& e) {
                        std::cout << "DEBUG class: Virtual method error: " << e.what() << std::endl;
                        current = save;
                    }
                }
            }

            // 🔥 3. ДЕСТРУКТОРЫ (виртуальные и невиртуальные)
            if (check("BITNOT") && current + 1 < tokens.size() &&
                tokens[current + 1].getValue() == classDecl->name) {
                std::cout << "DEBUG class: Found destructor" << std::endl;

                bool isVirtualDestructor = false;
                if (current > 0 && tokens[current - 1].getType() == "VIRTUAL") {
                    isVirtualDestructor = true;
                    std::cout << "DEBUG class: Destructor is virtual" << std::endl;
                }

                size_t save = current;
                try {
                    auto dtor = parseDestructor();
                    if (dtor) {
                        dtor->access = currentAccess;
                        dtor->isVirtual=isVirtualDestructor;
                        classDecl->members.push_back(std::move(dtor));
                        std::cout << "DEBUG class: Added destructor" << std::endl;
                        continue;
                    }
                } catch (const std::exception& e) {
                    std::cout << "DEBUG class: Destructor error: " << e.what() << std::endl;
                    current = save;
                }
            }
            // 4. Пробуем конструктор
            if (check("IDENTIFIER") && peek().getValue() == classDecl->name) {
                std::cout << "DEBUG: Found constructor: " << classDecl->name << std::endl;

                size_t ctorStart = current;
                try {
                    auto ctor = parseConstructor();
                    if (ctor) {
                        auto method = std::make_unique<CppConstructorDecl>();
                        method->name = classDecl->name;
                        method->parameters = std::move(ctor->parameters);
                        method->initializers = std::move(ctor->initializers);
                        method->body = std::move(ctor->body);
                        method->access = currentAccess;
                        if (method) {
                            std::cout << "DEBUG: Added constructor with "
                                      << method->initializers.size() << " initializers" << std::endl;
                        }
                        classDecl->members.push_back(std::move(method));

                        continue;
                    }
                } catch (...) {
                    current = ctorStart;
                }
            }
            // 🔥 ОПЕРАТОРЫ
            if (check("IDENTIFIER") || check("BOOL") || check("VOID") ||
                check("INT") || check("DOUBLE")) {

                // Смотрим вперед на 2 токена
                size_t lookahead = current + 1;
                if (lookahead < tokens.size() &&
                    tokens[lookahead].getType() == "OPERATOR") {

                    std::cout << "DEBUG class: Found operator" << std::endl;

                    size_t save = current;
                    try {
                        auto op = parseOperator();
                        if (op) {
                            op->access = currentAccess;
                            classDecl->members.push_back(std::move(op));
                            continue;
                        }
                    } catch (...) {
                        current = save;
                    }
                }
            }
            if (match("FRIEND")) {
                // Пропускаем friend объявление для упрощения
                skipUntilSemicolon();
                continue;
            }

            // 🔥 5. ПАРСИНГ ОБЫЧНЫХ МЕТОДОВ (после всех спецификаторов)
            if (isTypeToken(peekType()) || check("IDENTIFIER")) {
                size_t methodStart = current;

                // Проверяем, что это метод, а не поле
                // Для этого смотрим, есть ли открывающая скобка после имени
                size_t tempPos = current;
                bool isMethod = false;

                try {
                    // Парсим тип
                    std::string returnType = parseTypeName();

                    // Должно быть имя
                    if (check("IDENTIFIER") || check("OPERATOR") || check("BITNOT")) {
                        // Пропускаем имя
                        std::string name = peek().getValue();
                        tempPos = current + 1;

                        // Смотрим дальше
                        if (tempPos < tokens.size() &&
                            tokens[tempPos].getType() == "OPENPARENTHESES") {
                            isMethod = true;
                        }
                    }
                } catch (...) {
                    // Игнорируем ошибки
                }

                current = methodStart;

                if (isMethod) {
                    try {
                        auto method = parseMethod();
                        if (method) {
                            method->access = currentAccess;
                            currentClass->members.push_back(std::move(method));
                            continue;
                        }
                    } catch (...) {
                        // Если не получилось, продолжаем
                    }
                }
            }

            // 🔥 6. ПРОБУЕМ ТОЛЬКО ПОЛЯ
            size_t save = current;
            if (tryParseField(currentAccess, classDecl.get())) {
                continue;
            }


            // Если кал, просто пропускаем токен
            current = save;
            std::cout << "DEBUG: Skipping token (not a field): " << peek().getValue() << std::endl;
            advance();
        }

        if (isAtEnd()) {
            throw std::runtime_error("Unexpected end of file while parsing class");
        }

        consume("CLOSECURLY", "Expected '}' after class body");

        if (check("SEMICOLON")) {
            advance();
        }

        std::cout << "DEBUG: Finished parsing class: " << classDecl->name
                  << " with " << classDecl->members.size() << " members" << std::endl;

        currentClass = previousClass;
        return classDecl;
    }

    bool tryParseField(CppAccessSpecifier access, CppClassDecl *currentClass) {
        size_t save = current;

        std::cout << "DEBUG tryParseField START: " << peek().getValue()
                  << " (type: " << peek().getType() << ")" << std::endl;

        try {
            // Парсим только первый токен как тип
            std::string typeName;
            bool isStatic = false;

            if (match("STATIC")) {
                isStatic = true;
            }
            // Берем только один токен как тип (Color)
            if (isTypeToken(peekType()) || check("IDENTIFIER")) {
                typeName = advance().getValue();
                std::cout << "DEBUG tryParseField: type = " << typeName << std::endl;
            } else {
                current = save;
                return false;
            }

            // Проверяем, что следующий токен - идентификатор (имя поля)
            if (!check("IDENTIFIER")) {
                std::cout << "DEBUG tryParseField: no field name after type" << std::endl;
                current = save;
                return false;
            }

            std::string fieldName = peek().getValue();
            std::cout << "DEBUG tryParseField: field name candidate: " << fieldName << std::endl;

            // 🔥 ВАЖНО: смотрим вперед на 2 токена
            // color ; - это поле
            // color ( - это метод
            // color = - это поле с инициализатором
            size_t lookahead = current + 1;
            if (lookahead < tokens.size()) {
                std::string nextType = tokens[lookahead].getType();
                std::cout << "DEBUG tryParseField: next token type = " << nextType
                          << ", value = " << tokens[lookahead].getValue() << std::endl;

                if (nextType == "OPENPARENTHESES") {
                    std::cout << "DEBUG tryParseField: has '(' after name, probably a method" << std::endl;
                    current = save;
                    return false;
                }
            }

            // Если дошли сюда - это поле
            advance(); // потребляем имя поля

            auto field = std::make_unique<CppFieldDecl>();
            field->typeName = typeName;
            field->name = fieldName;
            field->access = access;
            field->isStatic = isStatic;

            // Инициализатор
            if (match("ASSIGN")) {
                std::cout << "DEBUG tryParseField: parsing initializer" << std::endl;
                field->initializer = parseExpression();
            }

            if (!match("SEMICOLON")) {
                std::cout << "DEBUG tryParseField: expected ';', got: " << peek().getValue() << std::endl;
                throw std::runtime_error("Expected ';' after field");
            }

            currentClass->members.push_back(std::move(field));
            std::cout << "DEBUG tryParseField SUCCESS: added field " << typeName << " " << fieldName << std::endl;
            return true;

        } catch (const std::exception &e) {
            std::cout << "DEBUG tryParseField ERROR: " << e.what() << std::endl;
            current = save;
            return false;
        }
    }

    Token previous() {
        static Token dummy;
        if (current == 0 || tokens.empty()) return dummy;
        return tokens[current - 1];
    }

    void skipUntilSemicolonOrComma() {
        int parenLevel = 0;
        while (!isAtEnd()) {
            if (check("OPENPARENTHESES")) parenLevel++;
            else if (check("CLOSEPARENTHESES") && parenLevel > 0) parenLevel--;
            else if (check("SEMICOLON") && parenLevel == 0) return;
            else if (check("COMMA") && parenLevel == 0) return;
            advance();
        }
    }

    bool isKnownType(const std::string &name) {
        static const std::unordered_set<std::string> builtinTypes = {
                "void", "char", "short", "int", "long", "float", "double",
                "bool", "wchar_t", "size_t", "int8_t", "int16_t", "int32_t", "int64_t"
        };
        return builtinTypes.count(name) > 0;
    }

    std::unique_ptr<CppOperatorDecl> parseOperator() {
        auto opDecl = std::make_unique<CppOperatorDecl>();

        // Возвращаемый тип (может быть сложным, например "Circle&")
        try {
            opDecl->returnType = parseTypeName();
            std::cout << "DEBUG parseOperator: return type = " << opDecl->returnType << std::endl;
        } catch (...) {
            opDecl->returnType = "";
        }

        // Пропускаем "operator"
        if (!match("OPERATOR")) {
            throw std::runtime_error("Expected operator keyword");
        }

        // Парсим символ оператора
        std::string opSymbol;
        if (check("ASSIGN") || check("PLUS") || check("MINUS") ||
            check("JE") || check("INCREMENT") || check("BITSHIFTLEFT")) {
            opDecl->operatorSymbol = advance().getValue();
        } else {
            throw std::runtime_error("Unsupported operator");
        }


        // Параметры
        consume("OPENPARENTHESES", "Expected '(' after operator");
        opDecl->parameters = parseParameters();
        consume("CLOSEPARENTHESES", "Expected ')' after parameters");

        // Тело оператора
        if (match("OPENCURLY")) {
            opDecl->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        } else if (match("SEMICOLON")) {
            throw std::runtime_error("UnExpected operator body");
        } else {
            throw std::runtime_error("Expected operator body or ';'");
        }

        return opDecl;
    }

    std::unique_ptr<CppFunctionDecl> parseConstructor() {
        auto func = std::make_unique<CppFunctionDecl>();

        if (!check("IDENTIFIER")) {
            throw std::runtime_error("Expected constructor name");
        }

        func->name = advance().getValue();
        func->returnType = "";

        if (!match("OPENPARENTHESES")) {
            throw std::runtime_error("Expected '(' after constructor name");
        }

        func->parameters = parseParameters();

        if (!match("CLOSEPARENTHESES")) {
            throw std::runtime_error("Expected ')' after parameters");
        }

        std::cout << "🔥 DEBUG parseConstructor: After parameters, current token: "
                  << peek().getValue() << " (type: " << peek().getType() << ")" << std::endl;
        std::cout << "🔥 Looking for ':' for initializer list..." << std::endl;

        if (match("COLON")) {
            std::cout << "🔥 FOUND COLON! Parsing initializer list" << std::endl;
            std::cout << "🔥 Next token: " << peek().getValue() << " (type: " << peek().getType() << ")" << std::endl;

            while (!isAtEnd() && !check("OPENCURLY") && !check("SEMICOLON")) {
                std::cout << "🔥 In init loop, current: " << peek().getValue()
                          << " (type: " << peek().getType() << ")" << std::endl;

                if (check("IDENTIFIER")) {
                    CppInitializer init;
                    init.memberName = advance().getValue();
                    init.isBaseClass = false;

                    std::cout << "🔥 Found member: " << init.memberName << std::endl;

                    if (match("OPENPARENTHESES")) {
                        std::cout << "🔥 Found '('" << std::endl;
                        std::cout << "🔥 Before parseExpression: " << peek().getValue() << std::endl;

                        init.value = parseExpression();
                        std::cout << "🔥 Parsed expression" << std::endl;

                        if (!match("CLOSEPARENTHESES")) {
                            throw std::runtime_error("Expected ')' in initializer");
                        }
                        std::cout << "🔥 Found ')'" << std::endl;
                    }

                    // Добавляем в конструктор (нужно добавить поле initializers в CppFunctionDecl)
                    func->initializers.push_back(std::move(init));

                    std::cout << "🔥 Added initializer, total: " << func->initializers.size() << std::endl;


                    if (match("COMMA")) {
                        std::cout << "🔥 Found comma, continuing..." << std::endl;
                        continue;
                    } else {
                        std::cout << "🔥 No comma, breaking. Next: " << peek().getValue() << std::endl;
                        break;
                    }

                } else {
                    std::cout << "🔥 Not identifier: " << peek().getValue() << ", breaking" << std::endl;
                    break;
                }
            }
            std::cout << "🔥 Finished initializer list parsing" << std::endl;
        }
        else {
            std::cout << "🔥 NO COLON FOUND! Next token: " << peek().getValue() << std::endl;
        }

        // Тело конструктора
        if (match("OPENCURLY")) {
            std::cout << "🔥 Found '{', parsing body" << std::endl;
            func->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        } else if (match("SEMICOLON")) {
            // Объявление конструктора без тела
        } else {
            throw std::runtime_error("Expected constructor body or ';'");
        }

        return func;
    }

    std::unique_ptr<CppMethodDecl> parseDestructor() {
        auto func = std::make_unique<CppMethodDecl>();
        match("BITNOT"); // пропускаем ~
        func->name = "~" + advance().getValue(); // имя деструктора
        func->returnType = ""; // деструкторы не имеют возвращаемого типа

        std::cout << "DEBUG: Parsing destructor parameters for: " << func->name << std::endl;

        // Параметры (у деструктора нет параметров)
        if (!match("OPENPARENTHESES")) {
            throw std::runtime_error("Expected '(' after destructor name");
        }

        // У деструктора могут быть пустые параметры или с void
        if (!check("CLOSEPARENTHESES")) {
            // Если есть параметры, пропускаем их для упрощения
            skipUntilCommaOrParen();
        }

        if (!match("CLOSEPARENTHESES")) {
            throw std::runtime_error("Expected ')' after destructor parameters");
        }

        // Квалификаторы
        if (match("OVERRIDE")) {
            func->isOverride = true;
        }
        if (match("FINAL")) {
            // final qualifier
        }

        std::cout << "DEBUG: Parsing destructor body for: " << func->name << std::endl;


        // Тело деструктора
        if (match("OPENCURLY")) {
            func->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        } else if (match("SEMICOLON")) {
            // Объявление деструктора без тела
        } else {
            throw std::runtime_error("Expected destructor body or ';'");
        }
        std::cout << "DEBUG: Successfully parsed destructor: " << func->name << std::endl;

        return func;
    }

    std::unique_ptr<CppDecl> parseFieldDecl() {
        std::cout << "DEBUG: parseFieldDecl called at: " << peek().getValue() << std::endl;

        size_t save = current;

        try {
            // Квалификаторы
            CppStorageClass storage = CppStorageClass::Auto;
            bool isMutable = false;

            if (match("STATIC")) {
                storage = CppStorageClass::Static;
            }
            if (match("MUTABLE")) {
                isMutable = true;
            }

            // Тип
            std::string typeName;

            // Простой парсинг типа - берем все до идентификатора
            while (!isAtEnd() &&
                   (isTypeToken(peekType()) ||
                    check("IDENTIFIER") ||
                    check("CONST") || check("VOLATILE"))) {
                if (!typeName.empty()) typeName += " ";
                typeName += advance().getValue();
            }

            if (typeName.empty()) {
                throw std::runtime_error("Failed to parse field type");
            }

            // Имя поля
            if (!check("IDENTIFIER")) {
                throw std::runtime_error("Expected field name");
            }

            auto field = std::make_unique<CppFieldDecl>();
            field->typeName = typeName;
            field->name = advance().getValue();
            field->isMutable = isMutable;

            std::cout << "DEBUG: Parsed field: " << typeName << " " << field->name << std::endl;

            // Инициализатор
            if (match("ASSIGN")) {
                std::cout << "DEBUG: Parsing field initializer" << std::endl;
                field->initializer = parseExpression();
            }

            // Конец объявления
            if (!match("SEMICOLON")) {
                throw std::runtime_error("Expected ';' after field declaration");
            }

            std::cout << "DEBUG: Successfully parsed field: " << field->name << std::endl;
            return std::unique_ptr<CppDecl>(field.release());

        } catch (const std::exception &e) {
            current = save;
            std::cout << "DEBUG: parseFieldDecl failed: " << e.what() << std::endl;
            return nullptr;
        }
    }

    std::unique_ptr<CppFunctionDecl> parseFunction() {

        std::cout << "DEBUG: Starting parseFunction at token: " << peek().getValue() << std::endl;

        auto func = std::make_unique<CppFunctionDecl>();

        // Возвращаемый тип
        try {
            func->returnType = parseTypeName();
            std::cout << "DEBUG: Parsed return type: " << func->returnType << std::endl;
        } catch (const std::exception &e) {
            std::cout << "DEBUG: Failed to parse return type: " << e.what() << std::endl;
            func->returnType = "";
        }

        // Имя функции
        if (check("IDENTIFIER") || check("OPERATOR")) {
            func->name = advance().getValue();
            std::cout << "DEBUG: Function name: " << func->name << std::endl;
        } else {
            std::cout << "DEBUG: Expected function name, got: " << peek().getValue() << std::endl;
            return nullptr;
        }


//        // Шаблонные специализации
//        if (match("OPENANGLE")) {
//            // Пропускаем template arguments для упрощения
//            while (!isAtEnd() && !check("CLOSEANGLE")) {
//                advance();
//            }
//            consume("CLOSEANGLE", "Expected '>'");
//        }

        // Параметры
        if (!match("OPENPARENTHESES")) {
            std::cout << "DEBUG: Expected '(', got: " << peek().getValue() << std::endl;
            throw std::runtime_error("Expected '(' after function name");
        }


        std::cout << "DEBUG: Parsing parameters..." << std::endl;

        func->parameters = parseParameters();

        if (!match("CLOSEPARENTHESES")) {
            std::cout << "DEBUG: Expected ')', got: " << peek().getValue() << std::endl;
            throw std::runtime_error("Expected ')' after parameters");
        }

        // Квалификаторы
        if (match("CONST")) {
            func->isConst = true;
        }
        if (match("VOLATILE")) {
            // volatile qualifier
        }
        if (match("NOEXCEPT")) {
            //func->isNoexcept = true;
            if (match("OPENPARENTHESES")) {
                // Пропускаем condition
                parseExpression();
                consume("CLOSEPARENTHESES", "Expected ')' after noexcept");
            }
        }
        if (match("OVERRIDE")) {
            func->isOverride = true;
        }
        if (match("FINAL")) {
            //func->isFinal = true;
        }

        // Тело функции
//        if (match("ASSIGN")) {
//            if (match("ZERO")) { // = 0 (pure virtual)
//                // pure virtual function
//            } else if (match("DEFAULT")) { // = default
//                // defaulted function
//            } else if (match("DELETE")) { // = delete
//                // deleted function
//            }
//            consume("SEMICOLON", "Expected ';'");
//        } else
        if (match("OPENCURLY")) {
            func->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        } else if (match("SEMICOLON")) {
            // Прототип функции
        } else {
            throw std::runtime_error("Expected function body or ';'");
        }

        return func;
    }

    bool isFieldStart() {
        size_t save = current;

        // Проверяем квалификаторы
        while (match("STATIC") || match("MUTABLE") ||
               match("CONST") || match("VOLATILE")) {
            // просто пропускаем
        }

        // Пытаемся распарсить тип
        try {
            std::string typeName = parseTypeName();

            // Должен быть идентификатор
            if (check("IDENTIFIER")) {
                current = save;
                return true;
            }
        } catch (...) {
            // игнорируем ошибки
        }

        current = save;
        return false;
    }

    bool isFieldDeclaration() {
        size_t save = current;

        // Пропускаем квалификаторы
        while (match("STATIC") || match("MUTABLE") ||
               match("CONST") || match("VOLATILE")) {
            // просто пропускаем
        }

        // Пытаемся распарсить тип
        try {
            std::string typeName = parseTypeName();

            // Если удалось распарсить тип, проверяем что дальше
            if (check("IDENTIFIER")) {
                // Это поле, если нет открывающей скобки сразу после имени
                advance(); // потребляем имя

                bool isField = !check("OPENPARENTHESES");

                current = save;
                return isField;
            }
        } catch (...) {
            // Игнорируем ошибки
        }

        current = save;
        return false;
    }

    std::unique_ptr<CppEnumDecl> parseEnum() {
        auto enumDecl = std::make_unique<CppEnumDecl>();

        if (match("CLASS") || match("STRUCT")) {
            enumDecl->isScoped = true;
        }

        if (check("IDENTIFIER")) {
            enumDecl->name = advance().getValue();
        }

        // Базовый тип
        if (match("COLON")) {
            // Парсим тип, а не просто токен
            std::string baseType;
            while (isTypeToken(peekType()) || check("IDENTIFIER")) {
                if (!baseType.empty()) baseType += " ";
                baseType += advance().getValue();
            }
            enumDecl->underlyingType = baseType;
        }

        if (!match("OPENCURLY")) {
            std::cout << "DEBUG: Expected '{', got: " << peek().getValue() << std::endl;
            throw std::runtime_error("Expected '{' after enum");
        }

        // Перечислители
        while (!isAtEnd() && !check("CLOSECURLY")) {
            if (check("IDENTIFIER")) {
                std::string enumerator = advance().getValue();
                std::unique_ptr<CppExpr> value;

                if (match("ASSIGN")) {
                    value = parseExpression();
                }

                enumDecl->enumerators.emplace_back(enumerator, std::move(value));
            }

            if (!match("COMMA")) {
                break;
            }
        }

        consume("CLOSECURLY", "Expected '}' after enum");
        consume("SEMICOLON", "Expected ';' after enum");
        return enumDecl;
    }

    // ============ ВЫРАЖЕНИЯ ============

    std::unique_ptr<CppExpr> parseExpression() {
        return parseAssignment();
    }

    std::unique_ptr<CppExpr> parseAssignment() {
        auto expr = parseTernary();

        if (match("ASSIGN") || match("PLUSASSIGN") || match("MINUSASSIGN") ||
            match("MULTIASSIGN") || match("DIVASSIGN") || match("MODASSIGN") ||
            match("BITSHIFTLEFTASSIGN") || match("BITSHIFTRIGHTASSIGN")) {

            auto assign = std::make_unique<CppBinaryOp>();
            assign->left = std::move(expr);

            std::string op = previous().getValue();
            if (op == "=") assign->op = CppBinOpKind::Assign;
            else if (op == "+=") assign->op = CppBinOpKind::AddAssign;
            else if (op == "-=") assign->op = CppBinOpKind::SubAssign;
            else if (op == "*=") assign->op = CppBinOpKind::MulAssign;
            else if (op == "/=") assign->op = CppBinOpKind::DivAssign;
            else if (op == "%=") assign->op = CppBinOpKind::ModAssign;
            else if (op == "<<=") assign->op = CppBinOpKind::ShlAssign;
            else if (op == ">>=") assign->op = CppBinOpKind::ShrAssign;

            assign->right = parseAssignment();
            return assign;
        }

        return expr;
    }

    std::unique_ptr<CppExpr> parseTernary() {
        auto expr = parseLogicalOr();

        if (match("QUESTION")) {
            auto ternary = std::make_unique<CppTernaryOp>();
            ternary->condition = std::move(expr);
            ternary->thenExpr = parseExpression();
            consume("COLON", "Expected ':' in ternary operator");
            ternary->elseExpr = parseExpression();
            return ternary;
        }

        return expr;
    }

    std::unique_ptr<CppExpr> parseLogicalOr() {
        auto expr = parseLogicalAnd();
        while (match("OR")) {
            auto binOp = std::make_unique<CppBinaryOp>();
            binOp->op = CppBinOpKind::Or;
            binOp->left = std::move(expr);
            binOp->right = parseLogicalAnd();
            expr = std::move(binOp);
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseLogicalAnd() {
        auto expr = parseBitwiseOr();
        while (match("AND")) {
            auto binOp = std::make_unique<CppBinaryOp>();
            binOp->op = CppBinOpKind::And;
            binOp->left = std::move(expr);
            binOp->right = parseBitwiseOr();
            expr = std::move(binOp);
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseBitwiseOr() {
        auto expr = parseBitwiseXor();
        while (match("BITOR")) {
            auto right = parseBitwiseXor();
            expr = makeBinary(std::move(expr), CppBinOpKind::BitOr, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseBitwiseXor() {
        auto expr = parseBitwiseAnd();
        while (match("BITXOR")) {
            auto right = parseBitwiseAnd();
            expr = makeBinary(std::move(expr), CppBinOpKind::BitXor, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseBitwiseAnd() {
        auto expr = parseEquality();
        while (match("BITAND")) {
            auto right = parseEquality();
            expr = makeBinary(std::move(expr), CppBinOpKind::BitAnd, std::move(right));
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseEquality() {
        auto expr = parseRelational();
        while (true) {
            if (match("JE")) {
                auto right = parseRelational();
                expr = makeBinary(std::move(expr), CppBinOpKind::Eq, std::move(right));
            } else if (match("JNE")) {
                auto right = parseRelational();
                expr = makeBinary(std::move(expr), CppBinOpKind::Ne, std::move(right));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseRelational() {
        auto expr = parseShift();
        while (true) {
            if (match("JGE")) {
                auto right = parseShift();
                expr = makeBinary(std::move(expr), CppBinOpKind::Ge, std::move(right));
            } else if (match("JLE")) {
                auto right = parseShift();
                expr = makeBinary(std::move(expr), CppBinOpKind::Le, std::move(right));
            } else if (match("JG")) {
                auto right = parseShift();
                expr = makeBinary(std::move(expr), CppBinOpKind::Gt, std::move(right));
            } else if (match("JL")) {
                auto right = parseShift();
                expr = makeBinary(std::move(expr), CppBinOpKind::Lt, std::move(right));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseShift() {
        auto expr = parseAdditive();
        while (true) {
            if (match("BITSHIFTLEFT")) {
                // Проверяем, это вывод в поток или битовый сдвиг
                // Упрощенно - всегда считаем битовым сдвигом
                auto right = parseAdditive();
                expr = makeBinary(std::move(expr), CppBinOpKind::Shl, std::move(right));
            } else if (match("BITSHIFTRIGHT")) {
                auto right = parseAdditive();
                expr = makeBinary(std::move(expr), CppBinOpKind::Shr, std::move(right));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppMethodDecl> parseMethod() {
        size_t methodStart = current;
        std::cout << "DEBUG parseMethod START at: " << peek().getValue()
                  << " (type: " << peek().getType() << ")" << std::endl;
        // 🔥 ПРОВЕРКА НА ДЕСТРУКТОР
        if (check("BITNOT")) {
            std::cout << "DEBUG parseMethod: this is a destructor, calling parseDestructor" << std::endl;
            // Временно откатываемся и вызываем parseDestructor
            current = methodStart;
            return nullptr; // parseDestructor будет вызван отдельно
        }
        auto method = std::make_unique<CppMethodDecl>();

        if (match("STATIC")) {
            method->isStatic = true;
            std::cout << "DEBUG parseMethod: method is static" << std::endl;
        }

        // 🔥 УПРОЩЕННЫЙ ПАРСИНГ ТИПА
        std::string returnType;
        try {
            // Просто берем следующий токен как тип
            if (isTypeToken(peekType()) || check("IDENTIFIER")) {
                returnType = advance().getValue();
                method->returnType = returnType;
                std::cout << "DEBUG parseMethod: return type = " << returnType << std::endl;
            } else {
                std::cout << "DEBUG parseMethod: no return type" << std::endl;
                throw std::runtime_error("No return type");
            }
        } catch (const std::exception &e) {
            std::cout << "DEBUG parseMethod: failed to parse return type: " << e.what() << std::endl;
            current = methodStart;
            return nullptr;
        }

        // Имя метода
        if (check("IDENTIFIER") || check("OPERATOR") || check("BITNOT")) {
            method->name = advance().getValue();
            std::cout << "DEBUG parseMethod: method name = " << method->name << std::endl;
        } else {
            std::cout << "DEBUG parseMethod: expected method name" << std::endl;
            current = methodStart;
            return nullptr;
        }

        // Параметры
        if (!match("OPENPARENTHESES")) {
            std::cout << "DEBUG parseMethod: expected '('" << std::endl;
            current = methodStart;
            return nullptr;
        }

        std::cout << "DEBUG parseMethod: parsing parameters..." << std::endl;
        method->parameters = parseParameters();

        if (!match("CLOSEPARENTHESES")) {
            std::cout << "DEBUG parseMethod: expected ')'" << std::endl;
            current = methodStart;
            return nullptr;
        }

        // Квалификаторы
        if (match("CONST")) {
            method->isConst = true;
            std::cout << "DEBUG parseMethod: method is const" << std::endl;
        }
        if (match("OVERRIDE")) {
            method->isOverride = true;
            std::cout << "DEBUG parseMethod: method is override" << std::endl;
        }
        if (match("STATIC")) {
            method->isStatic = true;
            std::cout << "DEBUG parseMethod: method is static" << std::endl;
        }
        if (match("FINAL")) {
            method->isFinal = true;
            std::cout << "DEBUG parseMethod: method is final" << std::endl;
        }

        // 🔥 ОБРАБОТКА = 0 (чисто виртуальный)
        if (match("ASSIGN")) {
            if (check("ZERO") || check("VALUEINTEGER")) {
                std::string zero = advance().getValue();
                std::cout << "DEBUG parseMethod: pure virtual method (= " << zero << ")" << std::endl;
                method->isVirtual = true; // чисто виртуальный

                if (!match("SEMICOLON")) {
                    std::cout << "DEBUG parseMethod: expected ';' after = 0" << std::endl;
                    current = methodStart;
                    return nullptr;
                }

                std::cout << "DEBUG parseMethod SUCCESS: pure virtual method" << std::endl;
                return method; // нет тела
            } else {
                // Если не 0, откатываемся
                std::cout << "DEBUG parseMethod: not = 0, rolling back" << std::endl;
                current--;
            }
        }

        // Тело метода
        if (match("OPENCURLY")) {
            std::cout << "DEBUG parseMethod: parsing body..." << std::endl;
            method->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
            std::cout << "DEBUG parseMethod: body parsed" << std::endl;
        } else if (match("SEMICOLON")) {
            // Прототип метода
            std::cout << "DEBUG parseMethod: method prototype (no body)" << std::endl;
        } else {
            std::cout << "DEBUG parseMethod: expected body or ';'" << std::endl;
            current = methodStart;
            return nullptr;
        }

        std::cout << "DEBUG parseMethod SUCCESS: " << method->returnType << " " << method->name << std::endl;
        return method;

    }

    std::unique_ptr<CppExpr> parseAdditive() {
        auto expr = parseMultiplicative();
        while (true) {
            if (match("PLUS")) {
                auto right = parseMultiplicative();
                expr = makeBinary(std::move(expr), CppBinOpKind::Add, std::move(right));
            } else if (match("MINUS")) {
                auto right = parseMultiplicative();
                expr = makeBinary(std::move(expr), CppBinOpKind::Sub, std::move(right));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseMultiplicative() {
        auto expr = parseUnary();
        while (true) {
            if (match("MULTI")) {
                auto right = parseUnary();
                expr = makeBinary(std::move(expr), CppBinOpKind::Mul, std::move(right));
            } else if (match("DIV")) {
                auto right = parseUnary();
                expr = makeBinary(std::move(expr), CppBinOpKind::Div, std::move(right));
            } else if (match("MOD")) {
                auto right = parseUnary();
                expr = makeBinary(std::move(expr), CppBinOpKind::Mod, std::move(right));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parseSizeofExpr() {
        auto sizeofExpr = std::make_unique<CppSizeofExpr>();

        if (match("OPENPARENTHESES")) {
            if (isTypeToken(peekType())) {
                sizeofExpr->expression = parseType();
                sizeofExpr->isType = true;
            } else {
                sizeofExpr->expression = parseExpression();
                sizeofExpr->isType = false;
            }
            consume("CLOSEPARENTHESES", "Expected ')' after sizeof");
        } else {
            sizeofExpr->expression = parseUnary();
            sizeofExpr->isType = false;
        }

        return sizeofExpr;
    }

    std::unique_ptr<CppExpr> parseLiteral() {
        Token token = previous();

        if (token.getType() == "VALUESTRING" || token.getType() == "VALUECHAR") {
            auto lit = std::make_unique<CppStringLiteral>();
            lit->value = token.getValue();
            return lit;
        } else if (token.getType() == "VALUEBOOL") {
            auto lit = std::make_unique<CppBoolLiteral>();
            lit->value = (token.getValue() == "true");
            return lit;
        } else if (token.getType() == "VALUEINTEGER") {
            auto lit = std::make_unique<CppIntLiteral>();
            try {
                lit->value = std::stoll(token.getValue());
            } catch (...) {
                lit->value = 0;
            }
            return lit;
        } else if (token.getType() == "VALUEFLOAT") {
            auto lit = std::make_unique<CppRealLiteral>();
            try {
                lit->value = std::stod(token.getValue());
            } catch (...) {
                lit->value = 0.0;
            }
            return lit;
        }

        return nullptr;
    }

    CppStmtPtr parseTryStmt() {
        auto tryStmt = std::make_unique<CppTryStmt>();

        tryStmt->tryBlock = std::unique_ptr<CppCompoundStmt>(
                static_cast<CppCompoundStmt *>(parseCompoundStmt().release())
        );

        while (match("CATCH")) {
            auto catchStmt = std::make_unique<CppCatchStmt>();
            consume("OPENPARENTHESES", "Expected '(' after catch");

            if (isTypeToken(peekType())) {
                catchStmt->exceptionType = parseTypeName();
                if (check("IDENTIFIER")) {
                    catchStmt->exceptionName = advance().getValue();
                }
            }

            consume("CLOSEPARENTHESES", "Expected ')' after catch parameters");
            catchStmt->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release())
            );

            tryStmt->handlers.push_back(std::move(catchStmt));
        }

        return tryStmt;
    }

    CppExprPtr parseType() {
        // Упрощенная версия - возвращаем идентификатор типа
        auto typeExpr = std::make_unique<CppIdentifier>();
        typeExpr->name = parseTypeName();
        return typeExpr;
    }

    std::unique_ptr<CppExpr> makeUnary(const std::string &op, std::unique_ptr<CppExpr> operand, bool postfix) {
        if (!operand) {
            throw std::runtime_error("Операнд унарной операции не может быть nullptr");
        }
        auto node = std::make_unique<CppUnaryOp>();
        node->op = op;
        node->postfix = postfix;
        node->operand = std::move(operand);
        return node;
    }

    std::unique_ptr<CppExpr>
    makeBinary(std::unique_ptr<CppExpr> left, CppBinOpKind kind, std::unique_ptr<CppExpr> right) {
        if (!left || !right) {
            throw std::runtime_error("Операнды бинарной операции не могут быть nullptr");
        }
        auto node = std::make_unique<CppBinaryOp>();
        node->op = kind;
        node->left = std::move(left);
        node->right = std::move(right);
        return node;
    }

    CppStmtPtr parseGlobalVar() {
        size_t save = current;

        try {
            // 🔥 СПЕЦИАЛЬНАЯ ОБРАБОТКА ДЛЯ КВАЛИФИЦИРОВАННЫХ ИМЕН
            // Проверяем, может ли это быть Circle::circleCount
            if (check("IDENTIFIER") && knownClasses.count(peek().getValue()) > 0) {
                std::string className = advance().getValue();

                if (match("SCOPE")) {
                    // Это точно квалифицированное имя
                    auto varDecl = std::make_unique<CppVarDecl>();
                    varDecl->typeName = ""; // Тип уже был в классе
                    varDecl->name = className + "::" +
                                    consume("IDENTIFIER", "Expected member name").getValue();

                    // Инициализатор
                    if (match("ASSIGN")) {
                        varDecl->initializer = parseExpression();
                    }

                    consume("SEMICOLON", "Expected ';' after member initialization");
                    return varDecl;
                } else {
                    // Не квалифицированное - откатываемся
                    current = save;
                }
            }

            // 🔥 ЕСЛИ НЕ КВАЛИФИЦИРОВАННОЕ, ИСПОЛЬЗУЕМ СТАРЫЙ ПАРСЕР
            return parseVarDecl(true);

        } catch (const std::exception &e) {
            current = save;
            std::cerr << "Error in parseGlobalVar: " << e.what() << std::endl;
            return nullptr;
        }
    }

    std::unique_ptr<CppExpr> parseUnary() {
//        if (match("SIZEOF")) {
//            auto sizeofExpr = std::make_unique<SizeofExpr>();
//
//            if (match("OPENPARENTHESES")) {
//                // sizeof(type)
//                if (isTypeToken(peekType())) {
//                    sizeofExpr->expression = parseType();
//                    sizeofExpr->isType = true;
//                } else {
//                    //sizeof(expression)
//                    sizeofExpr->expression = parseExpression();
//                    sizeofExpr->isType = false;
//                }
//                consume("CLOSEPARENTHESES", "Ожидалась ')' после sizeof");
//            } else {
//                // sizeof expression (без скобок)
//                sizeofExpr->expression = parseUnary();
//                sizeofExpr->isType = false;
//            }
//
//            return sizeofExpr;
//        }
        if (match("INCREMENT")) {
            auto operand = parseUnary();
            return makeUnary("++", std::move(operand), false);
        }
        if (match("DECREMENT")) {
            auto operand = parseUnary();
            return makeUnary("--", std::move(operand), false);
        }
        if (match("MULTI")) {
            auto operand = parseUnary();
            return makeUnary("*", std::move(operand), false);
        }

        if (match("BITAND")) {
            auto operand = parseUnary();
            return makeUnary("&", std::move(operand), false);
        }
        if (match("PLUS")) {
            auto operand = parseUnary();
            return operand;
        }
        if (match("MINUS")) {
            auto operand = parseUnary();
            return makeUnary("-", std::move(operand), false);
        }
        if (match("NOT")) {
            auto operand = parseUnary();
            return makeUnary("!", std::move(operand), false);
        }
        if (match("BITNOT")) {
            auto operand = parseUnary();
            return makeUnary("~", std::move(operand), false);
        }
        return parsePostfix();
    }


    /*  std::unique_ptr<CppDecl> parseUnion() {
          auto unionDecl = std::make_unique<CppClassDecl>(); // или специальный UnionDecl
          unionDecl->isStruct = true; // обрабатываем union аналогично struct
          unionDecl->isUnion = true;

          if (check("IDENTIFIER")) {
              unionDecl->name = advance().getValue();
              knownClasses.insert(unionDecl->name);
              std::cout << "DEBUG: Parsing union: " << unionDecl->name << std::endl;
          }

          consume("OPENCURLY", "Ожидалась '{' после union");

          std::cout << "DEBUG: Started parsing union body" << std::endl;

          // Парсим поля
          /*    while (!isAtEnd() && !check("CLOSECURLY")) {
              if (isTypeToken(peekType())) {
                  auto field = parseFieldDecl();
                  if (field) {
                      if (auto fieldDecl = dynamic_cast<CppFieldDecl *>(field.get())) {
                          fieldDecl->access = CppAccessSpecifier::Public; // в union все public
                          unionDecl->members.push_back(std::move(field));
                      }
                  }
              } else {
                  advance();
              }
          }
  */

    // 🔥 УПРОЩЕННЫЙ ПАРСИНГ ПОЛЕЙ UNION
    /*  while (!isAtEnd() && !check("CLOSECURLY")) {
          std::cout << "DEBUG union loop: " << peek().getValue() << " (" << peek().getType() << ")" << std::endl;

          // Пропускаем пустые строки, комментарии
          if (peek().getType() == "SPACE" || peek().getType() == "COMMENT") {
              advance();
              continue;
          }

          // Сохраняем позицию
          size_t startPos = current;

          // Пробуем распарсить поле
          try {
              // Тип
              std::string typeName;

              std::cout << "DEBUG: Trying to parse type starting with: " << peek().getValue() << std::endl;

              // Собираем тип
              while (!isAtEnd() &&
                     (isTypeToken(peekType()) ||
                      check("IDENTIFIER") ||
                      peek().getValue() == "const" ||
                      peek().getValue() == "volatile" ||
                      peek().getValue() == "unsigned" ||
                      peek().getValue() == "signed" ||
                      peek().getValue() == "short" ||
                      peek().getValue() == "long")) {

                  if (!typeName.empty()) typeName += " ";
                  typeName += advance().getValue();
                  std::cout << "DEBUG: Added to type: " << typeName << std::endl;
              }

              if (typeName.empty()) {
                  throw std::runtime_error("Could not parse type");
              }

              std::cout << "DEBUG: Parsed type: " << typeName << std::endl;

              // Имя поля
              if (!check("IDENTIFIER")) {
                  std::cout << "DEBUG: No field name after type, got: " << peek().getValue() << std::endl;
                  throw std::runtime_error("Expected field name");
              }

              std::string fieldName = advance().getValue();
              std::cout << "DEBUG: Field name: " << fieldName << std::endl;

              // Точка с запятой
              if (!match("SEMICOLON")) {
                  std::cout << "DEBUG: No semicolon after field, got: " << peek().getValue() << std::endl;
                  throw std::runtime_error("Expected ';' after field");
              }

              // УСПЕХ!
              auto field = std::make_unique<CppFieldDecl>();
              field->typeName = typeName;
              field->name = fieldName;
              field->access = CppAccessSpecifier::Public;

              unionDecl->members.push_back(std::move(field));
              std::cout << "DEBUG: SUCCESS! Added union field: " << typeName << " " << fieldName << std::endl;
              continue;

          } catch (const std::exception &e) {
              std::cout << "DEBUG: Field parsing failed: " << e.what() << std::endl;
              // Возвращаемся к началу и пропускаем токен
              current = startPos;
              std::cout << "DEBUG: Skipping token: " << peek().getValue() << std::endl;
              advance();
          }
      }


      if (isAtEnd()) {
          throw std::runtime_error("Unexpected end of file while parsing union");
      }

      consume("CLOSECURLY", "Expected '}' after union");
      consume("SEMICOLON", "Expected ';' after union");

      return unionDecl;
  }
*/
    std::unique_ptr<CppDecl> parseUnion() {
        std::cout << "DEBUG: parseUnion() called" << std::endl;

        auto unionDecl = std::make_unique<CppClassDecl>();
        unionDecl->isStruct = true;
        unionDecl->isUnion = true;

        if (check("IDENTIFIER")) {
            unionDecl->name = advance().getValue();
            knownClasses.insert(unionDecl->name);
            std::cout << "DEBUG: Union name: " << unionDecl->name << std::endl;
        }

        if (!match("OPENCURLY")) {
            throw std::runtime_error("Expected '{' after union");
        }

        std::cout << "DEBUG: Parsing union body" << std::endl;

        // Парсим поля union
        while (!isAtEnd() && !check("CLOSECURLY")) {
            std::cout << "DEBUG union: at " << peek().getValue()
                      << " (" << peek().getType() << ")" << std::endl;

            // Пропускаем пробелы/комментарии
            if (peek().getType() == "SPACE" || peek().getType() == "COMMENT") {
                advance();
                continue;
            }

            size_t start = current;

            try {
                // 🔥 ВАЖНО: парсим только тип, без имени
                std::string fieldType;

                // Собираем тип (только ключевые слова типов и квалификаторы)
                while (!isAtEnd() &&
                       (isTypeToken(peekType()) ||
                        peek().getValue() == "const" ||
                        peek().getValue() == "volatile" ||
                        peek().getValue() == "signed" ||
                        peek().getValue() == "unsigned" ||
                        peek().getValue() == "short" ||
                        peek().getValue() == "long")) {

                    if (!fieldType.empty()) fieldType += " ";
                    fieldType += advance().getValue();
                }

                // 🔥 ЕСЛИ НЕТ ТИПА, проверяем пользовательский тип (union, struct, class)
                if (fieldType.empty() && check("IDENTIFIER") && knownClasses.count(peek().getValue()) > 0) {
                    fieldType = advance().getValue();
                }
                    // 🔥 ИЛИ это может быть простой идентификатор как тип
                else if (fieldType.empty() && check("IDENTIFIER")) {
                    fieldType = advance().getValue();
                }

                if (fieldType.empty()) {
                    throw std::runtime_error("Could not parse field type");
                }

                std::cout << "DEBUG union: parsed type = '" << fieldType << "'" << std::endl;

                // 🔥 Теперь имя поля (должен быть идентификатор)
                if (!check("IDENTIFIER")) {
                    std::cout << "DEBUG union: expected IDENTIFIER, got " << peek().getType()
                              << " [" << peek().getValue() << "]" << std::endl;
                    throw std::runtime_error("Expected field name");
                }

                std::string fieldName = advance().getValue();
                std::cout << "DEBUG union: field name = '" << fieldName << "'" << std::endl;

                // Точка с запятой
                if (!match("SEMICOLON")) {
                    std::cout << "DEBUG union: expected SEMICOLON, got " << peek().getType()
                              << " [" << peek().getValue() << "]" << std::endl;
                    throw std::runtime_error("Expected ';' after field");
                }

                // Добавляем поле
                auto field = std::make_unique<CppFieldDecl>();
                field->typeName = fieldType;
                field->name = fieldName;
                field->access = CppAccessSpecifier::Public;

                unionDecl->members.push_back(std::move(field));
                std::cout << "DEBUG: SUCCESS! Added union field: " << fieldType << " " << fieldName << std::endl;

            } catch (const std::exception &e) {
                std::cout << "DEBUG union field error: " << e.what() << std::endl;
                current = start;

                // Пропускаем токен чтобы не зациклиться
                if (!isAtEnd()) {
                    std::cout << "DEBUG union: skipping " << peek().getValue() << std::endl;
                    advance();
                }
            }
        }

        if (isAtEnd()) {
            throw std::runtime_error("Unexpected end of file while parsing union");
        }

        consume("CLOSECURLY", "Expected '}' after union");
        consume("SEMICOLON", "Expected ';' after union");

        std::cout << "DEBUG: Finished union with " << unionDecl->members.size() << " fields" << std::endl;
        return unionDecl;
    }

    std::unique_ptr<CppExpr> parsePostfix() {
        auto expr = parsePrimary();
        while (true) {
            if (match("INCREMENT")) {
                expr = makeUnary("++", std::move(expr), true);
            } else if (match("DECREMENT")) {
                expr = makeUnary("--", std::move(expr), true);
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<CppExpr> parsePrimary() {
        if (match("THIS")) {
            return std::make_unique<CppThisExpr>();
        }

        if (match("NULLPTR")) {
            return std::make_unique<CppNullptrLiteral>();
        }

        if (match("TRUE")) {
            auto lit = std::make_unique<CppBoolLiteral>();
            lit->value = true;
            return lit;
        }

        if (match("FALSE")) {
            auto lit = std::make_unique<CppBoolLiteral>();
            lit->value = false;
            return lit;
        }

        if (match("NEW")) {
            return parseNewExpr();
        }

        if (match("DELETE")) {
            return parseDeleteExpr();
        }

        if (match("SIZEOF")) {
            return parseSizeofExpr();
        }

        // Лямбда-выражения
        if (match("LAMBDA")) {
            return parseLambdaExpr();
        }

        // Базовые случаи (как в C парсере)
        if (match("OPENPARENTHESES")) {
            auto expr = parseExpression();
            consume("CLOSEPARENTHESES", "Expected ')'");
            return expr;
        }

        if (match("IDENTIFIER")) {
            return parseIdentifierExpr();
        }

        // Литералы
        if (match("VALUESTRING") || match("VALUECHAR") ||
            match("VALUEINTEGER") || match("VALUEFLOAT") || match("VALUEBOOL")) {
            return parseLiteral();
        }

        throw std::runtime_error("Unexpected token in expression");
    }

    CppExprPtr parseIdentifierExpr() {
        auto ident = std::make_unique<CppIdentifier>();
        ident->name = previous().getValue();

        CppExprPtr expr = std::move(ident);


        // Обработка квалифицированных имен (std::cout)
        while (match("SCOPE")) {
            if (check("IDENTIFIER")) {
                auto qualified = std::make_unique<CppMemberAccessExpr>();
                qualified->object = std::move(expr);
                qualified->member = advance().getValue();
                qualified->isPointerAccess = false;
                expr = std::move(qualified);
            }
        }

        // Обработка вызовов функций, доступ к массивам и полям
        while (true) {
            // Доступ к массиву
            if (match("OPENBRACKET")) {
                auto arrayAccess = std::make_unique<CppArrayAccessExpr>();
                arrayAccess->array = std::move(expr);
                arrayAccess->index = parseExpression();
                consume("CLOSEBRACKET", "Expected ']'");
                expr = std::move(arrayAccess);
            }
                // Доступ к полю
            else if (match("MEMBERACCESS") || match("PTRACCESS")) {
                auto memberAccess = std::make_unique<CppMemberAccessExpr>();
                memberAccess->object = std::move(expr);
                memberAccess->member = consume("IDENTIFIER", "Expected member name").getValue();
                memberAccess->isPointerAccess = (previous().getType() == "PTRACCESS");
                expr = std::move(memberAccess);
            }
                // Вызов функции
            else if (match("OPENPARENTHESES")) {
                auto call = std::make_unique<CppCallExpr>();
                call->callee = std::move(expr);
                if (!check("CLOSEPARENTHESES")) {
                    do {
                        call->arguments.push_back(parseExpression());
                    } while (match("COMMA"));
                }
                consume("CLOSEPARENTHESES", "Expected ')' after arguments");
                expr = std::move(call);
            } else {
                break;
            }
        }

        return expr;
    }

    CppExprPtr parseNewExpr() {
        auto newExpr = std::make_unique<CppNewExpr>();

        if (match("OPENBRACKET")) {
            newExpr->isArray = true;
            consume("CLOSEBRACKET", "Expected ']'");
        }

        newExpr->typeName = parseTypeName();

        if (match("OPENPARENTHESES")) {
            if (!check("CLOSEPARENTHESES")) {
                do {
                    newExpr->arguments.push_back(parseExpression());
                } while (match("COMMA"));
            }
            consume("CLOSEPARENTHESES", "Expected ')'");
        }

        return newExpr;
    }

    CppExprPtr parseDeleteExpr() {
        auto deleteExpr = std::make_unique<CppDeleteExpr>();

        if (match("OPENBRACKET")) {
            deleteExpr->isArray = true;
            consume("CLOSEBRACKET", "Expected ']'");
        }

        deleteExpr->operand = parseExpression();
        return deleteExpr;
    }


    std::unique_ptr<CppExpr> parseLambdaExpr() {
        auto lambda = std::make_unique<CppLambdaExpr>();

        // Захваты
        if (match("LAMBDA_CAPTURE")) {
            // Упрощенная обработка захватов
            std::string capture = previous().getValue();
            // ... парсинг capture list
        }

        // Упрощенная обработка параметров - конвертируем CppParameter в unique_ptr<CppParameter>
        if (match("OPENPARENTHESES")) {
            auto params = parseParameters();
            for (auto &param: params) {
                auto paramPtr = std::make_unique<CppParameter>();
                *paramPtr = std::move(param);
                lambda->parameters.push_back(std::move(paramPtr));
            }
            consume("CLOSEPARENTHESES", "Expected ')'");
        }

        // Возвращаемый тип - создаем идентификатор
        if (match("ARROW")) {
            auto returnType = std::make_unique<CppIdentifier>();
            returnType->name = parseTypeName();
            lambda->returnType = std::move(returnType);
        }

        // Тело
        if (match("OPENCURLY")) {
            lambda->body = std::unique_ptr<CppCompoundStmt>(
                    static_cast<CppCompoundStmt *>(parseCompoundStmt().release()));
        } else {
            // Expression lambda
            auto body = std::make_unique<CppCompoundStmt>();
            auto returnStmt = std::make_unique<CppReturnStmt>();
            returnStmt->value = parseExpression();
            body->statements.push_back(std::move(returnStmt));
            lambda->body = std::move(body);
        }

        return lambda;
    }

    // ============ ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ============

    std::vector<CppParameter> parseParameters() {
        std::vector<CppParameter> params;

        if (check("CLOSEPARENTHESES")) {
            std::cout << "DEBUG: Empty parameter list" << std::endl;
            return params;
        }

        do {
            CppParameter param;
            size_t paramStart = current;

            try {
                // 🔥 УПРОЩЕННЫЙ ПАРСИНГ ТИПА - избегаем рекурсивных вызовов
                param.typeName = parseSimpleTypeName();

                // Имя параметра (может отсутствовать)
                if (check("IDENTIFIER")) {
                    param.name = advance().getValue();
                }

                // Значение по умолчанию - пропускаем для упрощения
                if (match("ASSIGN")) {
                    std::cout << "DEBUG: Skipping default value for parameter" << std::endl;
                    skipUntilCommaOrParen();
                }

                params.push_back(param);
                std::cout << "DEBUG: Added parameter: " << param.typeName << " " << param.name << std::endl;
            } catch (const std::exception &e) {
                // Если не удалось распарсить параметр, пропускаем его
                std::cout << "DEBUG: Parameter parsing failed: " << e.what() << std::endl;
                current = paramStart;
                skipUntilCommaOrParen();
            }
        } while (match("COMMA"));
        std::cout << "DEBUG: Finished parsing " << params.size() << " parameters" << std::endl;

        return params;
    }

    void skipUntilCommaOrSemicolon() {
        int parenLevel = 0;
        while (!isAtEnd()) {
            if (check("OPENPARENTHESES")) {
                parenLevel++;
            } else if (check("CLOSEPARENTHESES")) {
                if (parenLevel == 0) break;
                parenLevel--;
            } else if ((check("COMMA") || check("SEMICOLON")) && parenLevel == 0) {
                break;
            }
            advance();
        }
    }

    std::string parseSimpleTypeName() {
        std::string typeName;
        size_t start = current;
        try {
            // Только базовые квалификаторы
            while (match("CONST") || match("VOLATILE")) {
                if (!typeName.empty()) typeName += " ";
                typeName += previous().getValue();
            }

            // Базовый тип
            if (isTypeToken(peekType()) || check("IDENTIFIER")) {
                if (!typeName.empty()) typeName += " ";
                typeName += advance().getValue();
            } else {
                throw std::runtime_error("Expected type in parameter");
            }

            // Простые указатели/ссылки
            while (match("MULTI") || match("BITAND") || match("BITANDAND")) {
                if (previous().getType() == "MULTI") typeName += "*";
                else if (previous().getType() == "BITAND") typeName += "&";
                else if (previous().getType() == "BITANDAND") typeName += "&&";
            }
            if (typeName.empty()) {
                throw std::runtime_error("Failed to parse type");
            }

            std::cout << "DEBUG: Successfully parsed simple type: " << typeName << std::endl;

            return typeName;
        } catch (const std::exception &e) {
            current = start;
            std::cout << "DEBUG: Failed to parse simple type: " << e.what() << std::endl;
            throw;
        }
    }

    void skipUntilCommaOrParen() {
        while (!isAtEnd() && !check("COMMA") && !check("CLOSEPARENTHESES")) {
            advance();
        }
    }

    std::string parseTypeName() {
        size_t start = current;
        std::string typeName;

        try {
            // Сначала проверяем квалификаторы
            static const std::unordered_set<std::string> qualifiers = {
                    "const", "volatile", "static", "mutable", "register",
                    "signed", "unsigned", "short", "long"
            };

            // Пропускаем квалификаторы
            while (!isAtEnd() && qualifiers.count(peek().getValue()) > 0) {
                if (!typeName.empty()) typeName += " ";
                typeName += advance().getValue();
            }

            // Теперь основной тип
            if (isAtEnd()) {
                throw std::runtime_error("Unexpected end while parsing type");
            }

            // Простой подход: берем идентификатор как тип
            if (check("IDENTIFIER") || isTypeToken(peekType())) {
                if (!typeName.empty()) typeName += " ";
                typeName += advance().getValue();
            } else {
                throw std::runtime_error("Expected type identifier");
            }

            // Указатели и ссылки
            while (!isAtEnd() &&
                   (check("MULTI") || check("BITAND") || check("BITANDAND"))) {
                if (peek().getValue() == "*") typeName += "*";
                else if (peek().getValue() == "&") typeName += "&";
                else if (peek().getValue() == "&&") typeName += "&&";
                advance();
            }

            std::cout << "DEBUG: Parsed type: " << typeName << std::endl;
            return typeName;

        } catch (const std::exception &e) {
            current = start;
            std::cout << "DEBUG: parseTypeName failed: " << e.what() << std::endl;
            throw;
        }
    }

    CppStmtPtr parseThrowStmt() {
        auto throwExpr = parseExpression();
        consume("SEMICOLON", "Expected ';' after throw");

        auto stmt = std::make_unique<CppExprStmt>();
        stmt->expression = std::move(throwExpr);
        return stmt;
    }

    std::unique_ptr<CppStmt> parseStatement() {
        if (isAtEnd()) {
            std::cout << "DEBUG parseStatement: at end" << std::endl;
            return nullptr;
        }
        // Отладочный вывод
        std::cout << "DEBUG parseStatement: " << peek().getValue() << " (" << peek().getType() << ")" << std::endl;


        if (match("IF")) return parseIfStmt();
        if (match("WHILE")) return parseWhileStmt();
        if (match("FOR")) return parseForStmt();
        if (match("DO")) return parseDoWhileStmt();
        if (match("SWITCH")) return parseSwitchStmt();
        if (match("RETURN")) return parseReturnStmt();
        if (match("BREAK")) return parseBreakStmt();
        if (match("CONTINUE")) return parseContinueStmt();
        if (match("TRY")) return parseTryStmt();
        if (match("THROW")) return parseThrowStmt();
        if (match("OPENCURLY")) return parseCompoundStmt();
        if (match("SEMICOLON")) return nullptr; // empty statement

        // Объявления переменных
        if (isTypeToken(peekType()) || check("AUTO")) {
            return parseVarDecl();
        }

        // Выражения
        try {
            auto expr = parseExpression();
            if (expr) {
                auto stmt = std::make_unique<CppExprStmt>();
                stmt->expression = std::move(expr);
                if (match("SEMICOLON")) {
                    return stmt;
                } else {
                    throw std::runtime_error("Expected ';' after expression");
                }
            }
        } catch (const std::exception &e) {
            std::cout << "Error parsing expression: " << e.what() << std::endl;
        }

        // Если ничего не распарсилось, пропускаем токен
        std::cout << "Skipping token in statement: " << peek().getValue() << std::endl;
        advance();
        return nullptr;
    }

    CppStmtPtr parseIfStmt() {
        consume("OPENPARENTHESES", "Expected '(' after if");
        auto condition = parseExpression();
        consume("CLOSEPARENTHESES", "Expected ')' after condition");

        auto thenBranch = parseStatement();
        CppStmtPtr elseBranch;
        if (match("ELSE")) {
            elseBranch = parseStatement();
        }

        auto stmt = std::make_unique<CppIfStmt>();
        stmt->condition = std::move(condition);
        stmt->thenBranch = std::move(thenBranch);
        stmt->elseBranch = std::move(elseBranch);
        return stmt;
    }

    CppStmtPtr parseWhileStmt() {
        consume("OPENPARENTHESES", "Expected '(' after while");
        auto condition = parseExpression();
        if (!condition) {
            throw std::runtime_error("Условие while не может быть nullptr");
        }
        consume("CLOSEPARENTHESES", "Expected ')' after condition");
        auto body = parseStatement();

        auto stmt = std::make_unique<CppWhileStmt>();
        stmt->condition = std::move(condition);
        stmt->body = std::move(body);
        return stmt;
    }

    CppStmtPtr parseForStmt() {
        consume("OPENPARENTHESES", "Expected '(' after for");

        // Инициализация
        CppStmtPtr init;
        if (!check("SEMICOLON")) {
            if (isTypeToken(peekType()) || check("AUTO")) {
                init = parseVarDecl(false); // без точки с запятой
            } else {
                init = parseExprStmt(false); // без точки с запятой
            }
        }

        if (!match("SEMICOLON")) {
            throw std::runtime_error("Expected ';' after initialization");
        }

        // Условие
        CppExprPtr condition;
        if (!check("SEMICOLON")) {
            condition = parseExpression();
        }

        if (!match("SEMICOLON")) {
            throw std::runtime_error("Expected ';' after condition");
        }
        // Инкремент
        CppExprPtr increment;
        if (!check("CLOSEPARENTHESES")) {
            increment = parseExpression();
        }
        consume("CLOSEPARENTHESES", "Expected ')' after for");

        auto body = parseStatement();

        auto stmt = std::make_unique<CppForStmt>();
        stmt->init = std::move(init);
        stmt->condition = std::move(condition);
        stmt->increment = std::move(increment);
        stmt->body = std::move(body);
        return stmt;
    }

    CppStmtPtr parseDoWhileStmt() {
        auto stmt = std::make_unique<CppDoWhileStmt>();

        // Парсим тело do
        stmt->body = parseStatement();
        if (!stmt->body) {
            throw std::runtime_error("Expected statement after 'do'");
        }

        // Ожидаем 'while'
        if (!match("WHILE")) {
            throw std::runtime_error("Expected 'while' after do body");
        }

        // Условие в скобках
        consume("OPENPARENTHESES", "Expected '(' after while");
        stmt->condition = parseExpression();
        consume("CLOSEPARENTHESES", "Expected ')' after condition");
        consume("SEMICOLON", "Expected ';' after do-while");

        return stmt;
    }

    CppStmtPtr parseCase() {
        auto caseStmt = std::make_unique<CppCaseStmt>();

        // Парсим значение case (может быть выражением)
        if (!check("COLON")) {
            caseStmt->value = parseExpression();
        }

        consume("COLON", "Ожидалось ':' после case");

        // Парсим тело case до следующего case/default/}
        auto bodyBlock = std::make_unique<CppCompoundStmt>();
        while (!isAtEnd() && !check("CASE") && !check("DEFAULT") && !check("CLOSECURLY")) {
            if (auto stmt = parseStatement()) {
                bodyBlock->statements.push_back(std::move(stmt));
            } else {
                advance();
            }
        }

        caseStmt->body = std::move(bodyBlock->statements);
        return caseStmt;
    }

    CppStmtPtr parseDefault() {
        auto defaultStmt = std::make_unique<CppDefaultStmt>();

        consume("COLON", "Ожидалось ':' после default");

        // Парсим тело default до следующего case/default/}
        auto bodyBlock = std::make_unique<CppCompoundStmt>();
        while (!isAtEnd() && !check("CASE") && !check("DEFAULT") && !check("CLOSECURLY")) {
            if (auto stmt = parseStatement()) {
                bodyBlock->statements.push_back(std::move(stmt));
            } else {
                advance();
            }
        }

        defaultStmt->body = std::move(bodyBlock->statements);
        return defaultStmt;
    }

    CppStmtPtr parseSwitchStmt() {
        consume("OPENPARENTHESES", "Ожидалась '(' после switch");
        auto condition = parseExpression();
        consume("CLOSEPARENTHESES", "Ожидалась ')' после условия switch");
        consume("OPENCURLY", "Ожидался '{' после switch");

        auto switchStmt = std::make_unique<CppSwitchStmt>();
        switchStmt->condition = std::move(condition);

        // Упрощенно: пропускаем содержимое switch
        while (!isAtEnd() && !check("CLOSECURLY")) {
            if (match("CASE")) {
                auto caseStmt = parseCase();
                switchStmt->cases.push_back(
                        std::unique_ptr<CppCaseStmt>(static_cast<CppCaseStmt *>(caseStmt.release())));
            } else if (match("DEFAULT")) {
                auto defaultStmt = parseDefault();
                switchStmt->cases.push_back(
                        std::unique_ptr<CppCaseStmt>(static_cast<CppCaseStmt *>(defaultStmt.release())));
            } else {
                advance();
            }
        }

        consume("CLOSECURLY", "Ожидался '}' после switch");
        return switchStmt;
    }

    CppStmtPtr parseReturnStmt() {
        auto stmt = std::make_unique<CppReturnStmt>();
        if (!check("SEMICOLON")) {
            stmt->value = parseExpression();
        }
        consume("SEMICOLON", "Expected ';' after return");
        return stmt;
    }

    CppStmtPtr parseBreakStmt() {
        auto stmt = std::make_unique<CppBreakStmt>();
        consume("SEMICOLON", "Expected ';' after break");
        return stmt;
    }

    CppStmtPtr parseContinueStmt() {
        auto stmt = std::make_unique<CppContinueStmt>();
        consume("SEMICOLON", "Expected ';' after continue");
        return stmt;
    }

    CppStmtPtr parseExprStmt(bool requireSemicolon = true) {
        auto expr = parseExpression();
        if (requireSemicolon) {
            consume("SEMICOLON", "Expected ';' after expression");
        }
        auto stmt = std::make_unique<CppExprStmt>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    CppStmtPtr parseVarDecl(bool requireSemicolon = true) {
        size_t save = current;

        try {
            std::string typeName = parseTypeName();

            if (!check("IDENTIFIER")) {
                current = save;
                return nullptr;
            }

            // Если это множественное объявление, создаем блок
            auto firstDecl = std::make_unique<CppVarDecl>();
            firstDecl->typeName = typeName;
            firstDecl->name = advance().getValue();

            // Инициализатор для первой переменной
            if (match("ASSIGN")) {
                firstDecl->initializer = parseExpression();
            }

            std::vector<std::unique_ptr<CppStmt>> declarations;
            declarations.push_back(std::move(firstDecl));

            // Обрабатываем дополнительные объявления через запятую
            while (match("COMMA")) {
                if (!check("IDENTIFIER")) break;

                auto additionalDecl = std::make_unique<CppVarDecl>();
                additionalDecl->typeName = typeName; // тот же тип
                additionalDecl->name = advance().getValue();

                if (match("ASSIGN")) {
                    additionalDecl->initializer = parseExpression();
                }

                declarations.push_back(std::move(additionalDecl));
            }

            if (requireSemicolon) {
                consume("SEMICOLON", "Expected ';' after declaration");
            }

            // Возвращаем одиночное объявление или блок
            if (declarations.size() == 1) {
                return std::move(declarations[0]);
            } else {
                auto block = std::make_unique<CppCompoundStmt>();
                for (auto &decl: declarations) {
                    block->statements.push_back(std::move(decl));
                }
                return block;
            }

        } catch (const std::exception &e) {
            current = save;
            return nullptr;
        }
    }

    CppStmtPtr parseCompoundStmt() {
        std::cout << "DEBUG: Starting parseCompoundStmt" << std::endl;
        auto block = std::make_unique<CppCompoundStmt>();

        while (!isAtEnd() && !check("CLOSECURLY")) {
            if (auto stmt = parseStatement()) {
                block->statements.push_back(std::move(stmt));
            } else {
                std::cout << "DEBUG: Skipping token in compound stmt: " << peek().getValue() << std::endl;
                advance();
            }
        }
        if (isAtEnd()) {
            throw std::runtime_error("Unexpected end of file while parsing compound statement");
        }

        consume("CLOSECURLY", "Expected '}'");
        std::cout << "DEBUG: Finished parseCompoundStmt with " << block->statements.size() << " statements"
                  << std::endl;

        return block;
    }

    // ============ УТИЛИТЫ ============
    bool isPreprocessor(const std::string &type) {
        return type == "INCLUDE" || type == "DEFINE" ||
               type == "IFDEF" || type == "IFNDEF" || type == "ENDIF";
    }

    bool isTypeToken(const std::string &type) {
        static const std::vector<std::string> typeTokens = {
                "VOID", "INT", "FLOAT", "DOUBLE", "CHAR", "BOOL", "SHORT", "LONG",
                "SIGNED", "UNSIGNED", "AUTO", "CONST", "VOLATILE", "STATIC",
                "CLASS", "STRUCT", "ENUM", "TYPENAME", "UNION"
        };
        return std::find(typeTokens.begin(), typeTokens.end(), type) != typeTokens.end();
    }

    bool isFunctionStart() {
        size_t save = current;

        if (check("BITNOT") && lookAhead(1).getType() == "IDENTIFIER") {
            current = save;
            return true;
        }

        try {
            // Пытаемся распарсить тип
            std::string typeName = parseTypeName();

            // Проверяем, что дальше идет имя и открывающая скобка
            if (check("IDENTIFIER") || check("OPERATOR")) {
                std::string name = peek().getValue();
                advance(); // потребляем имя

                // Это функция только если есть открывающая скобка
                if (check("OPENPARENTHESES")) {
                    current = save;
                    return true;
                }
            }
        } catch (...) {
            // Игнорируем ошибки
        }

        current = save;
        return false;
    }

    bool isGlobalVarStart() {
        size_t save = current;

        try {
            // Пытаемся распарсить тип
            std::string typeName = parseTypeName();

            if (!check("IDENTIFIER")) {
                current = save;
                return false;
            }

            // Продвигаемся для анализа контекста
            advance(); // идентификатор переменной

            // Это объявление переменной если не функция
            bool isVar = !check("OPENPARENTHESES");

            current = save;
            return isVar;

        } catch (const std::exception &e) {
            // Если не распарсился как известный тип, проверяем пользовательский тип
            current = save;

            if (check("IDENTIFIER")) {
                std::string potentialType = peek().getValue();

                // Проверяем, известен ли этот идентификатор как тип
                if (knownClasses.count(potentialType) > 0) {
                    advance(); // потребляем идентификатор типа

                    // Проверяем, что следующий токен - идентификатор переменной
                    if (check("IDENTIFIER")) {
                        current = save; // восстанавливаем позицию
                        return true;
                    }
                }
            }
            current = save;
            return false;
        }
    }

    bool isDeclarationStart(const std::string &type) {

        return isTypeToken(type) || type == "CLASS" || type == "STRUCT" ||
               type == "ENUM" || type == "TYPEDEF" || type == "USING" ||
               type == "TEMPLATE" || type == "NAMESPACE" || type == "UNION";
    }

    // Token matching utilities (аналогично C парсеру)
    bool match(const std::string &type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool check(const std::string &type) {
        return !isAtEnd() && tokens[current].getType() == type;
    }

    Token advance() {
        return isAtEnd() ? Token() : tokens[current++];
    }

    Token &peek() {
        static Token dummy;
        return isAtEnd() ? dummy : tokens[current];
    }

    std::string peekType() {
        return isAtEnd() ? "" : tokens[current].getType();
    }

    bool isAtEnd() {
        return current >= tokens.size();
    }

    Token consume(const std::string &type, const std::string &message) {
        if (check(type)) return advance();
        throw std::runtime_error(message);
    }
};


#endif //AISDLAB_CPPPARSERTOAST_H