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
class FunctionDistributor: public Expression{
private:
    vector<Token> list;
    string ans;
public:
    FunctionDistributor(vector<Token> vec){
        list=std::move(vec);
        ParseFunction(vec);
    }
    FunctionDistributor& operator=(const FunctionDistributor& other) {
        this->list=other.list;
        return *this;
    }
    void ParseFunction(vector<Token> vec){
        if(vec[2].getValue()=="AddNumbers"){

        }
        if (vec[2].getValue()=="GreetUser"){

        }
    }
    vector<Token>getList(){return list;}
    void print(int tab) override{

    }
};


#endif //FUNCTIONALLOCATOR_H
