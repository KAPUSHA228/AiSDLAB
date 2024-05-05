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
class TPostfixCalc // не доделан под нужды уравнений с переменными и сравнений
{
private:
    std::string infix;
    string postfix;
    TStack<char> operationStack;
    TStack<double> operandStack;
    double res;
protected:
    int Priority(char c) {
        switch (c)
        {
            case '(':
            case ')':
                return 1;
            case '+':
            case '-':
                return 2;
            case '*':
            case '/':
                return 3;
            default: throw - 1;
        }
    }
    int PriorityCond(string s){
        if((s=="(")||(s==")")){return 1;}
        if(s=="not"){return 4;}
        if(s=="and"){return 3;}
        if((s=="or")||(s=="xor")){return 2;}
        if((s==">")||(s=="<")||
        (s==">=")||(s=="<=")||
        (s=="<>")||(s=="=")){return 5;}
    }
public:
    TPostfixCalc() {
        postfix = "";
        infix = "";
        operationStack = TStack<char>();
        operandStack = TStack<double>();
        res = 0;
    }

    TPostfixCalc(string eq) {
        if (eq.length() <= 0) throw std::runtime_error{"ïóñòàÿ ñòðîêà"};
        postfix = "";
        infix = eq;
        operationStack = TStack<char>(eq.length());
        operandStack = TStack<double>(eq.length());
        res = 0;
    }
    TPostfixCalc(std::vector<Token> v){
        for (int i=0; i<v.size()-1;i++){
            infix+=v[i].getValue();
        }

    }

    TPostfixCalc(StatementExpression v){
        for (int i=0; i<v.getList().size();i++){
            infix+=v.getList()[i].getValue();
        }
    }
    TPostfixCalc(ConditionExpression v){
        auto body = v.getBody();
        auto con=body.first;
        auto exp=body.second;
        if(con[0].getValue()!="for") ToPostfixCondition(con);
    }
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
    TPostfixCalc(const TPostfixCalc& c) {
        if (&c == this) throw std::runtime_error{"Íå ìîæåò áûòü ïðèñâîåí ýëåìåíò ñàìîìó ñåáå"};
        postfix = c.postfix;
        infix = c.infix;
        operationStack = c.operationStack;
        operandStack = c.operandStack;
        res = c.res;
    }

    ~TPostfixCalc() {}

