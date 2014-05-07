#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"

void matrix_multiply(Matrix *result, Matrix a, Matrix b) {
    int i, j, k, temp;

    for (i = 0; i < a.height; i++) {
        for (j = 0; j < b.width; j++) {
            temp = 0;

            for (k = 0; k < a.width; k++)
                temp += matrix_get(a, i, k) * matrix_get(b, k, j);

            matrix_set(result, i, j, temp);
        }
    }
}

void matrix_add(Matrix *a, Matrix b) {
    if (a->height != b.height || a->width != b.width) {
        perror("Matricies are not the same dimensions.");
        return;
    }

    int i, j;
    float val;
    for (i = 0; i < a->height; i++) {
        for (j = 0; j < a->width; j++) {
            val = matrix_get(*a, i, j) + matrix_get(b, i, j);
            matrix_set(a, i, j, val);
        }
    }
}

Matrix matrix_malloc(int height, int width) {
    return (Matrix) {
        height,
        width,
        (float *)malloc(width * height * sizeof(float)),
        0
    };
}

void matrix_free(Matrix matrix) {
	free(matrix.data);
}

Matrix matrix_read(char *fname) {
    FILE *fp = fopen(fname, "r");

    if (fp == NULL) {
        perror(fname);
        return (Matrix) {
            0, 0, NULL, 0
        };
    }

    int width, height;
    fscanf(fp, "%d %d", &height, &width);

    if (width < 1 || height < 1) {
        printf("Invalid array size: %d x %d\n", height, width);
        return (Matrix) {
            0, 0, NULL, 0
        };
    }

    char *pch;
    int i, j;
    int line_size = CHARS_PER_FLOAT * width * sizeof(float);
    char *line    = (char *)malloc(line_size);
    Matrix result = matrix_malloc(width, height);

    // Ignoring the first line, which was already read
    fgets(line, line_size, fp);

    for (i = 0; i < height && fgets(line, line_size, fp) != NULL; i++) {
        pch = strtok(line, " ");

        for (j = 0; j < width && pch != NULL; j++) {
            matrix_set(&result, i, j, atof(pch));
            pch = strtok(NULL, " ");
        }
    }

    free(line);
    fclose(fp);
    return result;
}

void matrix_init(Matrix *matrix) {
    int i, j;
    for (i = 0; i < matrix->height; i++)
        for (j = 0; j < matrix->width; j++)
            matrix_set(matrix, i, j, 0);
}

void matrix_print(Matrix matrix) {
    int i, j;
    printf("%d %d\n", matrix.height, matrix.width);

    for (i = 0; i < matrix.height; i++) {
        for (j = 0; j < matrix.width; j++)
            printf("%f ", matrix_get(matrix, i, j));

        printf("\n");
    }
}

void matrix_write(char *fname, Matrix matrix) {
    int i, j;
    FILE *fp = fopen(fname, "w");

    fprintf(fp, "%d %d\n", matrix.height, matrix.width);

    for (i = 0; i < matrix.height; i++) {
        for (j = 0; j < matrix.width; j++)
            fprintf(fp, "%f ", matrix_get(matrix, i, j));

        fprintf(fp, "\n");
    }

    fclose(fp);
}

void matrix_invert(Matrix *matrix) {
    int i, j;
    Matrix result = matrix_malloc(matrix->width, matrix->height);

    for (i = 0; i < matrix->height; i++)
        for (j = 0; j < matrix->width; j++)
            matrix_set(&result, j, i, matrix_get(*matrix, i, j));

    free(matrix->data);
    result.is_inverted = matrix->is_inverted == 1 ? 0 : 1;
    *matrix = result;
}

void matrix_chunk(Matrix *matrix, int num_chunks) {
    int i, j;
    int size         = matrix->width / num_chunks;
    float *data      = (float *)malloc(matrix->height * matrix->width * sizeof(
                                           float));
    Matrix submatrix = {size, size, data, 0};

    for (i = 0; i < num_chunks; i++) {
        for (j = 0; j < num_chunks; j++) {
            matrix_get_submatrix(&submatrix, *matrix, i * num_chunks, j * num_chunks);
            submatrix.data += size * size;
        }
    }

    free(matrix->data);
    matrix->data = data;
}

void matrix_get_submatrix(Matrix *submatrix, Matrix matrix, int start_row,
                          int start_col) {
    int row, col;
    int cell;

    for (row = 0; row < submatrix->height; row++)
        for (col = 0; col < submatrix->width; col++)
            matrix_set(submatrix, row, col, matrix_get(matrix, row + start_row,
                       col + start_col));
}

float matrix_get(Matrix matrix, int row, int col) {
    if (matrix.width < col || matrix.height < row) {
        printf("Matrix out of bounds: getting [%d][%d] from %dx%d\n", row, col,
               matrix.height, matrix.width);
        return -1;
    }

    return matrix.data[matrix_get_index(matrix, row, col)];
}

int matrix_get_index(Matrix matrix, int row, int col) {
    if (matrix.is_inverted == 1)
        return col * matrix.height + row;

    return row * matrix.width + col;
}

void matrix_set(Matrix *matrix, int row, int col, float val) {
    if (matrix->width < col || matrix->height < row) {
        printf("Matrix out of bounds: setting [%d][%d] from %dx%d\n", row, col,
               matrix->height, matrix->width);
        return;
    }

    matrix->data[matrix_get_index(*matrix, row, col)] = val;
}
