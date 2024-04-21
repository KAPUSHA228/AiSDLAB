//
// Created by egorm on 20.04.2024.
//

#ifndef RUNNABLEEXPRESSION_H
#define RUNNABLEEXPRESSION_H
#include <vector>

#include "Expression.h"
//
class RunnableExpression : public Expression {
private:
    std::vector<Expression> expressionList;

public:
    void run() override {
        for (auto Expression: expressionList) {
            Expression.run();
        }
    }
};
#endif //RUNNABLEEXPRESSION_H
