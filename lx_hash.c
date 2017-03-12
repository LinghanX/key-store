#include <stdio.h>

unsigned long hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;
    while(c = *str++)
	hash = ((hash << 5) + hash) + c;
    return hash;
}

int main(int argc, char *argv[]){
    printf("hash string is: %s\n", argv[1]);
    size_t hashed =(size_t) hash(argv[1]);
    printf("hash value is: %d\n", hashed);

    printf("modulo is %d\n", hashed % 2);

    return 0;
}
