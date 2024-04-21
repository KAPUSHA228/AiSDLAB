//
// Created by egorm on 21.04.2024.
//

#ifndef EXCEPTIONAST_H
#define EXCEPTIONAST_H
#include <string>
#include <utility>

class MyException {
    std::string exception_string;

public:
    explicit MyException(std::string string) {
        this->exception_string = std::move(string);
    }
};
#endif //EXCEPTIONAST_H
