#include <stdio.h>

int myStrLen(char* s);

int main(int argc, char *argv[]) {
    char *str = "MALO MORI QUAM FOEDARI";
    printf("The length of the string '%s' is: %d\n", str, myStrLen(str));
    return 0;
}

int myStrLen(char* s) {
    int len;
    char *p = s;
    for (len = 0; p[len] != '\0'; len++) ;
    return len;
}
