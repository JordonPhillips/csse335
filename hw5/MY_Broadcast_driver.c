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
    int *buff = (int*)malloc(NUM_INTS*sizeof(int));

    int i,j;
    for (i = 0; i < NUM_INTS; i++) {
        buff[i] = i+1;
        printf("Sending %d\n", buff[i]);
    }
    fflush(stdout);

    MY_Broadcast(buff,NUM_INTS,MPI_INT,0,MPI_COMM_WORLD);
    free(buff);

    MPI_Status status;
    for (i = 1; i < total_procs; i++) {
        MPI_Recv(&j, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
}

void slave(int rank, int total_procs) {
    int *buff = (int*)malloc(NUM_INTS*sizeof(int));

    MY_Broadcast(buff,NUM_INTS,MPI_INT,0,MPI_COMM_WORLD);

    int i;
    for (i = 0; i < NUM_INTS; i++) {
        printf("Rank %d recieved %d\n",rank,buff[i]);
    }
    fflush(stdout);
    free(buff);

    MPI_Send(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}
