#include <stdio.h>
#include <stdlib.h>

#define NUM_ELEMENTS 10

void delEven(int* arr, int size);
void printIntArray(int* arr, int size);

int main(int argc, char *argv[]) {
    int *arr = (int*)malloc(NUM_ELEMENTS*sizeof(int));
    int i;

    for (i = 0; i < NUM_ELEMENTS; i++)
        arr[i] = i+1;

    printf("Initial Array:\n");
    printIntArray(arr, NUM_ELEMENTS);

    delEven(arr, NUM_ELEMENTS);

    printf("\nArray after delEven:\n");
    printIntArray(arr, NUM_ELEMENTS);
    printf("\n");

    free(arr);
    return 0;
}

void delEven(int* arr, int size) {
    int* ptr = arr;
    int i;

    for (i = 0; i < size; i++) {
        if ((*ptr % 2) == 0) *ptr = -1;
        ptr++;
    }
}

void printIntArray(int* arr, int size) {
    int i;
    printf("[");
    for (i = 0; i < size - 1; i++) printf("%d, ", arr[i]);
    printf("%d]", arr[size-1]);
}
