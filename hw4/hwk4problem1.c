#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (rank == 0) {
    int i;
    int* n;
    while(1) {
      printf("Input: ");
      scanf("%d", n);
      if (*n < 0) break;

      for (i = 1; i < total_procs; i++) {
        MPI_Send(n,1,MPI_INT,i,0,MPI_COMM_WORLD);
      }
    }
  } else {
    MPI_Status status;
    int* recv;
    MPI_Recv(recv,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    printf("Rank: %d Received: %d",rank,*recv);
  }

  MPI_Finalize();
}
