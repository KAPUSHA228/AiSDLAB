//
// Created by shuri on 26.04.2024.
//

#ifndef POSTFIX_H
#define POSTFIX_H
#include <string>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include "Stack.h"
#include "SearchTreeTable.h"
#include "Expression/Expression.h"
#include "Expression/StatementExpression.h"
#include "Expression/ConditionExpression.h"
using namespace std;
static SearchTreeTable<string, string>table;
class TPostfixCalc // не доделан под нужды уравнений с переменными, объявлений и сравнений
{
private:
    string type;
    vector<Token> infix;
    vector<Token> postfix;
    TStack<string> operationStack;
    TStack<double> operandStack;
    double res;
protected:
    static int Priority(string s) {
        if((s=="(")||(s==")"))
            {return 1;}
        if((s=="+")||(s=="-"))
            {return 2;}
        if((s=="*")||(s=="div")||(s=="mod"))
            {return 3;}
        return -1;
    }
   static int PrioritySearch(const string& s) {
        std::unordered_map<std::string, int> priorityMap = {
                {"(", 1},
                {")", 1},
                {"+", 2},
                {"-", 2},
                {"*", 3},
                {"div", 3},
                {"mod", 3}
        };

        // Проверяем, есть ли оператор в hashmap и возвращаем соответствующий приоритет
        if (priorityMap.find(s) != priorityMap.end()) {
            return priorityMap[s];
        }
        return -1;
    }
    static int PriorityCond(string s){
        if((s=="(")||(s==")")){return 1;}
        if(s=="not"){return 4;}
        if(s=="and"){return 3;}
        if((s=="or")||(s=="xor")){return 2;}
        if((s==">")||(s=="<")||
        (s==">=")||(s=="<=")||
        (s=="<>")||(s=="=")){return 5;}
        return -1;
    }
public:
    TPostfixCalc() {
        postfix = vector<Token>();
        infix = vector<Token>();
        operationStack = TStack<string>();
        operandStack = TStack<double>();
        res = 0;

    }
    TPostfixCalc(const TPostfixCalc& c) {
        if (&c == this) throw std::runtime_error{"odinakovo nasrano"};
        postfix = c.postfix;
        infix = c.infix;
        operationStack = c.operationStack;
        operandStack = c.operandStack;
        res = c.res;
    }
    ~TPostfixCalc() = default;

    static void setData(string key,string type){ table.Insert(key,type);}
    static SearchTreeTable<string,string> getTable(){return table;}

