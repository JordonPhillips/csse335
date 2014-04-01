#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (rank == 0) {
    int i;
    int n = 0;
    while(n >= 0) {
      printf("Input: ");
      fflush(stdout);
      scanf("%d", &n);

      for (i = 1; i < total_procs; i++) {
        MPI_Send(&n,1,MPI_INT,i,0,MPI_COMM_WORLD);
      }
    }

  } else {
    MPI_Status status;
    int recv = 0;
    while(recv >= 0) {
      MPI_Recv(&recv,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
      printf("Rank: %d Received: %d\n",rank,recv);
      fflush(stdout);
    }
  }

  MPI_Finalize();
}
