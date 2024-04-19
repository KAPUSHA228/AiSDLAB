//
// Created by egorm on 19.04.2024.
//

#ifndef TOKENTYPE_H
#define TOKENTYPE_H
enum class TokenType {
    CONST="Const",
    VAR = "var",
    VARIABLE = "[a-zA-z]*",
    VALUEINTEGER ="\d", //вроде с ними нельзя делить
    VALUEREAL="[\d]\.[\d]",
    VALUESTRING="[a-zA-z]*",
    VALUECHAR="[a-zA-z]",
    VALUEBOOLEAN="[01]", //но там True/False
    TYPEINTEGER ="integer",
    TYPEREAL="real",
    TYPESTRING="string",
    TYPECHAR="char",
    TYPEBOOLEAN="boolean",
    JG=">",
    JGE=">=",
    JL="<",
    JLE="<=",
    JE="=",
    JNE="<>",
    AND="and",
    OR="or",
    NOT ="not",
    XOR="xor",
    COLON=": ",
    COMMA=",",
    APOSTROF = "'",
    ENDOFLINE =";\s",
    TITLE ="program qq",
    OPENPARENTHESES="(",
    CLOSEPARENTHESES=")",
    OPENSQUARE="[",
    CLOSESQUARE="]",
    CONDITION ="if",
    THEN="then",
    UNCONDITION ="else",
    MOD ="mod",// %
    DIV = "div",// /
    PLUS ="+",
    MINUS="-",
    INIT =":=",
    WRITE="write", //Pascal не чувствителен к регистру
    READ="read", // тоже
    CICLEFOR="for",
    CICLEWHILE="while",
    DO="do",
    BEGIN="begin",
    END ="end",
};
#endif //TOKENTYPE_H
