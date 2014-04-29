#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 4028

void master(char* matrix_fname, char* vector_fname, char* out_fname);
void slave();
float **read_matrix(char* fname, int *m, int *n);
void print_matrix(float **matrix, int m, int n);
void free_matrix(float **matrix, int m);

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
    int m, n, l, w;
    float **matrix = read_matrix(matrix_fname, &m, &n);
    float **vector = read_matrix(vector_fname, &l, &w);

    print_matrix(matrix, m, n);
    print_matrix(vector, l, w);

    free_matrix(vector, l);
    free_matrix(matrix, m);
}

void slave() {

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

        char line[BUFFSIZE];
        char *pch;

        int i, j;
        float **A = malloc((*m)*sizeof(float*));
        for (i = 0; i < *m; i++) {
            A[i] = malloc((*n)*sizeof(float));
        }

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

void free_matrix(float **matrix, int m) {
    int i;
    for (i = 0; i < m; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix(float **matrix, int m, int n) {
    int i, j;
    printf("%d %d\n", m, n);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}
