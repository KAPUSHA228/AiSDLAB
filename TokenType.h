//
// Created by egorm on 19.04.2024.
//

#ifndef TOKENTYPE_H
#define TOKENTYPE_H

class TokenType {
public:
    static std::vector<std::pair<std::string, std::string> > tokenTypeList;

    static void initTokenTypeList() {
        tokenTypeList = {
            {"CONST", "Const"},
            {"CONST", "Const"},
            {"VAR", "var"},
            {"VARIABLE", "[a-zA-z]*"},
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
            {"OPENPARENTHESES", "("},
            {"CLOSEPARENTHESES", ")"},
            {"OPENSQUARE", "["},
            {"CLOSESQUARE", "]"},
            {"CONDITION", "if"},
            {"THEN", "then"},
            {"UNCONDITION", "else"},
            {"MOD", "mod"},
            {"DIV", "div"},
            {"PLUS", "+"},
            {"ASSIGN", ":="},
            {"WRITE", "write"},
            {"READ", "read"},
            {"CICLEFOR", "for"},
            {"CICLEWHILE", "while"},
            {"DO", "do"},
            {"BEGIN", "begin"},
            {"END", "end"},
            {"SPACE", "[ \n\t]"}
        };
    }
};
#endif //TOKENTYPE_H
