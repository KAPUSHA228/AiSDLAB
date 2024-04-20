#include <iostream>
#include <regex>
#include<string>

#include "Lexer.h"

using namespace std;

int main() {
    /* PascalABC for example
     *
     *var a,b :integer;
     *begin
     *a := 12;
     *b := 13;
     *end.
     */
    string text_program_pascal = " var a,b :integer;\n"
            "begin\n"
            "  a := 12;\n"
            "  b := 13;\n"
            " end;\n";
    // cout<<text_program_pascal;

    const std::string s = " +dshgdhgads)";
    std::regex rgx("^[ \n\t]");
    std::smatch match;
    if (std::regex_search(s.begin(), s.end(), match, rgx)) {
        string str2 = static_cast<string>(match[0]);
        // cout << str2.length()<<endl;
    }
    Lexer lexer(text_program_pascal);

    return 0;
}
