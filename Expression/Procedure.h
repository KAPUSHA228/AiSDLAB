//
// Created by shuri on 11.05.2024.
//

#ifndef PROCEDURE_H
#define PROCEDURE_H
#include "Expression.h"
#include "../Token.h"
#include "ConditionExpression.h"
#include "StatementExpression.h"
#include <utility>
#include <vector>
static int globalPosProc=0; //только для вложенных случаев нужен
class Procedure: public  Expression{
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> declaration;
    std::vector<Token> localList;
public:
    Procedure(int pos, vector<Token> list){
        doProcedure(pos,std::move(list));
    }
    Procedure( const Procedure& ex){
        this->declaration=ex.declaration;
        this->expressionList=ex.expressionList;
    }
    void doProcedure(int pos, vector<Token> list){
        globalPosProc=pos;
        while(list[globalPosProc].getValue() != "SEMICOLON"){
            declaration.push_back(list[pos]);
            globalPosProc++;
        }
        globalPosProc++; globalPosProc++;
        while(list[globalPosProc].getType() != "ENDofCycle"){
            if(((list[globalPosProc].getType() == "CONDITION")) || //если хоть какую-то в нем вложенность находим
               (list[globalPosProc].getType() == "CYCLEFOR") || // хоть вложенное условие, хоть вложенный цикл, то создаём новый объект
               (list[globalPosProc].getType() == "CYCLEWHILE") || // не забываем про static переменную, она указывает новое место где мы окажемся
               (list[globalPosProc].getType() == "CYCLEDOWHILE"))//поднявшись обратно наверх от вложенного объекта
            {
                ConditionExpression* cx = new ConditionExpression(globalPosProc, list);
                globalPosProc=cx->getGlobalPos();
                expressionList.push_back(cx);
            }
            else
            {
                while(list[globalPosProc].getType() != "SEMICOLON"){ //если вложенности нет или мы с ней уже закончили, то формируем обычные выражения
                    localList.push_back(list[globalPosProc]);
                    globalPosProc++;}
                StatementExpression* rx= new StatementExpression(localList);
                expressionList.push_back(rx);
                localList.clear();
                globalPosProc++;
            }
        }
        globalPosProc++;
        return;
    }
    void print(int tab) override{}
    int getPos(){ return globalPosProc;}
};


#endif //PROCEDURE_H
