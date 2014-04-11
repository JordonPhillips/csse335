#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "MY_MPI.h"

#define NUM_INTS 4

void master(int total_procs);
void slave(int rank, int total_procs);

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int *send = (int*)malloc(NUM_INTS*sizeof(int));
  int *recv = (int*)malloc(NUM_INTS*total_procs*sizeof(int));

  int i;
  for (i = 0; i < NUM_INTS; i++) {
    send[i] = (rank*NUM_INTS)+i;
  }

  MY_AllToAllBroadcast(send,NUM_INTS,MPI_INT,recv,NUM_INTS,MPI_INT,MPI_COMM_WORLD);

  if (rank == 0) {
    printf("Final Data: [%d", recv[0]);
    for (i = 1; i < total_procs*NUM_INTS; i++) {
        printf(",%d",recv[i]);
    }
    printf("]\n");
  }

  MPI_Finalize();
  return 0;
}
