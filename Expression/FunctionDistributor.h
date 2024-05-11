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
public:
    FunctionDistributor(vector<Token> vec){
        list=std::move(vec);
    }
    FunctionDistributor& operator=(const FunctionDistributor& other) {
        this->list=other.list;
        return *this;
    }
    vector<Token>getList(){return list;}
    void print(int tab) override{

    }
};


#endif //FUNCTIONALLOCATOR_H
