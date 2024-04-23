//
// Created by shuri on 23.04.2024.
//

#ifndef HIERARCHYLIST_H
#define HIERARCHYLIST_H
#include <iostream>
using namespace std;
struct Node {
    int value;
    int priority;
    Node() { value = 0; priority = 0; }
    Node(int a, int b) { value = a; priority = b; }
};
class HierarchyList {
private:
    Node* nextChapter;
    Node* description;
public:
    HierarchyList() {
        nextChapter = new Node(2, 8);
        description = new Node(3, 7);
    }

};

#endif //HIERARCHYLIST_H
