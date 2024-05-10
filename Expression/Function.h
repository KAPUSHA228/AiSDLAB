//
// Created by пк on 08.05.2024.
//

#ifndef FUNCTION_H
#define FUNCTION_H
#include "Expression.h"
#include "../Token.h"
#include <vector>
class Function: public Expression {
private:
    std::vector<Expression*> expressionList;
    std::vector<Token> declaration;
public:
    Function(int pos, vector<Token> list){
        doFunction(pos,list);
    }
    Function( const Function& ex){
        this->declaration=ex.declaration;
        this->expressionList=ex.expressionList;
    }
    void doFunction(int pos, vector<Token> list){
        while(list[pos].getValue()!="SEMICOLON"){
            declaration.push_back(list[pos]);
        }
        pos++;
        while(list[pos].getType()!="ENDofCycle"){

        }
    }
    void print() override{}
};


#endif //FUNCTION_H