    void ChangeEquation(string eq){
        Lexer lexer(eq);
        infix=lexer.getTokenList();
        postfix = vector<Token>();
        operationStack = TStack<string>(eq.length());
        operandStack = TStack<double>(eq.length());
        res=0;
    }
    void ChangeEquation(vector<Token>v) {
        infix = v;
        postfix = vector<Token>();
        operationStack = TStack<string>(v.size());
        operandStack = TStack<double>(v.size());
        res=0;
    }
    void ChangeEquation(StatementExpression s)// фул работает ура
    {
        infix = s.getList();
        type=infix[0].getValue();
        postfix = vector<Token>();
        operationStack = TStack<string>(s.getList().size());
        operandStack = TStack<double>(s.getList().size());
        res=0;
        if(infix[0].getValue()=="Write"){
           (infix[3].getType() == "COMMA") ?
           cout << infix[2].getValue() << " " << *(table.FindValue(infix[4].getValue()))
           : cout<<table.findNode(infix[4].getValue(),table.root)->data.value;
           return;
        }
        if(infix[0].getValue()=="Writeln"){
           (infix[3].getType() == "COMMA") ?
           cout << infix[2].getValue() << " " << *(table.FindValue(infix[4].getValue()))
           : cout<<table.findNode(infix[4].getValue(),table.root)->data.value;
           cout<<endl;
            return;
        }
        if(infix[0].getValue()=="Read"){
           string value;
           cin >> value;
           table.Change(infix[2].getValue(),value);
            return;
        }
        if(infix[0].getValue()=="Readln"){
           string value;
           cin >> value;
           table.Change(infix[2].getValue(),value);
           cout<<endl;
            return;
        }
        else{ //отсекли консоль, теперь объявления и выражения
            int i=0;
            while((infix[i].getValue()!=":")&&(i!=(infix.size()-1))){i++;} // токен ":" присутствует только в объявлениях
            if(i==(infix.size()-1)){Build();} //соответственно если дошли до конца то ":" не нашли и просто билдим
            else{ toDeclarate(infix);}
            return;
        }
    }
    void toDeclarate(vector<Token> s)
    {
        string str=s.back().getValue();
        int i=0;
        while(i<s.size()-2){
            if(s[i].getType()=="VARIABLE"){
                table.Insert(s[i].getValue(),str);
                i++;
            }
            else{i++;}

        }
    }
    void ChangeEquation(ConditionExpression s) {
        infix = s.getCondition();
        vector<Expression*> body=s.getBody().second;
        postfix = vector<Token>();
        operationStack = TStack<string>(s.getCondition().size());
        operandStack = TStack<double>(s.getCondition().size());
        res=0;
        ToPostfixCondition(s.getCondition());
        CalcCondition(type);
    }
    vector<Token> GetInf() { return infix; }
    vector<Token> GetPost() { return postfix; }
    double GetRes(){  return res; }
    void ToPostfix() //робит
    {
        type= infix[0].getValue();
        auto iter=infix.begin();
        infix.erase(iter);
        infix.erase(iter);
        string el;
        postfix = vector<Token>();
        vector<Token> s = vector<Token>();
        Token t={"OPENPARENTHESES","(",0};
        s.push_back(t);
        for(auto item:infix){
            s.push_back(item);
        }
        Token t1={"CLOSEPARENTHESES",")",0};
        s.push_back(t1);
        /*for(auto item:s){
        cout<<" "<<item.getValue();
        } cout<<endl;*/
        for (size_t i = 0; i < s.size(); i++)
        {
            /*if (s[i].getType()== "VARIABLE"){
                string str =*table.FindValue(s[i].getValue());
                string str2 =table.findNode(s[i].getValue(),table.root)->data.type;
                if(str2=="VALUEINTEGER"||str2=="VALUEREAL"){
                    postfix.push_back(s[i]);
                }
                else{
                    //ачё делать с char и string? стеки под них не переделаешь, у нас всегда double
                }
            }*/
            if ((s[i].getType()=="VALUEINTEGER")||(s[i].getType()=="VALUEREAL")||(s[i].getType()=="VARIABLE")){
                postfix.push_back(s[i]);}
            if (s[i].getType() == "DIV" || s[i].getType() == "MOD" || s[i].getType() == "PLUS" ||
                s[i].getType() == "MINUS"||s[i].getType() == "MULTI") {
                if(operationStack.IsEmpty()){
                    operationStack.Push(s[i].getValue());
                    continue;}
                el = operationStack.Pop();
                while (Priority(s[i].getValue()) <= Priority(el)) {
                    if(el[0]=='-'){ Token t={"MINUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='+'){ Token t={"PLUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='*'){ Token t={"MULTI",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='d'){ Token t={"DIV",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='m'){ Token t={"MOD",el,0};
                        postfix.push_back(t);}
                    el = operationStack.Pop();
                }
                operationStack.Push(el);
                operationStack.Push(s[i].getValue());
            }
            if (s[i].getValue() == "(")
                operationStack.Push(s[i].getValue());
            if (s[i].getValue() == ")") {
                el = operationStack.Pop();
                while (el != "(") {
                    if(el[0]=='-'){
                        Token t={"MINUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='+'){
                        Token t={"PLUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='*'){
                        Token t={"MULTI",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='d'){
                        Token t={"DIV",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='m'){
                        Token t={"MOD",el,0};
                        postfix.push_back(t);}
                    el = operationStack.Pop();
                }
            }
            else{continue;}
            }
    }
    void ToPostfixCondition(vector<Token>condition)//робит
    {
        type= condition[0].getValue();
        infix = condition;
        auto iter=infix.begin();
        infix.erase(iter);
        iter=infix.end();
        infix.erase(iter);
        string el;
        postfix = vector<Token>();
        vector<Token> s = vector<Token>();
        Token t={"OPENPARENTHESES","(",0};
        s.push_back(t);
        for(auto item:infix){
            s.push_back(item);
        }
        Token t1={"CLOSEPARENTHESES",")",0};
        s.push_back(t1);
        for (size_t i = 0; i < s.size(); i++)
        {
            if ((s[i].getType()=="VALUEINTEGER")||(s[i].getType()=="VALUEREAL")||(s[i].getType()=="VARIABLE")){
                postfix.push_back(s[i]);}
            if (s[i].getType() == "NOT" || s[i].getType() == "AND" ||
            s[i].getType() == "OR" ||s[i].getType() == "XOR"||
            s[i].getType() == "JG"||s[i].getType() == "JL" ||
            s[i].getType() == "JGE" ||s[i].getType() == "JLE"||
            s[i].getType() == "JNE"||s[i].getType() == "JE") {
                if(operationStack.IsEmpty()){operationStack.Push(s[i].getValue());
                    continue;}
                el = operationStack.Pop();
                while (PriorityCond(s[i].getValue()) <= Priority(el)) {
                    if(el[0]=='n'){ Token t={"NOT",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='a'){ Token t={"AND",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='x'){ Token t={"XOR",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='o'){ Token t={"OR",el,0};
                        postfix.push_back(t);}
                    if(el=="="){ Token t={"JE",el,0};
                        postfix.push_back(t);}
                    if(el=="<="){ Token t={"JLE",el,0};
                        postfix.push_back(t);}
                    if(el==">="){ Token t={"JGE",el,0};
                        postfix.push_back(t);}
                    if(el=="<>"){ Token t={"JNE",el,0};
                        postfix.push_back(t);}
                    if(el=="<"){ Token t={"JL",el,0};
                        postfix.push_back(t);}
                    if(el==">"){ Token t={"JG",el,0};
                        postfix.push_back(t);}
                    el = operationStack.Pop();

                }
                operationStack.Push(el);
                operationStack.Push(s[i].getValue());
            }
            if (s[i].getValue() == "(")            {
                operationStack.Push(s[i].getValue());}
            if (s[i].getValue() == ")") {
                el = operationStack.Pop();

                while (el != "(") {
                    if(el[0]=='n'){ Token t={"NOT",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='a'){ Token t={"AND",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='x'){ Token t={"XOR",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='o'){ Token t={"OR",el,0};
                        postfix.push_back(t);}
                    if(el=="="){ Token t={"JE",el,0};
                        postfix.push_back(t);}
                    if(el=="<="){ Token t={"JLE",el,0};
                        postfix.push_back(t);}
                    if(el==">="){ Token t={"JGE",el,0};
                        postfix.push_back(t);}
                    if(el=="<>"){ Token t={"JNE",el,0};
                        postfix.push_back(t);}
                    if(el=="<"){ Token t={"JL",el,0};
                        postfix.push_back(t);}
                    if(el==">"){ Token t={"JG",el,0};
                        postfix.push_back(t);}
                    el = operationStack.Pop();
                }
            }
            else{continue;}
        }
    }
    bool CalcCondition(string Ttype)//удивительно, но робит
    {
        int i=0;
        if(Ttype=="if"){
            for (size_t i = 0; i < postfix.size(); i++)
            {
                if( postfix[i].getValue() == "not") {
                    double d1;
                    d1=operandStack.Pop();
                    d1==1?operandStack.Push(0):operandStack.Push(1);
                }
                if(postfix[i].getValue() == "and" || postfix[i].getValue() == "or" || postfix[i].getValue() == "xor"){
                    double d1, d2;
                    d1 = operandStack.Pop();
                    d2 = operandStack.Pop();
                    if(postfix[i].getValue() == "and"){
                        ((d1==1)&&(d2==1))?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == "or"){
                        ((d1==1)||(d2==1))?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == "xor"){
                        ((d1==1)^(d2==1))?operandStack.Push(1):operandStack.Push(0);}
                }
                if (postfix[i].getValue() == "<" || postfix[i].getValue() == "<>" || postfix[i].getValue() == "<="||
                postfix[i].getValue() == ">"|| postfix[i].getValue() == ">=" || postfix[i].getValue() == "=") {
                    double d1, d2;
                    d1 = operandStack.Pop();
                    d2 = operandStack.Pop();
                    if(postfix[i].getValue() == "<>"){
                        d2!=d1?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == "<="){
                        d2<=d1?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == "<"){
                        d2<d1?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == ">="){
                        d2>=d1?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == ">"){
                        d2>d1?operandStack.Push(1):operandStack.Push(0);}
                    if(postfix[i].getValue() == "="){
                        d2==d1?operandStack.Push(1):operandStack.Push(0);}

                }
                if (postfix[i].getType()== "VALUEINTEGER"|| postfix[i].getType() == "VALUEREAL") {

                    double ans=std::stod(postfix[i].getValue());
                    operandStack.Push(ans);
                }
                if (postfix[i].getType()== "VARIABLE"){
                    double ans=std::stod(table.findNode(postfix[i].getValue(),table.root)->data.value);
                    operandStack.Push(ans);

                }
            }
            res = operandStack.TopView();
            if(res==1) return true;
            else return false;
        }
       //if(Ttype=="else"){}
       if(Ttype=="for"){}
       if(Ttype=="while"){ while (CalcCondition("if")){

       }}
       if(Ttype=="until"){//сначала выполнить одну итерацию цикл, а потом уже
           while (CalcCondition("if")){

           }
           //или сразу сделать dowhile????????????
       }
       return true;
    }


    /*
   TPostfixCalc(string eq) {
        if (eq.length() <= 0) throw std::runtime_error{"dlina nasrano"};
        Lexer lexer(eq);
        vector<Token>list=lexer.getTokenList();
        postfix = vector<Token>();
        infix = list;
        operationStack = TStack<string>(eq.length());
        operandStack = TStack<double>(eq.length());
        res = 0;
    }
    TPostfixCalc(std::vector<Token> v){
       if (v.empty()) throw std::runtime_error{"nasrano"};
       infix=v;
       postfix = vector<Token>();
       operationStack=TStack<string>(v.size());
       operandStack = TStack<double>(v.size());
       res = 0;
    }

    TPostfixCalc(StatementExpression v){
        infix=v.getList();
        postfix = vector<Token>();
        operationStack=TStack<string>(v.getList().size());
        operandStack = TStack<double>(v.getList().size());
        res = 0;
    }
    TPostfixCalc(ConditionExpression v){
        auto body = v.getBody();
        auto con=body.first;
        auto exp=body.second;
        if(con[0].getValue()!="for") ToPostfixCondition(con);
    }*/
    /*int i=0;
        if(list.front().getValue()=="Write"){//3 варианта: текст, переменную, текст с переменной
            list[i+3].getType()=="СOMMA"?
            cout<<list[i+2].getValue()<<list[i+4].getValue():
            cout<<list[i+2].getValue();
        }
        if(list.front().getValue()=="Writeln"){
            list[i+3].getType()=="СOMMA"?
            cout<<list[i+2].getValue()<<list[i+4].getValue():
            cout<<list[i+2].getValue();
            cout<<endl;
        }
        if(list.front().getValue()=="Read"){
            double k;
            cin>>k;
            //обращение к таблице, добавить в неё значение с клавиатуры
        }
        if(list.front().getValue()=="Readln"){
            double k;
            cin>>k;
            //обращение к таблице, добавить в неё значение с клавиатуры
            cout<<endl;
        }
       // else{TPostfixCalc p(list);}//происходит выражение с присвоением*/
    /*int i1,i2;
            while(list[posofEndofIf].getType()!="ASSIGN"){  posofEndofIf++;  }
            i1=std::stoi(list[posofEndofIf+1].getValue());
            i2=std::stoi(list[posofEndofIf+3].getValue());
            posofEndofIf=posofEndofIf+6;
            if( list[posofEndofIf+2].getType()=="INC"){//или можно сделать if(i1<i2){} else{}, не знаю что быстрее,
                //но наверно второе потому что обратиться к локальной переменной всяко быстрее
                //чем лезть по индексу в вектор и у него брать поле
                for (i1;i1<i2;i1++){ //не усралось расшифровка цикла сейчас, надо потом
                    //либо добавить i2-i1 одинаковых строчек, либо париться с циклом потом, а сейчас сделать как с if
                    int  c=posofEndofIf;
                    if((list[c].getType()=="CONDITION")||(list[c].getType()=="CYCLEFOR")||
                       (list[c].getType()=="CYCLEWHILE")||(list[c].getType()=="CYCLEDOWHILE")){
                        ConditionExpression cx(c,list);
                    expressionList.push_back(&cx);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                            localList.push_back(list[posofEndofIf]);
                            posofEndofIf++;}
                        StatementExpression ex(localList);
                        expressionList.push_back(&ex);
                        localList.clear();
                        posofEndofIf++;
                }
            }
            else{
                for (i1;i1>i2;i2--){
                    if((list[posofEndofIf].getType()=="CONDITION")||
                       (list[posofEndofIf].getType()=="CYCLEFOR")||
                       (list[posofEndofIf].getType()=="CYCLEWHILE")||
                       (list[posofEndofIf].getType()=="CYCLEDOWHILE")){
                        ConditionExpression ex(posofEndofIf,list);}
                    while(list[posofEndofIf].getValue()!="SEMICOLON"){
                        localList.push_back(list[posofEndofIf]);
                        posofEndofIf++;}
                    StatementExpression ex(localList);
                    expressionList.push_back(&ex);
                    localList.clear();
                    posofEndofIf++;
                }
            }*/
    void CalcPostfix() {
        for (size_t i = 0; i < postfix.size(); i++)
        {
            if (postfix[i].getValue() == "+" || postfix[i].getValue() == "-" || postfix[i].getValue() == "*" ||
                postfix[i].getValue() == "mod"|| postfix[i].getValue() == "div") {
                double d1, d2;
                d1 = operandStack.Pop();
                d2 = operandStack.Pop();
                if(postfix[i].getValue() == "+"){
                    operandStack.Push(d2 + d1);}
                if(postfix[i].getValue() == "-") {
                    operandStack.Push(d2 - d1);}
                if(postfix[i].getValue() == "*") {
                    operandStack.Push(d2 * d1);}
                if(postfix[i].getValue() == "div") {
                    operandStack.Push(d2 / d1);}
                if(postfix[i].getValue() == "mod") {
                    operandStack.Push(fmod(d2,d1));}
            }
            if (postfix[i].getType()== "VALUEINTEGER"|| postfix[i].getType() == "VALUEREAL") {
                double ans=std::stod(postfix[i].getValue());
                operandStack.Push(ans);
            }
            if (postfix[i].getType()== "VARIABLE"){
                double ans=std::stod(table.findNode(postfix[i].getValue(),table.root)->data.value);
                operandStack.Push(ans);
            }
        }
        res = operandStack.TopView();
        table.Change(type,to_string(res));
    }
    void Build() {
        ToPostfix();
        CalcPostfix();
    }
    TPostfixCalc& operator=(const TPostfixCalc& c) {
        if (&c == this) return *this;
        postfix = c.postfix;
        infix = c.infix;
        operationStack = c.operationStack;
        operandStack = c.operandStack;
        return *this;
    }
/*
    bool operator==(const TPostfixCalc& c) {
        if (infix != c.infix || postfix != c.postfix || operandStack != c.operandStack || operationStack != c.operationStack)
            return false;
        return true;
    }
    bool operator!=(const TPostfixCalc& c) {
        if (infix != c.infix || postfix != c.postfix || operandStack != c.operandStack || operationStack != c.operationStack)
            return true;
        return false;
    }
*/
    friend istream& operator>>(istream& in, TPostfixCalc& c)
    {
        string exp;
        cout << "Ââåäèòå âàøå âûðàæåíèå:"; //што
        in >> exp;
        c.ChangeEquation(exp);
        return in;
    }
    friend ostream& operator<<(ostream& out, const TPostfixCalc& c)
    {
        out << "Infix: ";
        for(auto item:c.infix)
        {out << " " << item.getValue();}
        out << endl;
        if (c.postfix.size() == 0) { out
                    << "Postfix = 0 " << endl; }
        else { out << "Postfix: ";
        for(auto item:c.postfix)
            {out << " " << item.getValue();}
        }
        out << endl;
        out << "Res: " << c.res << endl;

        return out;
    }
};


#endif //POSTFIX_H
