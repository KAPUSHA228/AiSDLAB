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
class FunctionAllocator: public Expression{
private:
vector<Token> list;
public:
    FunctionAllocator(vector<Token> vec){
        list=std::move(vec);
    }
    FunctionAllocator& operator=(const FunctionAllocator& other) {
        this->list=other.list;
        return *this;
    }
    vector<Token>getList(){return list;}
    void print() override{

    }
};


#endif //FUNCTIONALLOCATOR_H
