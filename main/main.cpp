
// #include "bar.h"
#include "node.h"

// int main(int argc, char *argv[])
// {
//     Bar bar;
//     Foo foo(bar);
//     foo.baz(true);
//     foo.baz(false);
// }
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("hello world\n");
    Node node("13FF");

    printf("call node %d\n",  node.baz(true));
    node.print();

    Key* k1 = Key::random_key();
    Key* k2 = Key::random_key();
    std::cout << *k1 << std::endl;

    printf("k1 == k2: %d \n", *k1 == *k2);
    return 0;
}