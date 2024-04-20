//
// Created by egorm on 19.04.2024.
//

#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include<vector>
#include<string>

#include "Token.h"
#include "TokenType.h"


class Lexer {
private:
    std::vector<Token> tokenList;
    std::string input_string;
    std::string copy_input_string;

public:
    Lexer(std::string input_string) {
        this->input_string = input_string;
        copy_input_string = input_string;


    }
    bool hasNext() {
        return 1;
    }
};


#endif //LEXER_H
