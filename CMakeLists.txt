cmake_minimum_required(VERSION 3.28)
project(project1)

set(CMAKE_CXX_STANDARD 20)

add_executable(project1 main.cpp
        Lexer.cpp
        Lexer.h
        Token.cpp
        Token.h
        TokenType.h
        Expression/Expression.h
        Expression/RunnableExpression.h
        Parser.h
        Expression/ExceptionAST.h
        HierarchyList.h)
