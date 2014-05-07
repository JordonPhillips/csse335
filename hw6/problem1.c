#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARS_PER_FLOAT 16

void  master(char *matrix_fname, char *vector_fname, char *out_fname);
void  slave();
float **read_matrix(char *fname, int *m, int *n);
void  print_matrix(float **matrix, int m, int n);
void  write_matrix(char *fname, float *matrix, int m, int n);
float **malloc_matrix(int rows, int cols);
void  free_matrix(float **matrix, int m);
void  MPI_matrix_vector_multiply(float *A, float *x, float *result, int n, int root, MPI_Comm comm);
void  invert_row(float *buff, float **matrix, int m, int n);
void  invert_matrix(float **buff, float **matrix, int m, int n);
void  flatten_matrix(float *buff, float **matrix, int m, int n);

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
    int   m_rows, m_cols, v_rows, v_cols;
    float **matrix = read_matrix(matrix_fname, &m_rows, &m_cols);
    float **vector = read_matrix(vector_fname, &v_rows, &v_cols);

    // Invert them so it's easier to distribute them
    float **inverted_matrix = malloc_matrix(m_cols, m_rows);
    float *inverted_vector  = malloc( v_rows * sizeof(float) );


    invert_matrix(inverted_matrix, matrix, m_rows, m_cols);
    invert_row(inverted_vector, vector, v_rows, 0);

    free_matrix(vector, v_rows);
    free_matrix(matrix, m_rows);

    // Flatten the matrix to facilitate distribution
    float *flattened_matrix = malloc( m_cols * m_rows * sizeof(float) );
    flatten_matrix(flattened_matrix, inverted_matrix, m_cols, m_rows);

    free_matrix(inverted_matrix, m_cols);

    float *result = malloc( m_rows * sizeof(float) );

    if (m_rows == m_cols && v_rows == m_rows && v_cols == 1) {
        MPI_Bcast(&m_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

        double start = MPI_Wtime();
        MPI_matrix_vector_multiply(flattened_matrix, inverted_vector, result, m_rows, 0,
                                   MPI_COMM_WORLD);
        printf("Multiplication took %f seconds.\n", MPI_Wtime() - start);

        write_matrix(out_fname, result, m_rows, v_cols);
    } else {
        printf("Invalid inputs. Matrix must be nxn (was %dx%d) and vector must"
               " be nx1 (was %dx%d)", m_rows, m_cols, v_rows, v_cols);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    free(flattened_matrix);
    free(inverted_vector);
    free(result);
}

void slave() {
    int n;


    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_matrix_vector_multiply(NULL, NULL, NULL, n, 0, MPI_COMM_WORLD);
}

void MPI_matrix_vector_multiply(float *send_matrix, float *vector, float *result, int n, int root,
                                MPI_Comm comm) {
    int rank, total_procs;


    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int num_vals = n % total_procs == 0 ? n / total_procs : -1;

    if (num_vals < 1) {
        printf("Number of values is not evenly divisible by number of procedures\n");
        return;
    }

    float *x = (float *)malloc( num_vals * sizeof(float) );
    MPI_Scatter(vector, num_vals, MPI_FLOAT, x, num_vals, MPI_FLOAT,
                root, comm);

    float *A = (float *)malloc( num_vals * n * sizeof(float) );
    MPI_Scatter(send_matrix, num_vals * n, MPI_FLOAT, A, num_vals * n, MPI_FLOAT,
                root, comm);

    float *temp = (float *)malloc( n * sizeof(float) );
    int   i, j;

    for (i = 0; i < n; i++) {
        temp[i] = 0;

        for (j = 0; j < num_vals; j++)
            temp[i] += x[j] * A[j * n + i];
    }

    MPI_Reduce(temp, result, n, MPI_FLOAT, MPI_SUM, root, comm);

    free(x);
    free(A);
    free(temp);
}

float **read_matrix(char *fname, int *m, int *n) {
    FILE *fp;


    fp = fopen(fname, "r");

    if (fp != NULL) {
        fscanf(fp, "%d %d", m, n);

        if ( *m < 1 || *n < 1 || (*m == 1 && *n == 1) ) {
            printf("Invalid array size: %d x %d", *m, *n);
            return NULL;
        }

        char line[CHARS_PER_FLOAT * (*n)];
        char *pch;

        int   i, j;
        float **A = malloc_matrix(*m, *n);

        fgets(line, sizeof(line),
              fp); // Ignoring the first line, which was already read

        for (i = 0; i < *m && fgets(line, sizeof(line), fp) != NULL; i++) {
            pch = strtok(line, " ");

            for (j = 0; j < *n && pch != NULL; j++) {
                A[i][j] = atof(pch);
                pch     = strtok(NULL, " ");
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
    float **matrix = malloc( rows * sizeof(float *) );
    int   i;


    for (i = 0; i < rows; i++)
        matrix[i] = malloc( cols * sizeof(float) );

    return matrix;
}

void free_matrix(float **matrix, int m) {
    int i;


    for (i = 0; i < m; i++)
        free(matrix[i]);

        free(matrix);
}

void print_matrix(float **matrix, int m, int n) {
    int i, j;


            printf("%d %d\n", m, n);

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
            printf("%f ", matrix[i][j]);

            printf("\n");
    }
}

void write_matrix(char *fname, float *matrix, int m, int n) {
    FILE *fp = fopen(fname, "w");


            fprintf(fp, "%d %d\n", m, n);

    int i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
            fprintf(fp, "%f ", matrix[j * n + i]);

            fprintf(fp, "\n");
    }

    fclose(fp);
}

void invert_row(float *buff, float **matrix, int m, int n) {
    int i;


    for (i = 0; i < m; i++)
        buff[i] = matrix[i][n];
}

void invert_matrix(float **buff, float **matrix, int m, int n) {
    int i;


    for (i = 0; i < n; i++)
        invert_row(buff[i], matrix, m, i);
}

void flatten_matrix(float *buff, float **matrix, int m, int n) {
    int   i, j;
    float *ptr = buff;


    for (i = 0; i < m; i++) {
        memcpy( ptr, matrix[i], n * sizeof(float) );
        ptr += n;
    }
}
