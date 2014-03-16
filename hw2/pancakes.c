#include <stdio.h>

#define NUM_DINERS 10

int main(int argc, char* argv[]) {
    int diners[NUM_DINERS];
    int i, j, n;

    for (i = 0; i < NUM_DINERS; i++) {
        printf("How many pancakes did person %d eat? ", i+1);
        scanf("%d", &n);
        diners[i] = n;
    }

    n = diners[0];
    for (i = 1; i < NUM_DINERS; i++) {
        if (n < diners[i]) {
            n = diners[i];
            j = i;
        }
    }

    printf("Person %d ate the most pancakes.\n", j+1);
    return 0;
}
