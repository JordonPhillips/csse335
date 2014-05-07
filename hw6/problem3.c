#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.c"

void master(char *a_fname, char *b_fname, char *out_fname);
void slave();

void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int root, int n,
                         MPI_Comm comm);

int  cart_rank_to_rank(int x, int y, int sqrt_total_procs);
void cart_rank(int rank, int sqrt_total_procs, int *i, int *j);

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

void master(char *a_fname, char *b_fname, char *out_fname) {
    Matrix a = matrix_read(a_fname);
    Matrix b = matrix_read(b_fname);

    if (a.width != a.height || b.width != b.height || a.width != b.width) {
        printf("Invalid inputs. Both matricies must be nxn. A was %dx%d. B was"
               " %dx%d", a.height, a.width, b.height, b.width);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    MPI_Bcast(&(a.width), 1, MPI_INT, 0, MPI_COMM_WORLD);

    Matrix result = matrix_malloc(a.width, b.width);
    MPI_matrix_multiply(&result, &a, &b, a.width, 0, MPI_COMM_WORLD);
    matrix_write(out_fname, result);

    if (result.width <= 20)
        matrix_print(result);

    free(result.data);
    free(a.data);
    free(b.data);
}

void slave() {
    int n;

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_matrix_multiply(NULL, NULL, NULL, n, 0, MPI_COMM_WORLD);
}

void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int n, int root,
                         MPI_Comm comm) {
    int rank, total_procs;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int rows_per_proc    = n / total_procs;
    int total_procs_used = total_procs;

    MPI_Comm gather_comm = comm;
    int      gather_root = root;

    if (rows_per_proc == 0) {
        if (rank == root)
            printf("Warning: multilpying two %dx%d matricies takes %d or fewer"
                   " processes. %d were given.\n", n, n, n, total_procs);

        int color = rank + root - 1;
        int key   = color % total_procs + 1;
        color /= total_procs;
        MPI_Comm_split(comm, color, key, &gather_comm);

        if (rank == root)
            MPI_Comm_rank(gather_comm, &gather_root);

        MPI_Bcast(&gather_root, 1, MPI_INT, root, comm);

        if (color > 0) return;

        rows_per_proc    = 1;
        total_procs_used = n;
    }

    if (n > total_procs && n % total_procs != 0) {
        if (rank == root)
            printf("Number of rows is not evenly divisible by number of processes.\n");

        return;
    }

    int size = rows_per_proc * n;

    // ##################################################
    // Step 0: Distribute A and invert b
    // ##################################################

    Matrix A = matrix_malloc(rows_per_proc, n);
    MPI_Scatter(rank == root ? a->data : NULL, size, MPI_FLOAT, A.data, size,
                MPI_FLOAT, root, comm);

    if (rank == root && b->is_inverted != 1) matrix_invert(b);

    int i;
    int sender = (rank - 1) % total_procs;

    if (sender < 0) sender += total_procs;

    int reciever       = (rank + 1) % total_procs;
    int last_proc_rank = (root + total_procs_used - 1) % total_procs;

    Matrix B = matrix_malloc(n, rows_per_proc);
    B.is_inverted = 1;
    Matrix C = matrix_malloc(rows_per_proc, n);
    C.is_inverted = 1;

    float *c_data = C.data;

    MPI_Status status;

    double start_time = MPI_Wtime();

    for (i = 0; i < total_procs_used; i++) {
        // ##################################################
        // Step 1: Recieve Data
        // ##################################################
        if (rank == root)
            matrix_get_submatrix(&B, *b, 0, i * rows_per_proc);
        else
            MPI_Recv(B.data, rows_per_proc * n, MPI_FLOAT, sender, MPI_ANY_TAG, comm,
                     &status);

        // ##################################################
        // Step 2: Calculate Dot Product
        // ##################################################
        matrix_multiply(&C, A, B);
        C.data += rows_per_proc;

        // ##################################################
        // Step 3: Send Data
        // ##################################################
        if (rank != last_proc_rank)
            MPI_Send(B.data, rows_per_proc * n, MPI_FLOAT, reciever, 0, comm);
    }

    C.data = c_data;

    // ##################################################
    // Step 4: Get time and Gather on root
    // ##################################################

    double end_time = MPI_Wtime();

    if (rank == last_proc_rank)
        MPI_Send(&end_time, 1, MPI_DOUBLE, 0, 0, comm);

    if (rank == root) {
        MPI_Recv(&end_time, 1, MPI_DOUBLE, last_proc_rank, MPI_ANY_TAG, comm, &status);
        printf("Multiplication took %f seconds.\n", end_time - start_time);
    }

    int send_count = C.width * C.height;
    MPI_Gather(C.data, send_count, MPI_FLOAT,
               rank == root ? result->data : NULL, send_count, MPI_FLOAT,
               gather_root, gather_comm);

    free(C.data);
    free(B.data);
    free(A.data);
}

int cart_rank_to_rank(int x, int y, int sqrt_total_procs) {
    return x * sqrt_total_procs + y;
}

void cart_rank(int rank, int sqrt_total_procs, int *i, int *j) {
    *i = rank / sqrt_total_procs;
    *j = rank % sqrt_total_procs;
}
