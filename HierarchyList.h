//
// Created by shuri on 23.04.2024.
//

#ifndef HIERARCHYLIST_H
#define HIERARCHYLIST_H
#include <iostream>
#include "Expression/RunnableExpression.h"
#include "Token.h"
#include "Lexer.h"
using namespace std;
template <class Priority, class Value>
class HierarchyList {
private:
    struct Node {
        Node* nextChapter;
        Node* description;
        Value value;
        Priority priority;

        Node(Value v, Priority p) {
            nextChapter=nullptr;
            description=nullptr;
            value = v;
            priority = p; }
        Node(Node* n, Node* d, Value v, Priority p) {
            nextChapter=n;
            description=d;
            value = v;
            priority = p;
        }
        void toSolve(){
            if(this==nullptr)return;
            this->description->toSolve();
            this->value.toSolve();
            this->nextChapter->toSolve();
        }

    };
    Node* home;
public:
    HierarchyList() {
        home=nullptr;
    }
    HierarchyList(Value v, Priority p) {
        home=new Node(v,p);
    }
    HierarchyList(Node* n, Node* d, Value v, Priority p) {
        home=new Node(n,d,v,p);
    }
    void toAddNext(Value val, Priority pr) {
        Node* new_node= new Node(val,pr);
        if(this->home==nullptr){
            this->home=new_node;
        }
        Node* current=this->home;
        while(current!=nullptr) {
            if (current->priority == pr) {
                current=current->description;
            } else {
                current=current->nextChapter;
            }
        }current=new_node;
    }
    void toSolve(){
        home->toSolve();
    }
};

#endif //HIERARCHYLIST_H
