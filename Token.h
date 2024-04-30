//
// Created by egorm on 19.04.2024.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <utility>
using namespace std;

class Token {
private:
    std::string type;
    std::string value;
    int pos;

public:
    Token(){
        this->type=std::string();
        this->value=std::string();
        this->pos=int();

    }
    Token(std::string type, std::string value, int pos) {
        this->type = std::move(type);
        this->value = std::move(value);
        this->pos = pos;
    }

    void setType(std::string type) {
        this->type = type;
    }
    std::string getType() {
        return this->type;
    }
    std::string getValue() {
        return this->value;
    }
    int getPos() {
        return this->pos;
    }
    friend std::ostream& operator<<(std::ostream& ostr, const Token& v)
    {
       return ostr;
    }
};


#endif //TOKEN_H
