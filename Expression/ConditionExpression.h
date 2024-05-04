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
static int y=0;
static int posofEndofIf=0; //только для вложенных случаев нужен
class ConditionExpression: public Expression{
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> condition;
    std::vector<Token> localList;
public:
    static int getGlobalPos(){return posofEndofIf;}
    ConditionExpression(int pos, vector<Token>list){ doCondition(pos,list);}
    ConditionExpression(ConditionExpression& ex){
        this->condition=ex.condition;
        this->expressionList=ex.expressionList;
    }
    vector<Token> getList(){return condition;}
    vector<Expression*> getBody(){return expressionList;}
    void doCondition(int pos, vector<Token>list){
        posofEndofIf=pos;
        if(list[posofEndofIf].getType()=="CONDITION")
        {
            while(list[posofEndofIf].getType()!="BEGIN")
            {
                condition.push_back(list[posofEndofIf]);
                posofEndofIf++;
            }
            //TPostfixCalc p(condition);//мб сделать какой-то метод именно для проверки условий goto 50
            posofEndofIf++;
            while(list[posofEndofIf].getType()!="ENDofIF"){ //пока не дойдём до конца тела текущего if
                if(((list[posofEndofIf].getType()=="CONDITION"))|| //если хоть какую-то в нем вложенность находим
                   (list[posofEndofIf].getType()=="CYCLEFOR")|| // хоть вложенное условие, хоть вложенный цикл, то создаём новый объект
                   (list[posofEndofIf].getType()=="CYCLEWHILE")|| // не забываем про static переменную, она указывает новое место где мы окажемся
                   (list[posofEndofIf].getType()=="CYCLEDOWHILE"))//поднявшись обратно наверх от вложенного объекта
                {
                        ConditionExpression* cx = new ConditionExpression(posofEndofIf,list);
                        posofEndofIf=cx->getGlobalPos();
                        expressionList.push_back(cx);
                }
                else
                {
                    while(list[posofEndofIf].getType()!="SEMICOLON"){ //если вложенности нет или мы с ней уже закончили, то формируем обычные выражения
                        localList.push_back(list[posofEndofIf]);
                        posofEndofIf++;}
                    StatementExpression* rx= new StatementExpression(localList);
                    expressionList.push_back(rx);
                    localList.clear();
                    posofEndofIf++;
                }
            }
            posofEndofIf++;
            return;
            //после ENDofIF перескакивать не надо, чтобы было разделение на отдельные объекты у if и else
        }
        if(list[posofEndofIf].getType()=="UNCONDITION"){
            while(list[posofEndofIf].getType()!="BEGIN")
            { posofEndofIf++;} posofEndofIf++;
            //TPostfixCalc p(condition);// а здесь отрицание этого метода, довольно удобно получится
            // те же шаги, что и при condition, но уже в цикле до ENDofCycle
            while(list[posofEndofIf].getType()!="ENDofCycle"){
                if(((list[posofEndofIf].getType()=="CONDITION"))||
                   (list[posofEndofIf].getType()=="CYCLEFOR")||
                   (list[posofEndofIf].getType()=="CYCLEWHILE")||
                   (list[posofEndofIf].getType()=="CYCLEDOWHILE"))
                    {
                    ConditionExpression* cx =new ConditionExpression(posofEndofIf,list);
                    posofEndofIf=cx->getGlobalPos();
                    expressionList.push_back(cx);}
               else{
                   while(list[posofEndofIf].getType()!="SEMICOLON")
                   {
                       localList.push_back(list[posofEndofIf]);
                       posofEndofIf++;
                   }
                   StatementExpression* rx= new StatementExpression(localList);
                   expressionList.push_back(rx);
                   localList.clear();
                   posofEndofIf++;
               }
            }
            posofEndofIf++;
            return;
        }
        if((list[posofEndofIf].getType()=="CYCLEFOR")||(list[posofEndofIf].getType()=="CYCLEWHILE")){
            while(list[posofEndofIf].getType()!="DO"){
                condition.push_back(list[posofEndofIf]);
                posofEndofIf++;  }
            posofEndofIf++;
            posofEndofIf++;
            // те же шаги, что и при uncondition
            while(list[posofEndofIf].getType()!="ENDofCycle"){
                if((list[posofEndofIf].getType()=="CONDITION")||
                   (list[posofEndofIf].getType()=="CYCLEFOR")||
                   (list[posofEndofIf].getType()=="CYCLEWHILE")||
                   (list[posofEndofIf].getType()=="CYCLEDOWHILE")){
                    ConditionExpression* cx= new ConditionExpression(posofEndofIf,list);
                    posofEndofIf=cx->getGlobalPos();
                    expressionList.push_back(cx);}
               else{
                   while(list[posofEndofIf].getType()!="SEMICOLON"){
                       localList.push_back(list[posofEndofIf]);
                       posofEndofIf++;}
                   StatementExpression* rx=new StatementExpression(localList);
                   expressionList.push_back(rx);
                   localList.clear();
                   posofEndofIf++;
               }
            }
            posofEndofIf++;
            return;
            /*int i1,i2;
            while(list[posofEndofIf].getType()!="ASSIGN"){  posofEndofIf++;  }
            i1=std::stoi(list[posofEndofIf+1].getValue());
            i2=std::stoi(list[posofEndofIf+3].getValue());
            posofEndofIf=posofEndofIf+6;
            if( list[posofEndofIf+2].getType()=="INC"){//или можно сделать if(i1<i2){} else{}, не знаю что быстрее,
                //но наверно второе потому что обратиться к локальной переменной всяко быстрее
                //чем лезть по индексу в вектор и у него брать поле
                for (i1;i1<i2;i1++){ //не усралось расшифровка цикла сейчас, надо потом
                    //либо добавить i2-i1 одинаковых строчек, либо париться с циклом потом, а сейчас сделать как с if
                    int  c=posofEndofIf;
                    if((list[c].getType()=="CONDITION")||(list[c].getType()=="CYCLEFOR")||
                       (list[c].getType()=="CYCLEWHILE")||(list[c].getType()=="CYCLEDOWHILE")){
                        ConditionExpression cx(c,list);
                    expressionList.push_back(&cx);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                            localList.push_back(list[posofEndofIf]);
                            posofEndofIf++;}
                        StatementExpression ex(localList);
                        expressionList.push_back(&ex);
                        localList.clear();
                        posofEndofIf++;
                }
            }
            else{
                for (i1;i1>i2;i2--){
                    if((list[posofEndofIf].getType()=="CONDITION")||
                       (list[posofEndofIf].getType()=="CYCLEFOR")||
                       (list[posofEndofIf].getType()=="CYCLEWHILE")||
                       (list[posofEndofIf].getType()=="CYCLEDOWHILE")){
                        ConditionExpression ex(posofEndofIf,list);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                        localList.push_back(list[posofEndofIf]);
                        posofEndofIf++;}
                    StatementExpression ex(localList);
                    expressionList.push_back(&ex);
                    localList.clear();
                    posofEndofIf++;
                }
            }*/
       }
        if(list[posofEndofIf].getType()=="CYCLEDOWHILE"){
            posofEndofIf++;
            posofEndofIf++;
            // те же шаги, что и при uncondition
            while(list[posofEndofIf].getType()!="ENDofCycle"){
                if((list[posofEndofIf].getType()=="CONDITION")||
                   (list[posofEndofIf].getType()=="CYCLEFOR")||
                   (list[posofEndofIf].getType()=="CYCLEWHILE")||
                   (list[posofEndofIf].getType()=="CYCLEDOWHILE")){
                    ConditionExpression* cx= new ConditionExpression(posofEndofIf,list);
                    posofEndofIf=cx->getGlobalPos();
                    expressionList.push_back(cx);}
               else{
                   while(list[posofEndofIf].getType()!="SEMICOLON"){
                       localList.push_back(list[posofEndofIf]);
                       posofEndofIf++;}
                   StatementExpression* rx=new StatementExpression(localList);
                   expressionList.push_back(rx);
                   localList.clear();
                   posofEndofIf++;
               }
            }
            posofEndofIf++;
            while(list[posofEndofIf].getType()!="SEMICOLON")
            {
                condition.push_back(list[posofEndofIf]);
                posofEndofIf++;
            }
            posofEndofIf++;
            return;
        }
    }
    ConditionExpression& operator=(const ConditionExpression& other) {
        this->condition = other.condition;
        this->expressionList = other.expressionList;
        return *this;
    }
    void print() override{
       std::cout<<"ConditionExpression "<<++y<<" = ";
       if(!condition.empty()){
           for(auto token:condition)
           { std::cout<<token.getValue()<<" "; } std::cout<<endl;}
       else{std::cout<<"this is else"<<endl;}
       if(!expressionList.empty())
       {
           for(auto token2:expressionList)
           {
               std::cout<<"   ";token2->print(); cout<<std::endl;
           }
       }
    }
    void makeCondition(){}
    void toSolve(){}
};


#endif //CONDITIONEXPRESSION_H
