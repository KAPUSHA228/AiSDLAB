//
// Created by egorm on 21.04.2024.
//

#ifndef PARSER_H
#define PARSER_H
#include<bits/stdc++.h>
#include "stdexcept"
#include "Lexer.h"
#include "Token.h"
#include "Expression/ExceptionAST.h"
#include "Expression/Expression.h"
static bool switcher= false;
using namespace std;
class AgeException: public std::exception
{
public:
    AgeException(const std::string& message): message(message){}
    std::string getMessage() const {std::cout<<message; return message;}
    //const char * what() const throw() override {
    //    return message.c_str();
   // }

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
void init(){
        std::vector<Token>localList;
        while(!isTypeToken(";")){
           localList.push_back( tokenList[currentPos]);
           currentPos++;
        }
    }
    void initVar() {
        if (isTypeToken("VAR")) {
            currentPos++;
            while (!isTypeToken("BEGIN")) {
                initVariables(); }}
        else { throw AgeException( "Don't exist VAR!"); }
    }
    void initVariables() {
        while(!isTypeToken("SEMICOLON")) {
            if(isTypeToken("SPACE")) continue;
            if(!isTypeToken("VARIABLE")) {
                throw AgeException("Var is expected!");}
            else { currentPos++; initAssign();}
        }
    }
    void initAssign(){
        while(!isTypeToken("SEMICOLON")){
            if(isTypeToken("SPACE")) continue;
            if (isTypeToken("COLON")){
                currentPos++; initType();  }
            else{throw AgeException("assign is not present"); }
        }
    }
    void initType(){
        while(!isTypeToken("SEMICOLON")){
            if(isTypeToken("SPACE")) continue;
            if((isTypeToken("TYPEINTEGER"))||
            (isTypeToken("TYPEREAL"))||
            (isTypeToken("TYPECHAR"))||
            (isTypeToken("TYPESTRING")))
            {throw AgeException("type is present");}
            else{currentPos++;}
        }
    }
    bool isTypeToken(const string &typeToken) {
        return tokenList[currentPos].getType() == typeToken;
    }
};
#endif //PARSER_H
