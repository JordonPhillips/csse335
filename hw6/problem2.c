#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARS_PER_FLOAT 16

void master(char* matrix_fname, char* vector_fname, char* out_fname);
void slave();
float **read_matrix(char* fname, int *m, int *n);
void print_matrix(float *matrix, int m, int n);
void write_matrix(char *fname, float* matrix, int m, int n);
float **malloc_matrix(int rows, int cols);
void free_matrix(float **matrix, int m);
void MPI_matrix_vector_multiply(float *A, float *x, float *result, int n, int root, MPI_Comm comm);
void invert_row(float *buff, float **matrix, int m, int n);
void invert_matrix(float **buff, float **matrix, int m, int n);
void prepare_matrix(float *buff, float **matrix, int n, int sqrt_total_procs);
int cart_rank_to_rank(int x, int y, int sqrt_total_procs);
void copy_submatrix(float *buff, float **matrix, int x, int y, int size);
void cart_rank(int rank, int sqrt_total_procs, int *i, int *j);
void matrix_vector_multiply(float *buff, float *matrix, float *vector, int n);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == 0) {
        if (argc == 4) {
            master(argv[1], argv[2], argv[3]);
        } else {
            printf("Incorrect argument count. Expected 3, recieved %d\n", argc-1);
        }
    } else {
        if (argc == 4) {
            slave();
        }
    }

    MPI_Finalize();
    return 0;
}

