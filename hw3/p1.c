#include <stdio.h>
#include <stdlib.h>

#define NUM_ELEMENTS 10

int countEven(int* arr, int size);

int main(int argc, char *argv[]) {
    int *arr = (int*)malloc(NUM_ELEMENTS*sizeof(int));
    int i;

    for (i = 0; i < NUM_ELEMENTS; i++)
        arr[i] = i+1;

    printf("There are %d even numbers from 1 to %d\n",
            countEven(arr, NUM_ELEMENTS), NUM_ELEMENTS);

    free(arr);
    return 0;
}

int countEven(int* arr, int size) {
    int* ptr = arr;
    int i, count = 0;

    for (i = 0; i < size; i++) {
        if ((*ptr % 2) == 0) count++;
        ptr++;
    }

    return count;
}
