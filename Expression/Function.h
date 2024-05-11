//
// Created by shuri on 08.05.2024.
//

#ifndef FUNCTION_H
#define FUNCTION_H
#include "Expression.h"
#include "../Token.h"
#include "ConditionExpression.h"
#include "StatementExpression.h"
#include <utility>
#include <vector>
static int globalPos=0; //только для вложенных случаев нужен
class Function: public Expression {
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> declaration;
    std::vector<Token> localList;

public:
    Function(int pos, vector<Token> list){
        doFunction(pos,std::move(list));
    }
    Function( const Function& ex){
        this->declaration=ex.declaration;
        this->expressionList=ex.expressionList;
    }
    void doFunction(int pos, vector<Token> list){
        globalPos=pos;
        while(list[globalPos].getValue()!="SEMICOLON"){
            declaration.push_back(list[pos]);
            globalPos++;
        }
        globalPos++; globalPos++;
        while(list[globalPos].getType()!="ENDofCycle"){
            if(((list[globalPos].getType()=="CONDITION"))|| //если хоть какую-то в нем вложенность находим
               (list[globalPos].getType()=="CYCLEFOR")|| // хоть вложенное условие, хоть вложенный цикл, то создаём новый объект
               (list[globalPos].getType()=="CYCLEWHILE")|| // не забываем про static переменную, она указывает новое место где мы окажемся
               (list[globalPos].getType()=="CYCLEDOWHILE"))//поднявшись обратно наверх от вложенного объекта
            {
                ConditionExpression* cx = new ConditionExpression(globalPos,list);
                globalPos=cx->getGlobalPos();
                expressionList.push_back(cx);
            }
            else
            {
                while(list[globalPos].getType()!="SEMICOLON"){ //если вложенности нет или мы с ней уже закончили, то формируем обычные выражения
                    localList.push_back(list[globalPos]);
                    globalPos++;}
                StatementExpression* rx= new StatementExpression(localList);
                expressionList.push_back(rx);
                localList.clear();
                globalPos++;
            }
        }
        globalPos++;
        return;
    }
    void print(int tab) override{}
};


#endif //FUNCTION_H
