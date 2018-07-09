
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
    Node node("00111");

    printf("call node %d\n",  node.baz(true));
    node.print();
    return 0;
}