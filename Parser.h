//
// Created by egorm on 21.04.2024.
//

#ifndef PARSER_H
#define PARSER_H
#include<bits/stdc++.h>

#include "Lexer.h"
#include "Token.h"
#include "Expression/ExceptionAST.h"
#include "Expression/Expression.h"
using namespace std;
class AgeException
{
public:
    AgeException(std::string message): message{message}{}
    std::string getMessage() const {return message;}
private:
    std::string message;
};
class Parser {
private:
    std::vector<Token> tokenList;
    std::vector<Expression> expressionList;
    int currentPos = 0;

public:
    explicit Parser(Lexer lexer) {
        this->tokenList = lexer.getTokenList();
    }

    void parse() {
        initVar();
    }

    void initVar() {
        if (isTypeToken("VAR")) {
            currentPos++;
            while (!isTypeToken("BEGIN")) {
                initVariables();
            }
        } else {
            throw "Don't exist VAR!";
        }
    }
    void initVariables() {
        while(!isTypeToken(";")) {
            if(!isTypeToken("VARIABLE")) {
                throw "Var is expected!";
            }
            else {
                currentPos++;
            }
        }
    }
    bool isTypeToken(const string &typeToken) {
        return tokenList[currentPos].getType() == typeToken;
    }
};
#endif //PARSER_H
