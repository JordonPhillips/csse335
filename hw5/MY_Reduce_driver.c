#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "MY_MPI.h"

#define NUM_INTS 4

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int *send = (int*)malloc(NUM_INTS*sizeof(int));
  int *recv = (int*)malloc(NUM_INTS*sizeof(int));

  int i;
  for (i = 0; i < NUM_INTS; i++) {
    send[i] = NUM_INTS*rank+i;
    recv[i] = 0;
  }

  MY_Reduce(send, recv, NUM_INTS, MPI_INT, 1, 0, MPI_COMM_WORLD);

  if (rank == 0) {
      printf("Final Results: [%d",recv[0]);
      for (i = 1; i < NUM_INTS; i++) {
        printf(",%d",recv[i]);
      }
      printf("]\n");
      fflush(stdout);
  }

  free(send);
  free(recv);

  MPI_Finalize();
  return 0;
}
