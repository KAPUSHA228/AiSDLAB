#include <iostream>
#include <regex>


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
    cout<<text_program_pascal;
    cout<<regex_match("count", regex("[a-zA-Z]+"));
    return 0;
}
