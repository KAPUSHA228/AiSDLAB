#include <iostream>
#include <regex>
#include <string>
#include "Lexer.h"
#include "Parser.h"
using namespace std;
int main() {
    string test= "var"
                 "begin"
                    "num1:=2;"
                    "Write('Reader ', res2);"
                    "if Pi=num1 then begin"
                        "if Pi=num1 then begin"
                            "Write('Yes');"
                        "end"
                    "end"
                    "else begin"
                        "Write('No');"
                    "end;"
                 "end.";
    string test_text=
            "program qq;"
            "const"
                "Pi: real = 3.1415926;"
            "var"
                "num1, num2, i: integer;"
                "Res, d: real;"
                "res2: string;"
            "begin"
                "res2 := 'Hello world';"
                "num1:=2;"
                "Write('Reader ', res2);"
                "if Pi=num1 then begin"
                    "if Pi=num1 then begin"
                        "Write('Yes');"
                    "end"
                "end"
                "else begin"
                    "Write('No');"
                "end;"
                "num1:= num1 div 2;"
                "for i:=1 downto 8 do begin"
                    "Write(i);"
                "end;"
                "while a>b do begin"
                    "Write(i);"
                "end;"
                "repeat begin"
                    "Write(i);"
                "end;"
                "until a<b;"
            "end.";
    /*Lexer lexer(test_text);
    Parser parser(lexer);
    try{ parser.parse(); }
    catch(AgeException& e){ e.getMessage();}*/
    string s2="if ( 2 <> 8 ) and ( 4 > 2 ) then begin"
                "Write('Yes');"
              "end";
    Lexer lexer2(s2);
    string s="(8 + 3) * (9 + 4)";
    ConditionExpression cx(0,lexer2.getTokenList());
    Lexer lexer(s);
    vector<Token>list=lexer.getTokenList();
    TPostfixCalc c;
    c.ChangeEquation(cx);
    c.ToPostfixCondition(cx.getCondition());
    c.CalcCondition("if");
    //c.Build();
    //c.ToPostfix();
    //c.CalcPostfix();
    cout<<c;
    return 0;
}