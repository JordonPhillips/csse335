#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.c"

void master(char *matrix_fname, char *vector_fname, char *out_fname);
void slave();
void MPI_matrix_vector_multiply(Matrix *result, Matrix *matrix, Matrix *vector,
                                int n, int root, MPI_Comm comm);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        if (argc == 4)
            master(argv[1], argv[2], argv[3]);
        else
            printf("Incorrect argument count. Expected 3, recieved %d\n", argc - 1);
    } else if (argc == 4)
        slave();

    MPI_Finalize();
    return 0;
}

void master(char *matrix_fname, char *vector_fname, char *out_fname) {
    Matrix matrix = matrix_read(matrix_fname);
    Matrix vector = matrix_read(vector_fname);

    if (matrix.height != matrix.width || matrix.height != vector.height
            || vector.width != 1) {
        printf("Invalid inputs. Matrix must be nxn (was %dx%d) and vector must"
               " be nx1 (was %dx%d)\n", matrix.height, matrix.width, vector.height,
               vector.width);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    MPI_Bcast(&(matrix.width), 1, MPI_INT, 0, MPI_COMM_WORLD);

    Matrix result = matrix_malloc(vector.height, 1);

    MPI_matrix_vector_multiply(&result, &matrix, &vector, matrix.height, 0,
                               MPI_COMM_WORLD);
    free(matrix.data);
    free(vector.data);

    matrix_write(out_fname, result);

    if (result.height <= 20)
        matrix_print(result);

    free(result.data);
}

void slave() {
    int n;

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_matrix_vector_multiply(NULL, NULL, NULL, n, 0, MPI_COMM_WORLD);
}

void MPI_matrix_vector_multiply(Matrix *result, Matrix *matrix, Matrix *vector,
                                int n, int root, MPI_Comm comm) {
    int rank, total_procs;


    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int num_vals = n % total_procs == 0 ? n / total_procs : -1;

    if (num_vals < 1) {
        printf("Number of values is not evenly divisible by number of procedures\n");
        return;
    }

    // ##################################################
    // Step 0: Distribute A
    // ##################################################

    if (rank == root)
        matrix_invert(matrix);

    Matrix A = matrix_malloc(n, num_vals);
    MPI_Scatter(rank == root ? matrix->data : NULL, num_vals * n, MPI_FLOAT, A.data,
                num_vals * n, MPI_FLOAT, root, comm);

    double start_time = MPI_Wtime();

    // ##################################################
    // Step 1: Scatter x
    // ##################################################

    Matrix x = matrix_malloc(num_vals, 1);
    MPI_Scatter(rank == root ? vector->data : NULL, num_vals, MPI_FLOAT, x.data,
                num_vals, MPI_FLOAT, root, comm);

    // ##################################################
    // Step 2: Multiply
    // ##################################################

    Matrix b = matrix_malloc(n, 1);
    matrix_multiply(&b, A, x);

    // ##################################################
    // Step 3: Reduce
    // ##################################################

    MPI_Reduce(b.data, rank == root ? result->data : NULL, n, MPI_FLOAT, MPI_SUM,
               root, comm);

    if (rank == root)
        printf("Multiplication took %f seconds.\n", MPI_Wtime() - start_time);

    free(x.data);
    free(A.data);
    free(b.data);
}
