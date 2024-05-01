//
// Created by shuri on 23.04.2024.
//
#ifndef HIERARCHYLIST_H
#define HIERARCHYLIST_H
#include <iostream>
#include "Expression/StatementExpression.h"
#include "Token.h"
#include "Lexer.h"
using namespace std;
template <class Priority, class Value>
class HierarchyList {
private:
    struct Node {
        Node* nextDescription;
        Value value;
        Node(Value v) {
            nextDescription=nullptr;
            value = v;
        }
        Node( Node* d, Value v) {
            nextDescription = d;
            value = v;
        }
        void toSolve(){
            if(this==nullptr)return;
            this->nextDescription->toSolve();
            this->value.toSolve();
            return;
        }
    };
    struct Chapter{
        Priority chapter;
        Chapter* nextChapter;
        Node* description;
        Chapter(Priority p){
            chapter=p;
            nextChapter=nullptr;
            description=nullptr;
        }
    };
    Chapter* root;
public:
    HierarchyList() {
        root=new Chapter("Const");
        root->nextChapter=new Chapter("Var");
        (root->nextChapter)->nextChapter=new Chapter("Body");
    }
    void toAddNext(Value val, Priority pr) {
        Node* new_node= new Node(val);
        Chapter* currentChapter = this->root;
        while(currentChapter->chapter!=pr){
            currentChapter=currentChapter->nextChapter;
        }
        Node* currentNode=currentChapter->description;
        while(currentNode!=nullptr){
            currentNode=currentNode->nextDescription;
        }
        currentNode=new_node;
    }
    void toSolve(){
        root->toSolve();
    }
};

#endif //HIERARCHYLIST_H
