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

class ConditionExpression: public Expression{
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> condition;
public:
    ConditionExpression(std::vector<Token>_condition, std::vector<vector<Token>>_list){
        if(_condition.front().getValue()=="if"){ }
        if(_condition.front().getValue()=="for"){
            int i1,i2; int i=0;
            while(_condition[i].getType()!="ASSIGN"){
                i++;
            }
            i1=stoi(_condition[i+1].getValue());
            i2=stoi(_condition[i+3].getValue());
           if( _condition[i+2].getValue()=="to"){//или можно сделать if(i1<i2){} else{}, не знаю что быстрее
               //но наверно второе потому что обратиться к локальной переменной всяко быстрее
               //чем лезть по индексу в вектор и у него брать поле
               for (i1;i1<i2;i1++){

               }
           }
           else{
               for (i2;i2<i1;i2++){

               }
           }

        }
        if(_condition.front().getValue()=="while"){ }


        TPostfixCalc p(_condition);
        for(auto innerVector:_list){RunnableExpression re(innerVector);}
    }
    void run() override {
       // for (auto Expression: expressionList) {
         //   Expression.run();
        //}
    }
};


#endif //CONDITIONEXPRESSION_H
