#include <iostream>
#include <regex>
#include<string>
#include "Expression/Expression.h"
#include "Lexer.h"
#include "Parser.h"

using namespace std;

void forTestRegex(const string &str);

int main() {
    /* PascalABC for example
     *
     *var a,b :integer;
     *begin
     *a := 12;
     *b := 13;
     *end.
     */

    //В конце программы - end.   !!!
    string text_program_pascal =
            "var a:real;\n"
            "var b:char;\n"
            "var c: string;\n"
            "begin\n"
            "   a := 1.2;\n"
            "   b := '19w';\n"
            "\tc := a + b;\n"
            "end.\n";
    // cout<<text_program_pascal;
    forTestRegex(" +dshgdhgads)");
string test_text=" program qq;"
                 "const"
                 "Pi: real = 3.1415926;"
                 "var"
                 "num1, num2: integer;"
                 "Res, d: real;"
                 "res2: string;"
                 "begin"
                 "res2 := 'Hello world';"
                 "num1:=2;"
                 "Write('Reader ', res2);"
                 "num1:= num1 div 2;"
                 "Write(num1);"
                 "end.";
     Lexer lexer(test_text);
    Parser parser(lexer);
    //try{ parser.parse(); }
    //catch(AgeException& e){ e.getMessage();}


    return 0;
}

void forTestRegex(const string &str) {
    const std::string &s = str;
    std::regex rgx("^[ \n\t]");
    std::smatch match;
    if (std::regex_search(s.begin(), s.end(), match, rgx)) {
        string str2 = static_cast<string>(match[0]);
        // cout << str2.length()<<endl;
    }
}
