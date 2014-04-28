#include <mpi.h>
#include <stdio.h>

void master(char* matrix_fname, char* vector_fname, char* out_fname);
void slave();
float *read_matrix(char* fname);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == 0) {
        if (argc == 3) {
            master(argv[0], argv[1], argv[2]);
        } else {
            printf("Incorrect argument count. Expected 3, recieved %d\n", argc);
        }
    } else {
        if (argc == 3) {
            slave();
        }
    }

    MPI_Finalize();
    return 0;
}

void master(char* matrix_fname, char* vector_fname, char* out_fname) {
    
}

void slave() {

}

float *read_matrix(char *fname) {
    FILE *fp;
    fp = fopen(fname, "r");

    int m, n;
    fscanf(fp, "%d %d", m, n);

    fclose(fp);
}
