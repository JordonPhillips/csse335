#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 256

void revString(char* s);

int main(int argc, char *argv[]) {
    char *str = (char*)malloc(SIZE*sizeof(char));
    strcpy(str,"MALO MORI QUAM FOEDARI");
    printf("The reverse of '%s' ", str);
    revString(str);
    printf("is '%s'\n", str);
    free(str);
    return 0;
}

void revString(char* a) {
    char *p1 = a, *p2 = a, tmp;
    while (*p1 != '\0') p1++;
    while (p1 != p2) {
        p1--;
        tmp = *p1;
        *p1 = *p2; // SEG FAULT
        *p2 = tmp;
        p2++;
    }
}
