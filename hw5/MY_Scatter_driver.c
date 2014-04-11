#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "MY_MPI.h"

#define NUM_INTS 4

void master(int total_procs);
void slave(int rank, int total_procs);

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (rank == 0) {
    master(total_procs);
  } else {
    slave(rank, total_procs);
  }

  MPI_Finalize();
  return 0;
}

void master(int total_procs) {
    int *data = (int*)malloc((total_procs-1)*NUM_INTS*sizeof(int));
    int i, j;
    for (i = 0; i < total_procs - 1; i++) {
        for (j = 0; j < NUM_INTS; j++) {
            data[i+j] = i+j+1;
        }
    }

    MY_Scatter(data, NUM_INTS, MPI_INT, NULL, 0, NULL, 0, MPI_COMM_WORLD);
    free(data);

    fscanf(stdin,"%d",&i);
}

void slave(int rank, int total_procs) {
    int *data = (int*)malloc(NUM_INTS*sizeof(int));
    MY_Scatter(NULL, 0, NULL, data, NUM_INTS, MPI_INT, 0, MPI_COMM_WORLD);

    int i;
    for (i = 0; i < NUM_INTS; i++) {
        printf("Rank %d recived integer %d\n", rank, data[i]);
    }
    fflush(stdout);
}
