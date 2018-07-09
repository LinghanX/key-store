#include <stdio.h>
#include <iostream>
#include "key.h"

using namespace std;

class Node {
private:
    Key nodeID;
public:
    Node(string id);
    bool baz(bool foo);
    void print();
};
