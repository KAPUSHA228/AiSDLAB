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
    vector<vector<Token>>infixStorage;
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
        if((s=="or")||(s=="xor"))
        {return 2;}
        if((s==">")||(s=="<")||(s==">=")||
           (s=="<=")||(s=="<>")||(s=="="))
        {return 5;}
        if((s=="+")||(s=="-"))
        {return 6;}
        if((s=="*")||(s=="div")||(s=="mod"))
        {return 7;}
        return -1;
    }
public:
    TPostfixCalc() {
        postfix = vector<Token>();
        infix = vector<Token>();
        infixStorage=vector<vector<Token>>();
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
        infixStorage.push_back(infix);
        postfix = vector<Token>();
        operationStack = TStack<string>(eq.length());
        operandStack = TStack<double>(eq.length());
        res=0;
    }
    void ChangeEquation(StatementExpression sx)// фул работает ура
    {
        infix = sx.getList();
        infixStorage.push_back(infix);
       // type=infix[0].getValue();
        postfix = vector<Token>();
        operationStack = TStack<string>(sx.getList().size());
        operandStack = TStack<double>(sx.getList().size());
        res=0;
        if(infix[0].getValue()=="Write"){
           if(infix[3].getType() == "COMMA") {
           cout << infix[2].getValue() << " " << *(table.FindValue(infix[4].getValue()));}
           else {
               if(infix[2].getType()=="VARIABLE"){
                   cout<<table.findNode(infix[2].getValue(),table.root)->data.value;}
               else{
                   cout<<infix[2].getValue();}
           }
           infixStorage.pop_back();
           return;
        }
        if(infix[0].getValue()=="Writeln"){
            if(infix[3].getType() == "COMMA") {
                cout << infix[2].getValue() << " " << *(table.FindValue(infix[4].getValue()));}
            else {
                if(infix[2].getType()=="VARIABLE"){cout<<table.findNode(infix[2].getValue(),table.root)->data.value;}
                else{cout<<infix[2].getValue();}
            }
            cout<<endl;
            infixStorage.pop_back();
            return;
        }
        if(infix[0].getValue()=="Read"){
           string value;
           cin >> value;
           table.Change(infix[2].getValue(),value);
           infixStorage.pop_back();
           return;
        }
        if(infix[0].getValue()=="Readln"){
           string value;
           cin >> value;
           table.Change(infix[2].getValue(),value);
           cout<<endl;
           infixStorage.pop_back();
           return;
        }
        else{ //отсекли консоль, теперь объявления и выражения
            int i=0;
            while((infix[i].getValue()!=":")&&(i!=(infix.size()-1))){i++;} // токен ":" присутствует только в объявлениях и константах
            if(i==(infix.size()-1)){  //соответственно, если дошли до конца, то значит ":" не нашли и просто билдим
                Build();
                infixStorage.pop_back();
            }else
            {
                if(i!=infix.size()-2)
                {
                    table.Insert(infix[0].getValue(),infix[4].getValue(),infix[2].getValue());
                }else
                {
                    toDeclarate(infix);
                }
                infixStorage.pop_back();
            }
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
    void ChangeEquation(ConditionExpression cx) {
        infix = cx.getBody().first;
        infixStorage.push_back(infix);
        auto body=cx.getBody().second;
        postfix = vector<Token>();
        operationStack = TStack<string>(cx.getCondition().size());
        operandStack = TStack<double>(cx.getCondition().size());
        res=0;
        if(infix.front().getValue()=="if"){
            ToPostfixCondition(cx.getCondition());
            if(CalcCondition()==1){
                for(auto item:body){
                    if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                        ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                    } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                        ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                    }
                }
            }
            infixStorage.pop_back();
            return;
        }
        if(infix.front().getValue()=="else"){
            auto iter=infix.begin();
            infix.erase(iter);
            ToPostfixCondition(cx.getCondition());
            if(CalcCondition()!=1){
                for(auto item:body){
                    if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                        ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                    } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                        ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                    }
                }
            }
            infixStorage.pop_back();
            return;
        }
        if(infix.front().getValue()=="while"){
            ToPostfixCondition(cx.getCondition());
            while(CalcCondition()==1){
                for(auto item:body){
                    if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                        ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                    } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                        ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                    }
                }
                ToPostfixCondition(cx.getCondition());
            }
            infixStorage.pop_back();
            return;
        }
        if(infix.front().getValue()=="until") {
            do{
                for(auto item:body){
                    if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                        ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                    } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                        ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                    }
                }
                ToPostfixCondition(cx.getCondition());
            }while(CalcCondition()==1);
            infixStorage.pop_back();
            return;
        }
        if(infix.front().getValue()=="for") {
            int i1,i2;
            string nameValue=infix[1].getValue();
            i1=std::stoi(infix[3].getValue());
            i2=std::stoi(infix[5].getValue());
            if(i1<i2){
                for (i1;i1<i2;i1++){
                    table.Change(nameValue,to_string(i1));
                    for(auto item:body){
                        if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                            ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                        } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                            ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                        }
                    }
                }
            }
            else{
                for (i1;i1>i2;i2--){
                    table.Change(infix[1].getValue(),to_string(i1));
                    for(auto item:body){
                        if (auto statementExpr = dynamic_cast<StatementExpression*>(item)) {
                            ChangeEquation(*statementExpr); // Вызов метода для StatementExpression
                        } else if (auto conditionExpr = dynamic_cast<ConditionExpression*>(item)) {
                            ChangeEquation(*conditionExpr); // Вызов метода для ConditionExpression
                        }
                    }
                }
            }
            return;
        }
        return;
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
        for (size_t i = 0; i < s.size(); i++)
        {
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
                postfix.push_back(s[i]);
                continue; }
            if (s[i].getType() == "JG" || s[i].getType() == "MOD"||s[i].getType() == "NOT"
            || s[i].getType() == "AND" ||s[i].getType() == "OR" || s[i].getType() == "XOR"
            || s[i].getType() == "JL" || s[i].getType() == "JGE" ||s[i].getType() == "JLE"
            ||s[i].getType() == "JNE"||s[i].getType() == "JE" || s[i].getType() == "DIV" ||
            s[i].getType() == "PLUS" || s[i].getType() == "MINUS"||s[i].getType() == "MULTI") {
                if(operationStack.IsEmpty()){
                    operationStack.Push(s[i].getValue());
                    continue;}
                el = operationStack.Pop();
                while (PriorityCond(s[i].getValue()) <= PriorityCond(el)) {
                    if(el[0]=='-'){ Token t={"MINUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='+'){Token t={"PLUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='*'){Token t={"MULTI",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='d'){Token t={"DIV",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='m'){Token t={"MOD",el,0};
                        postfix.push_back(t);}
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
                continue;
            }
            if (s[i].getValue() == "("){
                operationStack.Push(s[i].getValue());
                continue;}
            if (s[i].getValue() == ")") {
                el = operationStack.Pop();

                while (el != "(") {
                    if(el[0]=='-'){Token t={"MINUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='+'){Token t={"PLUS",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='*'){Token t={"MULTI",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='d'){Token t={"DIV",el,0};
                        postfix.push_back(t);}
                    if(el[0]=='m'){ Token t={"MOD",el,0};
                        postfix.push_back(t);}
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
                continue;
            }
            else{continue;}
        }
    }
    bool CalcCondition()
    {
        for (size_t i = 0; i < postfix.size(); i++)
        {
            if( postfix[i].getValue() == "not") {
                double d1;
                d1=operandStack.Pop();
                d1==1?operandStack.Push(0):operandStack.Push(1);
                continue;
            }
            if(postfix[i].getValue() == "and" || postfix[i].getValue() == "or" || postfix[i].getValue() == "xor"){
                double d1, d2;
                d1 = operandStack.Pop();
                d2 = operandStack.Pop();
                if(postfix[i].getValue() == "and"){
                    ((d1==1)&&(d2==1))?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == "or"){
                    ((d1==1)||(d2==1))?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == "xor"){
                    ((d1==1)^(d2==1))?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
            }
            if (postfix[i].getValue() == "<" || postfix[i].getValue() == "<>" || postfix[i].getValue() == "<="||
            postfix[i].getValue() == ">"|| postfix[i].getValue() == ">=" || postfix[i].getValue() == "=" ||
            postfix[i].getValue() == "+" || postfix[i].getValue() == "-" || postfix[i].getValue() == "*" ||
            postfix[i].getValue() == "mod"|| postfix[i].getValue() == "div") {
                double d1, d2;
                d1 = operandStack.Pop();
                d2 = operandStack.Pop();
                if(postfix[i].getValue() == "+"){
                    operandStack.Push(d2 + d1);
                    continue;
                }
                if(postfix[i].getValue() == "-") {
                    operandStack.Push(d2 - d1);
                    continue;
                }
                if(postfix[i].getValue() == "*") {
                    operandStack.Push(d2 * d1);
                    continue;
                }
                if(postfix[i].getValue() == "div") {
                    operandStack.Push(d2 / d1);
                    continue;
                }
                if(postfix[i].getValue() == "mod") {
                    operandStack.Push(fmod(d2,d1));
                    continue;
                }
                if(postfix[i].getValue() == "<>"){
                    d2!=d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == "<="){
                    d2<=d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == "<"){
                    d2<d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == ">="){
                    d2>=d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == ">"){
                    d2>d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
                if(postfix[i].getValue() == "="){
                    d2==d1?operandStack.Push(1):operandStack.Push(0);
                    continue;
                }
            }
            if (postfix[i].getType()== "VALUEINTEGER"|| postfix[i].getType() == "VALUEREAL") {
                double ans=std::stod(postfix[i].getValue());
                operandStack.Push(ans);
                continue;

            }
            if (postfix[i].getType()== "VARIABLE"){
                double ans=std::stod(table.findNode(postfix[i].getValue(),table.root)->data.value);
                operandStack.Push(ans);
                continue;

            }
        }
        res = operandStack.TopView();
        if(res==1) return true;
        else return false;
    }
    void CalcPostfix()
    {
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
    bool operator==(const TPostfixCalc& c) {
        if (infix.begin() != c.infix.begin() || postfix.begin() != c.postfix.begin() || operandStack != c.operandStack || operationStack != c.operationStack)
            return false;
        return true;
    }
    bool operator!=(const TPostfixCalc& c) {
        if (infix.begin() != c.infix.begin() || postfix.begin() != c.postfix.begin() || operandStack != c.operandStack || operationStack != c.operationStack)
            return true;
        return false;
    }
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
