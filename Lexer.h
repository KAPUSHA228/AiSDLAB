//
// Created by egorm on 19.04.2024.
//

#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <regex>
#include<vector>
#include<string>
#include <set>
#include "Token.h"
#include "TokenType.h"
#include <locale.h>

class Lexer {
private:
    std::set<Token>tokenSet;
    std::vector<Token> tokenList;
    std::string input_string;
    std::string copy_input_string;
    std::vector<std::pair<std::string, std::string>> vector;
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
       // std::setlocale(LC_ALL,"Russian)");
        for (const auto &item: vector) {
            const std::string s = input_string;
            std::regex rgx("^" + item.second);
            std::smatch match;
            // std::cout<<"^" + item.second<<std::endl;
            //std::cout <<"In line 1 were found the following elements: ";
            if (std::regex_search(s.begin() + pos, s.end(), match, rgx)) {
                //std::cout <<"In line 1 were found the following elements: ";
                std::string res = static_cast<std::string>(match[0]);
                tokenList.emplace_back(item.first, res, pos);
                std::cout<<"Type of lexema: " <<item.first <<
                "; The space spent on the lexema: " << res.length() <<
                "; Value of lexema: "<<res<<" ;"<<std::endl;
                pos+=res.length();
                return true;
            }
        }
        return false;
    }
    /*friend std::ostream& operator<<(std::ostream& ostr, const Lexer& v)
    {
        ostr << ;
        return ostr;
    }*/
    std::vector<std::pair<std::string, std::string> > getLitTokenType() {
        return {
            {"CONST","Const"},
            {"VAR","var"},
            {"BEGIN","begin"},
            {"END","end"},
            {"TYPEINTEGER", "integer"},
            {"TYPEREAL",    "real"},
            {"TYPESTRING",  "string"},
            {"TYPECHAR",    "char"},
            {"TYPEBOOLEAN", "boolean"},
            {"VALUEREAL","[0-9]+\.[0-9]+"},
            {"VALUEINTEGER","[0-9]+"},
            {"VALUECHAR","'[a-z0-9A-z]'"},
            {"VALUESTRING","'[a-z0-9A-z]+'"},
            {"VALUEBOOLEANTrue","True"},
            {"VALUEBOOLEANFalse","False"},
            {"ASSIGN",      ":="},
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
           // {"APOSTROF", "'"},
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
            {"WRITE", "write"},
            {"READ", "read"},
            {"CICLEFOR", "for"},
            {"CICLEWHILE", "while"},
            {"DO", "do"},
            {"VARIABLE", "[a-zA-z]+"},
            {"SPACE", "[ ]"},
            {"TAB", "[\t]"},
            {"ENTER", "[\n]"},
        };
    }
};


#endif //LEXER_H
