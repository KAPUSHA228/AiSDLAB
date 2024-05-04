//
// Created by egorm on 21.04.2024.
//
#ifndef PARSER_H
#define PARSER_H
#include <bits/stdc++.h>
#include "stdexcept"
#include "Lexer.h"
#include "Token.h"
#include "HierarchyList.h"
#include "Expression/Expression.h"
#include "Expression/StatementExpression.h"
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
    HierarchyList<string,Expression*>hierarchyList;
    std::vector<Token> tokenList;
    std::vector<Expression*> expressionList;
    std::vector<Token> localList;
    int currentPos = 0;
public:
    Parser(Lexer lexer) {
        this->tokenList = lexer.getTokenList();
    }
    Parser(vector<Token> t){this->tokenList=t;}
    void parse() {
        initDeclaration();
        return;
    }
    void initDeclaration(){
        if(isTypeToken("TITLE")){
            while(!isTypeToken("SEMICOLON")){
                currentPos++;} //скип названия, чтобы дойти до разделов Const или Var
            currentPos++;}       //есть вариант вообще название не добавлять уже на стадии Lexer, но мб пригодится

        if(isTypeToken("CONST")){
            currentPos++;
            while (!isTypeToken("VAR")){
                initRowStatement();
                hierarchyList.toAddNext(expressionList.front(),"Const");
                expressionList.clear();
            }
        }
        if(isTypeToken("VAR")){
            currentPos++;
            while (!isTypeToken("BEGIN")){
                initRowStatement();
                hierarchyList.toAddNext(expressionList.front(),"Var");
                expressionList.clear();

            }
            currentPos++;
        }
        initBegin();
        return;
    }
    void print(){
        for(auto item:expressionList){
            item->print();
           }
    }
    void initRowStatement(){//метод чтобы строчку кода (не условие и не цикл) переводить в StatementExpression
        while(!isTypeToken("SEMICOLON")){
            localList.push_back( tokenList[currentPos]);
            currentPos++; }
        StatementExpression* rx=new StatementExpression(localList);
        expressionList.push_back(rx);
        localList.clear();
        currentPos++;
        return;
    }
    void initBegin() {
        std::vector<Token> condition;
        while (!isTypeToken("ENDofPROGRAM")) {
            if ((isTypeToken("CONDITION"))||(isTypeToken("UNCONDITION"))||(isTypeToken("CYCLEFOR"))||
                (isTypeToken("CYCLEWHILE"))||(isTypeToken("CYCLEDOWHILE")))
            {
                ConditionExpression *cx = new ConditionExpression(currentPos, tokenList);
                currentPos = cx->getGlobalPos();
                expressionList.push_back(cx);
                hierarchyList.toAddNext(expressionList.front(),"Body");
                expressionList.clear();

            }
            else
            {
                initRowStatement();
                hierarchyList.toAddNext(expressionList.front(),"Body");
                expressionList.clear();
            }
        }
        return;
    }
    bool isTypeToken(const string &typeToken) {
        return tokenList[currentPos].getType() == typeToken;
    }
};
#endif //PARSER_H
