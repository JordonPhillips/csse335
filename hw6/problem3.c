#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARS_PER_FLOAT 16

typedef struct {
    int height;
    int width;
    float *data;
} Matrix;

void  master(char *a_fname, char *b_fname, char *out_fname);
void  slave();

Matrix matrix_read(char *fname);
Matrix matrix_malloc(int height, int width);
void  matrix_print(Matrix matrix);
void  matrix_write(char *fname, Matrix matrix);

float matrix_get(Matrix matrix, int row, int col);
void  matrix_get_submatrix(Matrix *submatrix, Matrix matrix, int start_row,
                           int start_col);

void matrix_set(Matrix *matrix, int row, int col, float val);
void matrix_invert(Matrix *matrix);
void matrix_chunk(Matrix *matrix, int sqrt_total_procs);

void  matrix_multiply(Matrix *result, Matrix a, Matrix b);
void  MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int root,
                          MPI_Comm comm);

int   cart_rank_to_rank(int x, int y, int sqrt_total_procs);
void  cart_rank(int rank, int sqrt_total_procs, int *i, int *j);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        if (argc == 4)
            master(argv[1], argv[2], argv[3]);
        else
            printf("Incorrect argument count. Expected 3, recieved %d\n", argc - 1);
    } else {
        if (argc == 4)
            slave();
    }

    MPI_Finalize();
    return 0;
}

void master(char *a_fname, char *b_fname, char *out_fname) {
    Matrix a = matrix_read(a_fname);
    Matrix b = matrix_read(b_fname);

    if (a.width != a.height || b.width != b.height || a.width != b.width) {
        printf("Invalid inputs. Both matricies must be nxn. A was %dx%d. B was"
               " %dx%d", a.height, a.width, b.height, b.width);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    int total_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &total_procs);
    int sqrt_total_procs = sqrt(total_procs);

    if (total_procs != sqrt_total_procs * sqrt_total_procs) {
        printf("Number of processes (%d) is not a perfect square.\n", total_procs);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    Matrix result = matrix_malloc(a.width, b.width);
    double start = MPI_Wtime();
    MPI_matrix_multiply(&result, &a, &b, 0, MPI_COMM_WORLD);
    printf("Multiplication took %f seconds.\n", MPI_Wtime() - start);
    //matrix_write(out_fname, result);

    if (result.width <= 20)
        matrix_print(result);

    free(a.data);
    free(b.data);
    free(result.data);
}

void slave() {
    MPI_matrix_multiply(NULL, NULL, NULL, 0, MPI_COMM_WORLD);
}

void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int root,
                         MPI_Comm comm) {
    /*int rank, total_procs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int sqrt_total_procs = sqrt(total_procs);
    int v_size           = n / sqrt_total_procs;
    int a_size           = v_size * v_size;

    // ##################################################
    // Step 0: Distribute A
    // ##################################################

    float *A = (float *)malloc(a_size * sizeof(float));
    MPI_Scatter(send_matrix, a_size, MPI_FLOAT, A, a_size, MPI_FLOAT, root, comm);

    int rank_i, rank_j;
    cart_rank(rank, sqrt_total_procs, &rank_i, &rank_j);

    // ##################################################
    // Step 1: Distribuite x
    // This is done in 'waves'. First root
    // scatters it to its row, then each process
    // in that row sends it to the process 'below'
    // itself.
    // ##################################################

    MPI_Comm row_comm;
    MPI_Comm_split(comm, rank_i, rank_j, &row_comm);
    float *x = (float *)malloc(v_size * sizeof(float));

    int root_row = root / sqrt_total_procs;
    MPI_Status status;

    if (rank_i == root_row)
        MPI_Scatter(vector, v_size, MPI_FLOAT, x, v_size, MPI_FLOAT, root, row_comm);
    else {
        int sender = cart_rank_to_rank(rank_i - 1, rank_j, sqrt_total_procs);
        MPI_Recv(x, v_size, MPI_FLOAT, sender, MPI_ANY_TAG, comm, &status);
    }

    int final_row = (root_row - 1) % sqrt_total_procs;

    if (final_row < 0) final_row += sqrt_total_procs;

    if (rank_i != final_row) {
        int reciever = cart_rank_to_rank(rank_i + 1, rank_j, sqrt_total_procs);
        MPI_Send(x, v_size, MPI_FLOAT, reciever, 0, comm);
    }

    // ##################################################
    // Step 2: Aij * xi
    // ##################################################

    float *b = (float *)malloc(v_size * sizeof(float));
    matrix_vector_multiply(b, A, x, v_size);
    free(A);

    // ##################################################
    // Step 3: Reduce
    // ##################################################

    // Re-using x as a temporary variable since I don't need the info anymore
    MPI_Reduce(b, x, v_size, MPI_FLOAT, MPI_SUM, 0, row_comm);

    // ##################################################
    // Step 4: Gather to root
    // ##################################################

    MPI_Comm column_comm;
    MPI_Comm_split(comm, rank_j, rank_i, &column_comm);

    if (rank_j == 0)
        MPI_Gather(x, v_size, MPI_FLOAT, result, v_size, MPI_FLOAT, 0, column_comm);

    free(x);
    free(b);
    */
}

