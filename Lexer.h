//
// Created by egorm on 19.04.2024.
//

#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <regex>
#include<vector>
#include<string>

#include "Token.h"
#include "TokenType.h"


class Lexer {
private:
    std::vector<Token> tokenList;
    std::string input_string;
    std::string copy_input_string;
    std::vector<std::pair<std::string, std::string> > vector;
    int pos = 0;

public:
    Lexer(const std::string &input_string) {
        this->input_string = input_string;
        copy_input_string = input_string;
        vector = getLitTokenType();
        while (hasNext()) {
        }
    }

    bool hasNext() {
        for (const auto &item: vector) {
            const std::string s = input_string;
            std::regex rgx("^" + item.second);
            std::smatch match;
            // std::cout<<"^" + item.second<<std::endl;
            if (std::regex_search(s.begin() + pos, s.end(), match, rgx)) {
                std::string res = static_cast<std::string>(match[0]);
                tokenList.emplace_back(item.first, res, pos);
                std::cout << item.first << " " << res.length() <<" "<<res<< std::endl;

                pos+=res.length();
                return true;
            }
        }
        return false;
    }

    std::vector<std::pair<std::string, std::string> > getLitTokenType() {
        return {
            {"CONST", "Const"},
            {"CONST", "Const"},
            {"VAR", "var"},
            {"TYPEINTEGER", "integer"},
            {"TYPEREAL", "real"},
            {"TYPESTRING", "string"},
            {"TYPECHAR", "char"},
            {"TYPEBOOLEAN", "boolean"},
            {"JG", ">"},
            {"JGE", ">="},
            {"JL", "<"},
            {"JLE", "<="},
            {"JE", "="},
            {"JNE", "<>"},
            {"AND", "and"},
            {"OR", "or"},
            {"NOT", "not"},
            {"XOR", "xor"},
            {"COLON", ":"},
            {"COMMA", ","},
            {"APOSTROF", "'"},
            {"SEMICOLON", ";"},
            {"TITLE", "program qq"},
            {"OPENPARENTHESES", "[(]"},
            {"CLOSEPARENTHESES", "[)]"},
            {"OPENSQUARE", "[\[]"},
            {"CLOSESQUARE", "[\]]"},
            {"CONDITION", "if"},
            {"THEN", "then"},
            {"UNCONDITION", "else"},
            {"MOD", "mod"},
            {"DIV", "div"},
            {"PLUS", "[+]"},
            {"ASSIGN", ":="},
            {"WRITE", "write"},
            {"READ", "read"},
            {"CICLEFOR", "for"},
            {"CICLEWHILE", "while"},
            {"DO", "do"},
            {"BEGIN", "begin"},
            {"END", "end"},
            {"VARIABLE", "[a-zA-z]+"},
            {"SPACE", "[ \n\t]"}
        };
    }
};


#endif //LEXER_H
