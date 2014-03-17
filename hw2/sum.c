#include <stdio.h>

#define N 10
#define BUFFSIZE 1024

int main(int argc, char* argv[]) {
    int i, isInt, n, nsum = 0;
    char buffer[BUFFSIZE];

    for (i = 0; i < N; i++) {
        while (1) {
            printf("Please enter an integer (%d more): ", N - i);
            fflush(stdout);
            fgets(buffer, BUFFSIZE, stdin);
            isInt = sscanf(buffer, "%d", &n);
            if (isInt) break;
        }
        nsum += n;
    }

    printf("The sum of those numbers is %d.\n", nsum);
    return 0;
}
