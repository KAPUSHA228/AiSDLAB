#include <iostream>
#include <regex>
#include<string>

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
    const std::string s = "const tata";
    std::regex rgx("^const");
    std::smatch match;
    if (std::regex_search(s.begin(), s.end(), match, rgx))
        std::cout << "match: " << match[0] << '\n';
    return 0;
}