void master(char* matrix_fname, char* vector_fname, char* out_fname) {
    int m_rows, m_cols, v_rows, v_cols;
    float **matrix = read_matrix(matrix_fname, &m_rows, &m_cols);
    float **vector = read_matrix(vector_fname, &v_rows, &v_cols);

    if (m_rows != m_cols || v_rows != m_rows || v_cols != 1) {
        printf("Invalid inputs. Matrix must be nxn (was %dx%d) and vector must"
                " be nx1 (was %dx%d)\n",m_rows,m_cols,v_rows,v_cols);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    int total_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &total_procs);
    int sqrt_total_procs = sqrt(total_procs);

    if (total_procs != sqrt_total_procs*sqrt_total_procs) {
        printf("Number of processes (%d) is not a perfect square.\n", total_procs);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    if (m_rows % sqrt_total_procs != 0) {
        printf("Matrix size is not easily divisible among the number of processes.\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    float *inverted_vector = malloc(v_rows*sizeof(float));
    invert_row(inverted_vector, vector, v_rows, 0);
    free_matrix(vector, v_rows);

    float *prepared_matrix = malloc(m_cols*m_rows*sizeof(float));
    prepare_matrix(prepared_matrix, matrix, m_cols, sqrt_total_procs);
    free_matrix(matrix, m_cols);

    float *result = malloc(m_rows*sizeof(float));

    MPI_Bcast(&m_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double start = MPI_Wtime();
    MPI_matrix_vector_multiply(prepared_matrix, inverted_vector, result, m_rows, 0, MPI_COMM_WORLD);
    printf("Multiplication took %f seconds.\n", MPI_Wtime()-start);

    write_matrix(out_fname, result, m_rows, v_cols);

    free(prepared_matrix);
    free(inverted_vector);
    free(result);
}

void slave() {
    int n;
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_matrix_vector_multiply(NULL, NULL, NULL, n, 0, MPI_COMM_WORLD);
}

void MPI_matrix_vector_multiply(float *send_matrix, float *vector, float *result, int n, int root,  MPI_Comm comm) {
    int rank, total_procs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int sqrt_total_procs = sqrt(total_procs);
    int v_size = n / sqrt_total_procs;
    int a_size = v_size*v_size;

    float *A = (float*)malloc(a_size*sizeof(float));
    MPI_Scatter(send_matrix, a_size, MPI_FLOAT, A, a_size, MPI_FLOAT, root, comm);

    int rank_i, rank_j;
    cart_rank(rank, sqrt_total_procs, &rank_i, &rank_j);

    MPI_Comm row_comm;
    MPI_Comm_split(comm, rank_i, rank_j, &row_comm);

    float *x = (float*)malloc(v_size*sizeof(float));

    int root_row = root / sqrt_total_procs;
    MPI_Status status;
    if (rank_i == root_row) {
    	MPI_Scatter(vector, v_size, MPI_FLOAT, x, v_size, MPI_FLOAT, root, row_comm);
    } else {
    	int sender = cart_rank_to_rank(rank_i - 1, rank_j, sqrt_total_procs);
    	MPI_Recv(x, v_size, MPI_FLOAT, sender, MPI_ANY_TAG, comm, &status);
    }

    int final_row = (root_row - 1) % sqrt_total_procs;
    if (final_row < 0) final_row += sqrt_total_procs;
    if (rank_i != final_row) {
    	int reciever = cart_rank_to_rank(rank_i+1, rank_j, sqrt_total_procs);
    	MPI_Send(x, v_size, MPI_FLOAT, reciever, 0, comm);
    }

    float *b = (float*)malloc(v_size*sizeof(float));
    matrix_vector_multiply(b, A, x, v_size);
    free(A);

    // Re-using x as a temporary variable since I don't need the info anymore
    MPI_Reduce(b, x, v_size, MPI_FLOAT, MPI_SUM, 0, row_comm);

	MPI_Comm column_comm;
	MPI_Comm_split(comm, rank_j, rank_i, &column_comm);
    if (rank_j == 0) {
    	MPI_Gather(x, v_size, MPI_FLOAT, result, v_size, MPI_FLOAT, 0, column_comm);
    }

    free(x);
    free(b);
}

void matrix_vector_multiply(float *buff, float *matrix, float *vector, int n) {
	int i, j;
    for (i = 0; i < n; i++) {
        buff[i] = 0;
        for (j = 0; j < n; j++) {
            buff[i] += vector[j] * matrix[i*n+j];
        }
    }
}

float **read_matrix(char *fname, int *m, int *n) {
    FILE *fp;
    fp = fopen(fname, "r");

    if (fp != NULL) {
        fscanf(fp, "%d %d", m, n);

        if (*m < 1 || *n < 1 || (*m == 1 && *n == 1)) {
            printf("Invalid array size: %d x %d", *m, *n);
            return NULL;
        }

        char line[CHARS_PER_FLOAT*(*n)];
        char *pch;

        int i, j;
        float **A = malloc_matrix(*m, *n);

        fgets(line, sizeof(line),fp); // Ignoring the first line, which was already read
        for (i = 0; i < *m && fgets(line, sizeof(line), fp) != NULL; i++) {
            pch = strtok(line, " ");

            for (j = 0; j < *n && pch != NULL; j++) {
                A[i][j] = atof(pch);
                pch = strtok(NULL, " ");
            }
        }

        fclose(fp);
        return A;
    } else {
        perror(fname);
        return NULL;
    }
}

float **malloc_matrix(int rows, int cols) {
    float **matrix = malloc(rows*sizeof(float*));
    int i;
    for (i = 0; i < rows; i++) {
        matrix[i] = malloc(cols*sizeof(float));
    }
    return matrix;
}

void free_matrix(float **matrix, int m) {
    int i;
    for (i = 0; i < m; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix(float *matrix, int m, int n) {
    int i, j;
    printf("%d %d\n", m, n);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", matrix[i*n+j]);
        }
        printf("\n");
    }
}

void write_matrix(char *fname, float *matrix, int m, int n) {
    FILE *fp = fopen(fname, "w");

    fprintf(fp, "%d %d\n", m, n);

    int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            fprintf(fp, "%f ", matrix[j*n + i]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void invert_row(float *buff, float **matrix, int m, int n) {
    int i;
    for (i = 0; i < m; i++) {
        buff[i] = matrix[i][n];
    }
}

void invert_matrix(float **buff, float **matrix, int m, int n) {
    int i;
    for (i = 0; i < n; i++) {
        invert_row(buff[i], matrix, m, i);
    }
}

void prepare_matrix(float *buff, float **matrix, int n, int sqrt_total_procs) {
    int i, j;
    int size = n / sqrt_total_procs;
    float *ptr = buff;
    for (i = 0; i < sqrt_total_procs; i++) {
        for (j = 0; j < sqrt_total_procs; j++) {
            copy_submatrix(ptr, matrix, i, j, size);
            ptr += size*size;
        }
    }
}

void copy_submatrix(float *buff, float **matrix, int x, int y, int size) {
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            buff[i*size+j] = matrix[x*size+i][y*size+j];
        }
    }
}

int cart_rank_to_rank(int x, int y, int sqrt_total_procs) {
	return x*sqrt_total_procs + y;
}

void cart_rank(int rank, int sqrt_total_procs, int *i, int *j) {
	*i = rank / sqrt_total_procs;
	*j = rank % sqrt_total_procs;
}
