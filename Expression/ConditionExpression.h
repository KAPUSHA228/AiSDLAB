//
// Created by shuri on 26.04.2024.
//
#ifndef CONDITIONEXPRESSION_H
#define CONDITIONEXPRESSION_H
#include <vector>
#include "Expression.h"
#include "../Parser.h"
#include <bits/stdc++.h>
#include "stdexcept"
#include "../Lexer.h"
#include "../Token.h"
#include "../Postfix.h"

static int posofEndofIf=0; //только для вложенных случаев нужен
class ConditionExpression: public Expression{
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> condition;
    std::vector<Token> localList;
public:
    ConditionExpression(int pos, vector<Token>list){
        posofEndofIf=pos;
        if(list[posofEndofIf].getType()=="CONDITION"){
            posofEndofIf++;
            while(list[posofEndofIf].getType()!="BEGIN"){
                condition.push_back(list[posofEndofIf]);
                posofEndofIf++;
            }
            TPostfixCalc p(condition);//мб сделать какой-то метод именно для проверки условий
            posofEndofIf++;

            while(list[posofEndofIf].getType()!="ENDofIF"){ //пока не дойдём до конца тела текущего if
                if((list[posofEndofIf].getType()=="CONDITION")|| //если хоть какую-то в нем вложенность находим
                (list[posofEndofIf].getType()=="CICLEFOR")||// хоть вложенное условие, хоть вложенный цикл, то создаём новый объект
                (list[posofEndofIf].getType()=="CICLEWHILE")||// не забываем про static переменную, она указывает новое место где мы окажемся
                (list[posofEndofIf].getType()=="CICLEDOWHILE")){ //поднявшись обратно наверх от вложенного объекта
                    ConditionExpression ex(posofEndofIf,list);}
                while(list[posofEndofIf].getType()!="SEMICOLON"){//если вложенности нет или мы с ней уже закончили, то формируем обычные выражения
                    localList.push_back(list[posofEndofIf]);
                    posofEndofIf++;}
                RunnableExpression ex(localList);
                expressionList.push_back(&ex);
                localList.clear();
                posofEndofIf++;}
            posofEndofIf++;
        }
        if(list[posofEndofIf].getType()=="UNCONDITION"){
            posofEndofIf++;
            TPostfixCalc p(condition);// а здесь отрицание этого метода, довольно удобно получится

            while(list[posofEndofIf].getType()!="ENDofCicle"){ // те же шаги, что и при condition, но уже в цикле до ENDofCicle
                if((list[posofEndofIf].getType()=="CONDITION")||
                   (list[posofEndofIf].getType()=="CICLEFOR")||
                   (list[posofEndofIf].getType()=="CICLEWHILE")||
                   (list[posofEndofIf].getType()=="CICLEDOWHILE")){
                    ConditionExpression ex(posofEndofIf,list);}
                while(list[posofEndofIf].getType()!="SEMICOLON"){
                    localList.push_back(list[posofEndofIf]);
                    posofEndofIf++;}
                RunnableExpression ex(localList);
                expressionList.push_back(&ex);
                localList.clear();
                posofEndofIf++;}
            posofEndofIf++;
        }
        if(list[posofEndofIf].getType()=="CICLEFOR"){
            int i1,i2; int i=posofEndofIf;
            while(list[i].getType()!="ASSIGN"){  i++;  }
            i1=std::stoi(list[i+1].getValue());
            i2=std::stoi(list[i+3].getValue());
            posofEndofIf=i+5;
            if( list[i+2].getValue()=="to"){//или можно сделать if(i1<i2){} else{}, не знаю что быстрее,
                //но наверно второе потому что обратиться к локальной переменной всяко быстрее
                //чем лезть по индексу в вектор и у него брать поле
                for (i1;i1<i2;i1++){
                    if((list[posofEndofIf].getType()=="CONDITION")||
                       (list[posofEndofIf].getType()=="CICLEFOR")||
                       (list[posofEndofIf].getType()=="CICLEWHILE")||
                       (list[posofEndofIf].getType()=="CICLEDOWHILE")){
                        ConditionExpression ex(posofEndofIf,list);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                            localList.push_back(list[posofEndofIf]);
                            posofEndofIf++;}
                        RunnableExpression ex(localList);
                        expressionList.push_back(&ex);
                        localList.clear();
                        posofEndofIf++;
                }
            }
            else{
                for (i1;i1>i2;i2--){
                    if((list[posofEndofIf].getType()=="CONDITION")||
                       (list[posofEndofIf].getType()=="CICLEFOR")||
                       (list[posofEndofIf].getType()=="CICLEWHILE")||
                       (list[posofEndofIf].getType()=="CICLEDOWHILE")){
                        ConditionExpression ex(posofEndofIf,list);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                        localList.push_back(list[posofEndofIf]);
                        posofEndofIf++;}
                    RunnableExpression ex(localList);
                    expressionList.push_back(&ex);
                    localList.clear();
                    posofEndofIf++;
                }
            }
        }
        if(list[posofEndofIf].getType()=="CICLEWHILE"){ }
        if(list[posofEndofIf].getType()=="CICLEDOWHILE"){ }
    }
    void toSolve(){

    }
    void run() override {
       // for (auto Expression: expressionList) {
         //   Expression.run();
        //}
    }
};


#endif //CONDITIONEXPRESSION_H
