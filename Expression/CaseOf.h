//
// Created by shuri on 11.05.2024.
//

#ifndef CASEOF_H
#define CASEOF_H


#include <vector>
#include "Expression.h"
#include "../Token.h"
static int globalPosCase=0;
class CaseOf: public Expression {
private:
    Token value;
    vector<std::pair<Expression*,char*>> body;
public:
    CaseOf(){value=Token();};
    CaseOf(int pos, vector<Token>list){
        doSwitch(pos,list);
    }
    void doSwitch(int pos, vector<Token>list){
        pos++;
        value=list[pos];
    }
    void print(int tab)override{}
    int getPos(){return globalPosCase;}
};


#endif //CASEOF_H
