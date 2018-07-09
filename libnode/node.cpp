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


class Contact {    
public:
    Contact* next;
    Contact* prev;
    Key nodeID;

    Contact(Key k): nodeID(k) {

    }
    ~Contact() {

    }
};
class ContactList
{
public:
    int size;
    Contact* head;

    ContactList();
    ~ContactList();
    void add(Key k);
    void print();
};

ContactList::ContactList(){
    this->size = 0;
    this->head = NULL;
}

ContactList::~ContactList(){
    std::cout << "LIST DELETED";
}

void ContactList::add(Key k){
    // Contact* node = new Contact(k);
    // node->data = data;
    // node->next = this->head;
    // this->head = node;
    // this->size++;
}

void ContactList::print(){
    // Contact* head = this->head;
    // int i = 1;
    // while(head){
    //     std::cout << i << ": " << head->data << std::endl;
    //     head = head->next;
    //     i++;
    // }
}




class Bucket {
private:
    int capacity;
    Bucket* l;
    Bucket* r;
    ContactList* c;
public:
    Bucket(int K) {
        capacity = K;
    }
    ~Bucket() {

    }
};
