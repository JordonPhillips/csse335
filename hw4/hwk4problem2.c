#include <mpi.h>
#include <stdio.h>

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
    int n;
    printf("Input: ");
    fflush(stdout);
    scanf("%d",&n);

    if (total_procs > 1) {
        MPI_Send(&n,1,MPI_INT,1,0,MPI_COMM_WORLD);
    }
}

void slave(int rank, int total_procs) {
    int n;
    MPI_Status status;
    MPI_Recv(&n,1,MPI_INT,rank-1,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

    printf("Rank: %d Sender: %d Data %d", rank, rank-1, n);
    fflush(stdout);

    if (total_procs - rank > 1) {
        MPI_Send(&n,1,MPI_INT,rank+1,0,MPI_COMM_WORLD);
    }
}
