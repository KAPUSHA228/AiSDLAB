//
// Created by shuri on 11.05.2024.
//

#ifndef FUNCTIONALLOCATOR_H
#define FUNCTIONALLOCATOR_H
#include <utility>
#include <vector>
#include "../Token.h"
#include "Expression.h"
#include "Function.h"
#include "Procedure.h"
class FunctionExecutor: public Expression{
private:
    vector<Token> list;
    string ans;
public:
    FunctionExecutor(vector<Token> argue, vector<Token> head, vector<Expression*> exp){
        list=argue;
        ParseFunction(argue, head, exp);
    }
    FunctionExecutor& operator=(const FunctionExecutor& other) {
        this->list=other.list;
        return *this;
    }
    void ParseFunction(vector<Token> argue, vector<Token> head, vector<Expression*> exp){

    }
    vector<Token>getList(){return list;}
    void print(int tab) override{

    }
};


#endif //FUNCTIONALLOCATOR_H
