#include <stdio.h>
#include <stdlib.h>

#define NUM_DINERS 10

int dinercmp(const void *a, const void *b);

typedef struct {
    int index;
    int num;
} diner;

int main(int argc, char* argv[]) {
    diner diners[NUM_DINERS];
    diner *d;
    int i, n;

    for (i = 0; i < NUM_DINERS; i++) {
        printf("How many pancakes did person %d eat? ", i+1);
        scanf("%d", &n);

        d = (diner*)malloc(sizeof(diner));
        *d = (diner){i, n};
        diners[i] = *d;
        free(d);
    }

    qsort(diners, NUM_DINERS, sizeof(diner), dinercmp);

    printf("Person %d ate the most pancakes.\n", diners[NUM_DINERS-1].index + 1);
    printf("Person %d ate the least pancakes.\n", diners[0].index + 1);
    return 0;
}

int dinercmp(const void *a, const void *b) {
    const diner *d1 = a, *d2 = b;
    const int c = (*d1).num, d = (*d2).num;
    return (c < d) ? -1 : (c > d);
}
