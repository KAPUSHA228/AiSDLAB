//
// Created by egorm on 21.04.2024.
//

#ifndef PARSER_H
#define PARSER_H
#include<bits/stdc++.h>
#include "stdexcept"
#include "Lexer.h"
#include "Token.h"
#include "HierarchyList.h"
#include "Expression/ExceptionAST.h"
#include "Expression/Expression.h"
#include "Expression/RunnableExpression.h"
#include "Expression/ConditionExpression.h"
using namespace std;
class AgeException: public std::exception
{
public:
    AgeException(const std::string& message): message(message){}
    std::string getMessage() const {std::cout<<message; return message;}
private:
    std::string message;
};
class Parser {
private:
    std::vector<Token> tokenList;
    std::vector<Expression*> expressionList;
    //HierarchyList hierarchyList;
    std::vector<Token>localList; //вектор для хранения строки которая пойдёт цельно в выражение
    int currentPos = 0;
public:
    explicit Parser(Lexer lexer) {
        this->tokenList = lexer.getTokenList();
    }
    void parse() {
        initDeclaration();
    }
    void initDeclaration(){
        if(isTypeToken("TITLE")){
            while(!isTypeToken("SEMICOLON"))
            {   currentPos++;   } //скип названия, чтобы дойти до разделов Const или Var
            currentPos++; }       //есть вариант вообще название не добавлять уже на стадии Lexer, но мб пригодится

        if((isTypeToken("VAR"))||(isTypeToken("CONST"))){
            currentPos++;
            while (!isTypeToken("BEGIN")){
                initRowStatement();
            }
        }
        currentPos++;
        initBegin();
    }
    void initBegin(){
        std::vector<Token> condition;
        if((isTypeToken("CONDITION"))||(isTypeToken("CYCLEFOR"))||
        (isTypeToken("CYCLEWHILE"))||(isTypeToken("CYCLEDOWHILE"))){
            ConditionExpression ex(currentPos,tokenList);
        }
        else{
            initRowStatement();
            initBegin();
            return;}
        return;

    }
    void initRowStatement(){//метод чтобы строчку кода (не условие и не цикл) переводить в RunnableExpression
        while(!isTypeToken("SEMICOLON")){
            if(isTypeToken("VAR")) continue;
            localList.push_back( tokenList[currentPos]);
            currentPos++; }
        RunnableExpression ex (localList);
        expressionList.push_back(&ex);
        localList.clear();
        currentPos++;
    }
    /*void initVar() {
        if (isTypeToken("VAR")) {
            currentPos++;
            while (!isTypeToken("BEGIN")) {
                initVariables(); }
        }
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
            if (isTypeToken("COLON"))
            {currentPos++; initType(); }
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
    }*/
    bool isTypeToken(const string &typeToken) {
        return tokenList[currentPos].getType() == typeToken;
    }
};
#endif //PARSER_H
