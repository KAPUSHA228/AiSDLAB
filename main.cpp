#include <string>
#include "Lexer.h"
#include "Parser.h"
using namespace std;
int main() {
    string test= "var"
                 "begin"
                    "num1 := 2 ;"
                    "Write('Reader ', res2);"
                    "if 2=num1 then begin"
                        "if num1=2 then begin"
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
                "PI: real = 3.1415926;"
            "var"
                "num1, num2, i: integer;"
                "Res, d: real;"
                "res2: string;"
            "begin"
                "if 5 mod 3 > 0 then begin"
                    "Writeln ('Yes,if');"
                "end"
                "else begin"
                    "Writeln ('No,else');"
                "end;"
                "if 5 mod 3 = 0 then begin"
                    "Writeln ('Yes,if ');"
                "end"
                "else begin"
                    "Writeln ('No,else ');"
                "end;"
                "res2 := 'Hello world' ;"
                "num1 := 2 ;"
                "Writeln ('Reader ' , num1);"
                "if PI <> num1 then begin"
                    "Writeln( 'Pim' );"
                    "if PI <> num1 then begin"
                        "Writeln ('Pam');"
                   "end"
                "end"
                "else begin"
                    "Writeln ('Pum');"
                "end;"
                "num1 := 4 div 2;"
                "for i := 1 to 8 do begin"
                    "Write ( '3' ) ;"
                "end;"
                "while num1 < 4 do begin"
                    "Write('Yes');"
                    "num1 := num1 + 1;"
                "end;"
                /*"repeat begin"
                    "Write('3');"
                    "num1 := num1 + 1;"
                "end;"
                "until num1 < 4;"*/
            "end.";
    Lexer lexer(test_text);
    Parser parser(lexer);
    try{ parser.parse(); }
    catch(AgeException& e){ e.getMessage();}
    /*TPostfixCalc c;
    string s1 ="b : integer ";
    string s2="a : integer ";
    string s3="a := 5 ";
    string s4="b := 2 ";
    string s5="a := a mod b";
    Lexer lexer1 (s1);
    Lexer lexer2(s2);
    Lexer lexer3(s3);
    Lexer lexer4(s4);
    Lexer lexer5(s5);
    StatementExpression sx1(lexer1.getTokenList());
    StatementExpression sx2(lexer2.getTokenList());
    StatementExpression sx3(lexer3.getTokenList());
    StatementExpression sx4(lexer4.getTokenList());
    StatementExpression sx5(lexer5.getTokenList());
    string s6="if ( 2 <> 8 ) xor ( 4 < 2 ) then begin"
                "Write('Yes');"
              "end";
    Lexer lexer6(s6);
    ConditionExpression cx(0,lexer6.getTokenList());*/
    //string s="(8 + 3) * (9 + 4)";
    //Lexer lexer(s);
    //vector<Token>list=lexer.getTokenList();
    /*c.ChangeEquation(sx1);
    c.ChangeEquation(sx2);
    c.ChangeEquation(sx3);
    c.ChangeEquation(sx4);
    c.ChangeEquation(sx5);*/
    //c.ChangeEquation(cx);
    //c.getTable().root->print();
    //cout<<c.getTable().findNode("a",c.getTable().root)->data.value;
    //cout<<c;
    return 0;
}