void matrix_multiply(Matrix *result, Matrix a, Matrix b) {
    int i, j, k, temp;

    for (i = 0; i < a.height; i++) {
        for (j = 0; j < b.height; j++) {
            temp = 0;

            for (k = 0; k < a.width; k++)
                temp += matrix_get(a, i, k) * matrix_get(b, k, j);

            matrix_set(result, i, j, temp);
        }
    }
}

Matrix matrix_malloc(int height, int width) {
    return (Matrix) {
        height,
        width,
        (float *)malloc(width * height * sizeof(float))
    };
}

Matrix matrix_read(char *fname) {
    FILE *fp = fopen(fname, "r");

    if (fp == NULL) {
        perror(fname);
        return (Matrix) {
            0, 0, NULL
        };
    }

    int width, height;
    fscanf(fp, "%d %d", &height, &width);

    if (width < 1 || height < 1) {
        printf("Invalid array size: %d x %d", height, width);
        return (Matrix) {
            0, 0, NULL
        };
    }

    char *pch;
    int i, j;
    int line_size = CHARS_PER_FLOAT * width * sizeof(float);
    char *line = (char *)malloc(line_size);
    Matrix result = matrix_malloc(width, height);

    // Ignoring the first line, which was already read
    fgets(line, line_size, fp);

    for (i = 0; i < height && fgets(line, line_size, fp) != NULL; i++) {
        pch = strtok(line, " ");

        for (j = 0; j < width && pch != NULL; j++) {
            matrix_set(&result, i, j, atof(pch));
            pch     = strtok(NULL, " ");
        }
    }

    free(line);
    fclose(fp);
    return result;
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
}

void matrix_chunk(Matrix *matrix, int sqrt_total_procs) {
    int i, j;
    int size   = matrix->width / sqrt_total_procs;
    Matrix submatrix = {size, size, matrix->data};

    for (i = 0; i < sqrt_total_procs; i++) {
        for (j = 0; j < sqrt_total_procs; j++) {
            matrix_get_submatrix(&submatrix, *matrix, i, j);
            submatrix.data += size * size;
        }
    }
}

void matrix_get_submatrix(Matrix *submatrix, Matrix matrix, int start_row,
                          int start_col) {
    int i, j;

    for (i = 0; i < submatrix->height; i++)
        for (j = 0; j < submatrix->width; j++)
            matrix_set(submatrix, i, j, matrix_get(matrix, i + start_row, j + start_col));
}

float matrix_get(Matrix matrix, int row, int col) {
    if (matrix.width < col || matrix.height < row) {
        printf("Matrix out of bounds: getting [%d][%d] from %dx%d", row, col,
               matrix.height, matrix.width);
        return -1;
    }

    return matrix.data[row * matrix.width + col];
}

void matrix_set(Matrix *matrix, int row, int col, float val) {
    if (matrix->width < col || matrix->height < row) {
        printf("Matrix out of bounds: getting [%d][%d] from %dx%d", row, col,
               matrix->height, matrix->width);
        return;
    }

    matrix->data[row * matrix->width + col] = val;
}

int cart_rank_to_rank(int x, int y, int sqrt_total_procs) {
    return x * sqrt_total_procs + y;
}

void cart_rank(int rank, int sqrt_total_procs, int *i, int *j) {
    *i = rank / sqrt_total_procs;
    *j = rank % sqrt_total_procs;
}

