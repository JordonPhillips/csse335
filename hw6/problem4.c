#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.c"

#define CHARS_PER_FLOAT 16
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

void master(char *a_fname, char *b_fname, char *out_fname);
void slave();
void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int root, int n, MPI_Comm comm);
void shift_data(int dir, int *cart_coords, int *neighbors, Matrix *send, Matrix *recv, int steps,
                MPI_Comm comm);
int get_opposite_direction(int dir);

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
    // matrix_write(out_fname, result);

    // if (result.width <= 20)
    //     matrix_print(result);

    free(result.data);
    free(a.data);
    free(b.data);
}

void slave() {
    int n;


    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_matrix_multiply(NULL, NULL, NULL, n, 0, MPI_COMM_WORLD);
}

void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int n, int root, MPI_Comm comm) {
    int rank, total_procs;


    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    if (n % total_procs != 0) {
        if (rank == root)
            printf("Matricies are not evenly divisible by number of processes.\n");

        return;
    }

    int sqrt_total_procs = sqrt(total_procs);

    if (pow(sqrt_total_procs, 2) != total_procs) {
        if (rank == root)
            printf("Number of processes must be a perfect square.\n");

        return;
    }

    int width = n / sqrt_total_procs;
    int size  = pow(width, 2);

    // ##################################################
    // Step 0: Chunk and Distribute A and B
    // ##################################################

    if (rank == root) {
        matrix_chunk(a, sqrt_total_procs);
        matrix_chunk(b, sqrt_total_procs);
    }

    Matrix A = matrix_malloc(width, width);
    MPI_Scatter(rank == root ? a->data : NULL, size, MPI_FLOAT, A.data, size,
                MPI_FLOAT, root, comm);

    Matrix B = matrix_malloc(width, width);
    MPI_Scatter(rank == root ? b->data : NULL, size, MPI_FLOAT, B.data, size,
                MPI_FLOAT, root, comm);

    Matrix shift_buff = matrix_malloc(width, width);

    Matrix C = matrix_malloc(width, width);
    matrix_init(&C);

    // ##################################################
    // Step 0.5: Set up Caretesian communicator
    // ##################################################
    int dim_sizes[2]   = {sqrt_total_procs, sqrt_total_procs};
    int wrap_around[2] = {1, 1};
    int neighbors[4];

    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, dim_sizes, wrap_around, 0, &cart_comm);

    int cart_rank;
    int cart_coords[2];

    MPI_Comm_rank(cart_comm, &cart_rank);
    MPI_Cart_coords(cart_comm, cart_rank, 2, cart_coords);

    int dummy;
    MPI_Cart_shift(cart_comm, 0, 1,  &dummy, &neighbors[UP]);
    MPI_Cart_shift(cart_comm, 0, -1, &dummy, &neighbors[DOWN]);
    MPI_Cart_shift(cart_comm, 1, 1,  &dummy, &neighbors[LEFT]);
    MPI_Cart_shift(cart_comm, 1, -1, &dummy, &neighbors[RIGHT]);

    // ##################################################
    // Step 1: Skew A
    // ##################################################
    int i, j, k;

    for (i = 1; i < sqrt_total_procs; i++)
        if (cart_coords[0] == i)
            shift_data(LEFT, cart_coords, neighbors, &A, &shift_buff, i, cart_comm);

    // ##################################################
    // Step 2: Skew B
    // ##################################################
    for (i = 1; i < sqrt_total_procs; i++)
        if (cart_coords[1] == i)
            shift_data(UP, cart_coords, neighbors, &B, &shift_buff, i, cart_comm);

    // ##################################################
    // Step 3: Perform the multiplication
    // ##################################################
    for (j = 0; j < sqrt_total_procs; j++) {
        matrix_multiply(&shift_buff, A, B);
        matrix_add(&C, shift_buff);
        shift_data(LEFT, cart_coords, neighbors, &A, &shift_buff, 1, cart_comm);
        shift_data(UP,   cart_coords, neighbors, &B, &shift_buff, 1, cart_comm);
    }

    // ##################################################
    // Step 4: Gather the results
    // ##################################################
    free(shift_buff.data);
    free(B.data);
    free(A.data);

    MPI_Gather(C.data, size, MPI_FLOAT,
               rank == root ? result->data : NULL, size, MPI_FLOAT,
               root, comm
               );

    free(C.data);

    if (rank == root) {
        matrix_chunk(result, sqrt_total_procs);
        matrix_print(*result);
    }
}

void shift_data(int dir, int *cart_coords, int *neighbors, Matrix *send, Matrix *recv, int steps,
                MPI_Comm comm) {
    if (steps < 1) return;

    int        i;
    int        send_count = send->width * send->height;
    int        recv_count = recv->width * recv->height;
    MPI_Status status;

    int send_first;

    if (dir == UP || dir == DOWN)
        send_first = cart_coords[0] % 2;
    else
        send_first = cart_coords[1] % 2;

    int recv_dir = get_opposite_direction(dir);

    if (send_first == 0)
        MPI_Recv(recv->data, recv_count, MPI_FLOAT, neighbors[recv_dir], 0, comm,
                 &status);

    MPI_Send(send->data, send_count, MPI_FLOAT, neighbors[dir], 0, comm);

    for (i = 1; i < steps; i++) {
        MPI_Recv(recv->data, recv_count, MPI_FLOAT, neighbors[recv_dir], 0, comm,
                 &status);
        MPI_Send(recv->data, send_count, MPI_FLOAT, neighbors[dir], 0, comm);
    }

    if (send_first == 1)
        MPI_Recv(recv->data, recv_count, MPI_FLOAT, neighbors[recv_dir], 0, comm,
                 &status);

    float *temp = recv->data;
    recv->data = send->data;
    send->data = temp;
}

int get_opposite_direction(int dir) {
    switch (dir) {
    case UP:
        return DOWN;

    case DOWN:
        return UP;

    case LEFT:
        return RIGHT;

    case RIGHT:
        return LEFT;
    }

    return -1;
}
