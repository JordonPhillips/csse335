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

  if (rank == 0) {
    master(total_procs);
  } else {
    slave(rank, total_procs);
  }

  MPI_Finalize();
  return 0;
}

void master(int total_procs) {
    int total_ints = NUM_INTS*(total_procs-1);
    int *nums = (int*)malloc(total_ints*sizeof(int));

    MPI_Gather(NULL, 0, NULL, nums, NUM_INTS, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Master recieved: [\n");
    int i;
    for (i = 0; i < total_ints; i++) {
        printf("\t%d,\n",nums[i]);
    }
    printf("]");
    fflush(stdout);

    free(nums);
}

void slave(int rank, int total_procs) {
    int *nums = (int*)malloc(NUM_INTS*sizeof(int));
    int i;
    for (i = 0; i < NUM_INTS; i++) {
        nums[i] = (NUM_INTS*(rank-1))+i+1;
    }
    MPI_Gather(nums,NUM_INTS,MPI_INT,NULL,0,NULL,0,MPI_COMM_WORLD);
    free(nums);
}
