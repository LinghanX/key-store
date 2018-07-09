#include "node.h"

Node::Node(string id):
    nodeID(id)
    {}

bool Node::baz(bool foo) {
    if (foo) {
        return true;
    } else {
        return false;
    }
};

void Node::print() {
    cout << "The node is " << nodeID.String() << endl;
}
