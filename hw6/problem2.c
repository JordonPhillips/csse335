#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.c"

#define CHARS_PER_FLOAT 16

void  master(char *matrix_fname, char *vector_fname, char *out_fname);
void  slave();
void  MPI_matrix_vector_multiply(Matrix *result, Matrix *a, Matrix *b, int n, int root, MPI_Comm comm);
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
    } else if (argc == 4) {
        slave();
    }

    MPI_Finalize();
    return 0;
}

void master(char *matrix_fname, char *vector_fname, char *out_fname) {
    Matrix matrix = matrix_read(matrix_fname);
    Matrix vector = matrix_read(vector_fname);

    if (matrix.height != matrix.width || matrix.height != vector.height || vector.width != 1) {
        printf("Invalid inputs. Matrix must be nxn (was %dx%d) and vector must"
               " be nx1 (was %dx%d)\n", matrix.height, matrix.width, vector.height, vector.width);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    int total_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &total_procs);
    int sqrt_total_procs = sqrt(total_procs);

    if (total_procs != sqrt_total_procs * sqrt_total_procs) {
        printf("Number of processes (%d) is not a perfect square.\n", total_procs);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    if (matrix.width % sqrt_total_procs != 0) {
        printf("Matrix size is not easily divisible among the number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    matrix_chunk(&matrix, sqrt_total_procs);

    MPI_Bcast(&(matrix.width), 1, MPI_INT, 0, MPI_COMM_WORLD);

    Matrix result = matrix_malloc(vector.height, 1);

    MPI_matrix_vector_multiply(&result, &matrix, &vector, matrix.width, 0, MPI_COMM_WORLD);

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

void MPI_matrix_vector_multiply(Matrix *result, Matrix *matrix, Matrix *vector, int n, int root, MPI_Comm comm) {
    int rank, total_procs;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int sqrt_total_procs = sqrt(total_procs);
    int v_size           = n / sqrt_total_procs;
    int a_size           = v_size * v_size;

    // ##################################################
    // Step 0: Distribute A
    // ##################################################

    Matrix A = matrix_malloc(v_size, v_size);
    MPI_Scatter(rank == root ? matrix->data : NULL, a_size, MPI_FLOAT, A.data, a_size, MPI_FLOAT, root, comm);

    int rank_i, rank_j;
    cart_rank(rank, sqrt_total_procs, &rank_i, &rank_j);

    // ##################################################
    // Step 1: Distribuite x
    // This is done in 'waves'. First root
    // scatters it to its row, then each process
    // in that row sends it to the process 'below'
    // itself.
    // ##################################################

    double start_time = MPI_Wtime();

    MPI_Comm row_comm;
    MPI_Status status;
    MPI_Comm_split(comm, rank_i, rank_j, &row_comm);

    Matrix x = matrix_malloc(v_size, 1);
    int root_row = root / sqrt_total_procs;

    if (rank_i == root_row) {
        MPI_Scatter(rank == root ? vector->data : NULL, v_size, MPI_FLOAT, x.data, v_size, MPI_FLOAT, root, row_comm);
    } else {
        int sender = cart_rank_to_rank(rank_i - 1, rank_j, sqrt_total_procs);
        MPI_Recv(x.data, v_size, MPI_FLOAT, sender, MPI_ANY_TAG, comm, &status);
    }

    int final_row = (root_row - 1) % sqrt_total_procs;
    if (final_row < 0) final_row += sqrt_total_procs;

    if (rank_i != final_row) {
        int reciever = cart_rank_to_rank(rank_i + 1, rank_j, sqrt_total_procs);
        MPI_Send(x.data, v_size, MPI_FLOAT, reciever, 0, comm);
    }

    // ##################################################
    // Step 2: Aij * xi
    // ##################################################

    Matrix b = matrix_malloc(v_size, 1);
    matrix_multiply(&b, A, x);

    // ##################################################
    // Step 3: Reduce
    // ##################################################

    // Re-using x since we don't need its data anymore
    MPI_Reduce(b.data, x.data, v_size, MPI_FLOAT, MPI_SUM, 0, row_comm);

    // ##################################################
    // Step 4: Gather to root & time
    // ##################################################

    double end_time = MPI_Wtime();
    double *end_times;
    int i;

    free(A.data);
    free(b.data);

    if (rank == root) {
        end_times = (double*)malloc(total_procs*sizeof(double));
    }

    MPI_Gather(&end_time, 1, MPI_DOUBLE, end_times, 1, MPI_DOUBLE, root, comm);

    if (rank == root) {
        for (i = 1; i < total_procs; i++) {
            if (end_time < end_times[i])
                end_time = end_times[i];
        }
        printf("Multiplication took %f seconds.\n", end_time-start_time);
        free(end_times);
    }

    MPI_Comm column_comm;
    MPI_Comm_split(comm, rank_j, rank_i, &column_comm);

    if (rank_j == 0)
        MPI_Gather(x.data, v_size, MPI_FLOAT, rank == root ? result->data : NULL, v_size, MPI_FLOAT, root, column_comm);

    free(x.data);
}

int cart_rank_to_rank(int x, int y, int sqrt_total_procs) {
    return x * sqrt_total_procs + y;
}

void cart_rank(int rank, int sqrt_total_procs, int *i, int *j) {
    *i = rank / sqrt_total_procs;
    *j = rank % sqrt_total_procs;
}
