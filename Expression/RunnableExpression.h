//
// Created by egorm on 20.04.2024.
//

#ifndef RUNNABLEEXPRESSION_H
#define RUNNABLEEXPRESSION_H
#include <vector>
#include "Expression.h"
#include "../Parser.h"
#include<bits/stdc++.h>
#include "stdexcept"
#include "../Lexer.h"
#include "../Token.h"
class RunnableExpression : public Expression {
private:
    std::vector<Expression*> expressionList;

public:
    RunnableExpression(std::vector<Token> list){
        Token t1=list.back();
        Token t2=list.front();


    }


    void run() override {
        for (auto Expression: expressionList) {
            Expression->run();
        }
    }
};
#endif //RUNNABLEEXPRESSION_H
