//
// Created by egorm on 20.04.2024.
//

#ifndef RUNNABLEEXPRESSION_H
#define RUNNABLEEXPRESSION_H
#include <utility>
#include <vector>
#include "Expression.h"
#include "../Parser.h"
#include<bits/stdc++.h>
#include "stdexcept"
#include "../Lexer.h"
#include "../Token.h"
static int i=0;
class StatementExpression : public Expression {
private:
    std::vector<Token> list;
public:
    StatementExpression(){cout << list.size();}
    StatementExpression(std::vector<Token> _list)
    {list=std::move(_list);}
    StatementExpression(const StatementExpression& ex){
        this->list=ex.list;
    }
    StatementExpression& operator=(const StatementExpression& other) {
        this->list=other.list;
        return *this;
    }
    vector<Token>getList(){return list;}
    void add(Token t){list.push_back(t);}
    void print() override{
        std::cout<<"StateExpression "<<++i<<" = ";
        for(auto token:list){
          std::cout<<token.getValue()<<" ";
        }
        std::cout<<endl;
    }
};
#endif //RUNNABLEEXPRESSION_H
