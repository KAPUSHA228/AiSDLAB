//
// Created by egorm on 19.04.2024.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <utility>


class Token {
private:
    std::string type;
    std::string value;
    int pos;

public:
    Token(std::string type, std::string value, int pos) {
        this->type = std::move(type);
        this->value = std::move(value);
        this->pos = pos;
    }
};


#endif //TOKEN_H
