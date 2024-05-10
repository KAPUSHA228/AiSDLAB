#include <string>
#include "Lexer.h"
#include "Parser.h"
using namespace std;
int main() {
    string test= "var"
                 "begin"
                    "num1 := 2 ;"
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
                /*"if 5 mod 3 > 0 then begin"
                    "Write ('Yes');"
                "end"
                "else begin"
                    "Write ('No');"
                "end;"*/
                //"res2 := 'Hello world' ;"
                 //"num1 := 2 ;"
                //"Write('Reader ' , num1);"
                /*"if Pi=num1 then begin"
                    "if Pi=num1 then begin"
                        "Write('Yes');"
                    "end"
                "end"
                "else begin"
                    "Write('No');"
                "end;"*/
                "num1 := 4 div 2;"
                "for i := 1 to 8 do begin"
                    "Writeln ( '3' ) ;"
                "end;"

               /* "while num1 < 4 do begin"
                    "Write('Yes');"
                    "num1 := num1 + 1;"
                "end;"*/
                /*"repeat begin"
                    "Write(i);"
                "end;"
                "until a<b;"*/
            "end.";
    Lexer lexer(test_text);
    Parser parser(lexer);
    try{ parser.parse(); }
    catch(AgeException& e){ e.getMessage();}
    /*TPostfixCalc c;
    string s5="a := 5";
    string s6="b := 2";
    string s3="c := a mod b";
    string s4="Read(a)";
    Lexer lexer3(s5);
    Lexer lexer4(s6);
    Lexer lexer5(s3);
    StatementExpression sx(lexer3.getTokenList());
    StatementExpression sx2(lexer4.getTokenList());
    StatementExpression sx3(lexer5.getTokenList());
    string s2="if ( 2 <> 8 ) xor ( 4 < 2 ) then begin"
              "Write('Yes');"
              "end";
    Lexer lexer2(s2);
    ConditionExpression cx(0,lexer2.getTokenList());
    //string s="(8 + 3) * (9 + 4)";
    //Lexer lexer(s);
    //vector<Token>list=lexer.getTokenList();
    c.ChangeEquation(cx);
    //c.ChangeEquation(sx);
    //c.ChangeEquation(sx2);
    //c.ChangeEquation(sx3);
    //c.getTable().root->print();
    //cout<<c.getTable().findNode("a",c.getTable().root)->data.value;
    //cout<<c;*/
    return 0;
}