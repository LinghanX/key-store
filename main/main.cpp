
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
    Node node;
    printf("call node %d\n",  node.baz(true));
    return 0;
}