    void ChangeEquation(string s) {
        infix = s;
        operationStack = TStack<char>(s.length());
        operandStack = TStack<double>(s.length());
    }
   /*void ChangeEquation(StatementExpression s) {
        infix = s;
        operationStack = TStack<char>(s.length());
        operandStack = TStack<double>(s.length());
    }
    void ChangeEquation(ConditionExpression s) {
        infix = s;
        operationStack = TStack<char>(s.length());
        operandStack = TStack<double>(s.length());
    }*/
    string GetInf() { return infix; }
    string GetPost() { return postfix; }
    double GetRes() { return res; }
    void ToPostfix() {
        char el;
        postfix = "";
        string s = "(" + infix + ")";
        for (size_t i = 0; i < s.size(); i++)
        {
            if (s[i] <= '9' && s[i] >= '0') postfix += s[i];
            if (s[i] == '/' || s[i] == '*' || s[i] == '+' || s[i] == '-') {
                postfix += " ";
                el = operationStack.Pop();
                while (Priority(s[i]) <= Priority(el)) {
                    postfix += el;
                    postfix += " ";
                    el = operationStack.Pop();
                }
                operationStack.Push(el);
                operationStack.Push(s[i]);
            }
            if (s[i] == '(') operationStack.Push(s[i]);
            if (s[i] == ')') {
                el = operationStack.Pop();

                while (el != '(') {
                    postfix += el;
                    el = operationStack.Pop();
                }
            }
        }
    }
    void ToPostfixCondition(vector<Token>condition){
       /* string type= condition[0].getValue();
        vector<Token> copy=condition;
        auto iter=copy.begin();
        copy.erase(iter);
        for(auto item:copy){
            infix+=item.getValue();
        }
        infix="(" + infix + ")";
        string el;
        postfix = "";

        for (size_t i = 0; i < copy.size(); i++)
        {
            if ((copy[i].getType() == "VALUEINTEGER")||(copy[i].getType() == "VALUEREAL")
            {postfix += copy[i].getValue();}
            if (copy[i].getValue() == '(') operationStack.Push(s[i]);
            if (copy[i].getValue() == ')') {
                el = operationStack.Pop();

                while (el != "(") {
                    postfix += el;
                    el = operationStack.Pop();
                }
            }
            if (s[i] == '/' || s[i] == '*' || s[i] == '+' || s[i] == '-') {
                postfix += " ";
                el = operationStack.Pop();
                while (Priority(s[i]) <= Priority(el)) {
                    postfix += el;
                    postfix += " ";
                    el = operationStack.Pop();
                }
                operationStack.Push(el);
                operationStack.Push(s[i]);
            }
        }*/
    }
    bool CalcCondition(string type,vector<Token>condition){
        int i=0;
        if(condition[0].getValue()=="if"){
           while((condition[i].getValue()!="and")&&(condition[i].getValue()!="or"))
           {
               i++;
           }
           if(condition[i].getValue()=="or"){ }
           if(condition[i].getValue()=="and"){}
           else{}

       }
       //if(typeOfCond=="else"){}
       if(condition[0].getValue()=="for"){}
       if(condition[0].getValue()=="while"){}
       if(condition[0].getValue()=="until"){}

    }
    void CalcPostfix(SearchTreeTable<string,double>&table) {
        for (size_t i = 0; i < postfix.size(); i++)
        {
            if (postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/') {
                double d1, d2;
                switch (postfix[i]) {
                    case '+':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 + d2);
                        break;
                    case '-':
                        d2 = operandStack.Pop();
                        d1 = operandStack.Pop();
                        operandStack.Push(d1 - d2);
                        break;
                    case '*':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 * d2);
                        break;
                    case '/':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 / d2);
                        break;
                    default: throw "ââåäè ÷î íèòü òî";
                }
            }
            if (postfix[i] <= '9' && postfix[i] >= '0') {
                size_t count;
                double t = std::stod(&postfix[i], &count);
                operandStack.Push(t);
                i += count - 1;
            }
        }
        res = operandStack.TopView();
    }
    void CalcPostfix() {
        for (size_t i = 0; i < postfix.size(); i++)
        {
            if (postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/') {
                double d1, d2;
                switch (postfix[i]) {
                    case '+':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 + d2);
                        break;
                    case '-':
                        d2 = operandStack.Pop();
                        d1 = operandStack.Pop();
                        operandStack.Push(d1 - d2);
                        break;
                    case '*':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 * d2);
                        break;
                    case '/':
                        d1 = operandStack.Pop();
                        d2 = operandStack.Pop();
                        operandStack.Push(d1 / d2);
                        break;
                    default: throw "ââåäè ÷î íèòü òî";
                }
            }
            if (postfix[i] <= '9' && postfix[i] >= '0') {
                size_t count;
                double t = std::stod(&postfix[i], &count);
                operandStack.Push(t);
                i += count - 1;
            }
        }
        res = operandStack.TopView();
    }
    void Build(SearchTreeTable<string,double>&table) {
        ToPostfix();
        CalcPostfix(table);
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
        if (infix != c.infix || postfix != c.postfix || operandStack != c.operandStack || operationStack != c.operationStack)
            return false;
        return true;
    }
    bool operator!=(const TPostfixCalc& c) {
        if (infix != c.infix || postfix != c.postfix || operandStack != c.operandStack || operationStack != c.operationStack)
            return true;
        return false;
    }

    friend istream& operator>>(istream& istr, TPostfixCalc& c)
    {
        string exp;
        cout << "Ââåäèòå âàøå âûðàæåíèå:";
        istr >> exp;
        c.ChangeEquation(exp); return istr;
    }
    friend ostream& operator<<(ostream& ostr, const TPostfixCalc& c)
    {
        ostr << "Íà÷àëüíîå âûðàæåíèå: " << c.infix << endl;
        if (c.postfix.size() == 0) { ostr
                    << "Âûðàæåíèå ïóñòîå èëè íå ïîäëåæèò âûñ÷èòûâàíèþ" << endl; }
        else { ostr << "Ðåçóëüòàò: " << c.postfix << endl; }
        ostr << "Êîíå÷íîå ðåøåíèå: " << c.res << endl;

        return ostr;
    }
};


#endif //POSTFIX_H
