//
// Created by Linghan Xing on 4/9/17.
//

#include "dict.h"

int main(int argc, char** argv){
    Dict d = DictCreate();


    DictInsert(d, "hello", "world");
    DictInsert(d, "a", "b");
    DictInsert(d, "b", "c");
    DictInsert(d, "c", "d");

    printf("a dictionary with %d k-v pairs;\n", 4);
    //char* key = DictNextKey(d);
    //printf("%s\n", key);
    while(DictSize(d) > 0) {
        printf("%d\n", DictSize(d));
        char* key = DictNextKey(d);
        char* value = DictSearch(d, key);

        printf("removing key-value pair %s-%s from dict.\n", key, value);
        DictDelete(d, key);
    }
}

