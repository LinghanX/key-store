#include "node.h"

Node::Node() {};

bool Node::baz(bool foo) {
    if (foo) {
        return true;
    } else {
        return false;
    }
}
int Node::NewRandomNodeID() {
    return 7; // returns a random number
}

