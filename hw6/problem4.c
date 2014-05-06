#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARS_PER_FLOAT 16
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

typedef struct {
    int height;
    int width;
    float *data;
    int is_inverted;
} Matrix;

void   master(char *a_fname, char *b_fname, char *out_fname);
void   slave();

Matrix matrix_read(char *fname);
Matrix matrix_malloc(int height, int width);
void   matrix_print(Matrix matrix);
void   matrix_write(char *fname, Matrix matrix);

float  matrix_get(Matrix matrix, int row, int col);
int    matrix_get_index(Matrix matrix, int row, int col);
void   matrix_get_submatrix(Matrix *submatrix, Matrix matrix, int start_row,
                            int start_col);

void   matrix_set(Matrix *matrix, int row, int col, float val);
void   matrix_invert(Matrix *matrix);
void   matrix_chunk(Matrix *matrix, int sqrt_total_procs);

void   matrix_multiply(Matrix *result, Matrix a, Matrix b);
void   MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int root,
                           int n, MPI_Comm comm);
void   shift_data(int dir, int *cart_coords, int *neighbors, Matrix *send,
                  Matrix *recv, int steps, MPI_Comm comm);
int    get_opposite_direction(int dir);

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

void MPI_matrix_multiply(Matrix *result, Matrix *a, Matrix *b, int n, int root,
                         MPI_Comm comm) {
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
    int size = pow(width, 2);

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

    Matrix C = {
        width,
        width,
        rank == root ? result->data : (float *)malloc(pow(n, 2)*sizeof(float)),
        0
    };

    // ##################################################
    // Step 0.5: Set up Caretesian communicator
    // ##################################################
    int dim_sizes[2] = {sqrt_total_procs, sqrt_total_procs};
    int wrap_around[2] = {1, 1};
    int neighbors[4];

    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, dim_sizes, wrap_around, 0, &cart_comm);

    int cart_rank;
    int cart_coords[2];

    MPI_Comm_rank(cart_comm, &cart_rank);
    MPI_Cart_coords(cart_comm, cart_rank, 2, cart_coords);

    int dummy;
    MPI_Cart_shift(cart_comm, 0, 1, &dummy, &neighbors[UP]);
    MPI_Cart_shift(cart_comm, 0, -1, &dummy, &neighbors[DOWN]);
    MPI_Cart_shift(cart_comm, 1, 1, &dummy, &neighbors[LEFT]);
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
    // Step 3: Profit
    // ##################################################
    for (k = 0; k < sqrt_total_procs; k++) {
        for (i = 0; i < sqrt_total_procs; i++) {
            for (j = 0; j < sqrt_total_procs; j++) {
                
            }
        }
    }

    if (rank != root)
        free(C.data);

    free(shift_buff.data);
    free(B.data);
    free(A.data);
}

void shift_data(int dir, int *cart_coords, int *neighbors, Matrix *send,
                Matrix *recv, int steps, MPI_Comm comm) {
    if (steps < 1) return;

    int i;
    int send_count = send->width * send->height;
    int recv_count = recv->width * recv->height;
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

Matrix matrix_malloc(int height, int width) {
    return (Matrix) {
        height,
        width,
        (float *)malloc(width * height * sizeof(float)),
        0
    };
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
