#include <stdio.h>
#include <stdlib.h>

#define SIZE 15

double* maximum(double* a, int size);

int main(int argc, char *argv[]) {
    double* arr = (double*)malloc(SIZE*sizeof(double));
    int i;

    for (i = 0; i < SIZE; i++)
        arr[i] = i + 1;

    printf("The max of a double array that goes from 1 to %d is: %f\n",
            SIZE, *(maximum(arr, SIZE)));

    free(arr);
    return 0;
}

double* maximum(double* a, int size) {
    double *p1 = a;
    int i;
    for (i = 1; i < size; i++)
        if (*p1 < *(a+i)) p1++;
    return p1;
}
