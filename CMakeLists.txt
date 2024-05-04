cmake_minimum_required(VERSION 3.28)
project(project1)

set(CMAKE_CXX_STANDARD 20)

add_executable(project1 main.cpp
        Lexer.h
        Token.h
        Expression/Expression.h
        Expression/StatementExpression.h
        Parser.h
        HierarchyList.h
        Expression/ConditionExpression.h
        Postfix.h
        Stack.h
        SearchTreeTable.h
)
