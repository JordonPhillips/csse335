#include <stdio.h>

#define BUFFSIZE 1024

int un(int n);

int main(int argc, char* argv[]) {
    char buffer[BUFFSIZE];
    int n, isInt;

    while (1) {
        printf("Please enter an integer: ");
        fgets(buffer, BUFFSIZE, stdin);
        isInt = sscanf(buffer, "%d", &n);
        if (isInt) break;
    }

    printf("The result of applying u(N) is: %d\n", un(n));

    return 0;
}

int un(int n) {
    int previous = 3;
    int i;

    for (i = 1; i <= n; i++) {
        previous = 3*previous + 4;
    }

    return previous;
}
