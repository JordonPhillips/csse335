#include <stdio.h>

#define BUFFSIZE 1024

int main(int argc, char* argv[]) {
    char buffer[BUFFSIZE];
    int n, isInt, i, j;

    while (1) {
        printf("Please enter an integer: ");
        fgets(buffer, BUFFSIZE, stdin);
        isInt = sscanf(buffer, "%d", &n);
        if (isInt) break;
    }

    for (i = n; i > 0; i--) {
        for (j = 0; j < n - i; j++)
            printf(" ");
        for (j = 0; j < i; j++)
            printf("*");
        printf("\n");
    }

    return 0;
}
