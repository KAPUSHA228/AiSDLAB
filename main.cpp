#include <iostream>
#include <regex>
#include<string>
#include "Lexer.h"
#include "Parser.h"
using namespace std;
void forTestRegex(const string &str);
class A{
public:
    virtual void print() =0;
};
class B:public A{
    vector<Token>v;

public:
    B(){cout<<"size of b = "<<v.size()<<endl;}
    B(std::vector<Token> _list)
    {v=_list;}
    void add(const Token& t){v.push_back(t);}
    void print() override{
            for(auto token:v){std::cout<<token.getValue()<<" ";}
            cout<<"size ex of b = "<<v.size()<<endl;
       // std::cout<<endl;
    }
    B& operator=(const B& other) {
        this->v = other.v;
        return *this;
    }
    void clear(){
        this->v.clear();
    }
    void getValue(){
        for(auto item:v){
            item.getValue();
        }
    }
};
class C{
private:
    std::vector<Token>token;
    std::vector<A*> vec;
public:
    C(Lexer lexer){this->token=lexer.getTokenList();}
    C(A* v){vec.push_back(v); }
    C(std::vector<Token> t){this->token=t;}
    void add(A* v){ A* vc=v; vec.push_back(vc);}
    void add(Token t){token.push_back(t);}
    void create(vector<Token>t){B b(t); vec.push_back(&b); }
    C& operator=(const C& other) {
        this->token = other.token;
        this->vec = other.vec;
        return *this;
    }
    void print(){
        cout<<"size of c = "<<vec.size()<<endl;
        auto iter{vec.begin()};
        while(iter!=vec.end()){
            cout<<"Size of el of c = ";
            cout<<endl;
            (*iter)->print();
            ++iter;}


    }
    void parse(vector<Token>token){
        B* b1= new B(token);
        vec.push_back(b1);
        vec.push_back(b1);
        cout<<"size of c ex = "<<vec.size();
         auto iter{vec.begin()};
         while(iter!=vec.end()){cout<<"k";  (*iter)->print();++iter;}
        //здесь элемент b
         // у сысоева был пример с вектором целых чисел и *iter показывал
         //целое число, а у нас тут вектор указателей на класс А, соответсвенно
         // я думал что *iter=A*, но почему-то это равно просто B
    }

};
int main() {
   /* vector <Token>v={
            {"VAR","var",2},
            {"VARIABLE","b",2},
            {"SEMICOLON",":integer;",2},
            {"BEGIN","begin",2}
    };
    vector <Token>v1={
            {"VAR","var",2},
            {"VARIABLE","b",2},
            {"SEMICOLON",":integer;",2},
            {"BEGIN","begin",2},
            {"BEGIN","begin",2}
    };
    vector <Token>v2={
            {"VAR","var",2},
            {"VARIABLE","b",2},
            {"SEMICOLON",":integer;",2},
            {"BEGIN","begin",2},
            {"BEGIN","begin",2},
            {"BEGIN","begin",2}

    };
    Token t("VAR","var",2);
    Token t1("VARIABLE","c",2);
    //B b;
   //b.add(t);
   //b.add(t1);
    B b(v);
    B b1(v1);
    B b2(v2);
    C c(&b);
   // C c(v);
   // c.print();
   // c.add(&b1);
   // c.add(&b2);

   // c.create(v); //он не хоооооооооооооооочееееееееееет аааааааааааааааааааааааааааааааааааааааааааааааааааааааааааааа
   // c.create(v1);
   // c.create(v2);

   // c.print();

   // c.parse(v);
    //c.print();*/
    string text_program_pascal =
            "var a:real;\n"
            "    b:char;\n"
            "    c: string;\n"
            "begin\n"
            "   a := 1.2;\n"
            "   b := '19w';\n"
            "\tc := a + b;\n"
            "end.\n";
    string base= "program qq;"
                 "var"
                    "a: integer;"
                    "b: real;"
                 "begin"
                    "a:=3;"
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
                    "Write('Yes');"
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
    string test= "var" "begin"
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
    Lexer lexer(test_text);

    //HierarchyList <string,Expression*>l;
   /* Parser parser(lexer);
    try{ parser.parse(); }
    catch(AgeException& e){ e.getMessage();}
    parser.print();*/
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
