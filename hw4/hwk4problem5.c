#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void master(int total_procs);
void slave(int rank, int total_procs);
float rand_lim(int limit);

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
    float a,b,c,d,k;
    srand(time(NULL));
    a = rand_lim(10);
    b = rand_lim(10);
    c = rand_lim(10);
    d = rand_lim(10);
    k = rand_lim(20);
}

float rand_lim(int limit) {
  return (float)limit*rand()/RAND_MAX;
}

void slave(int rank, int total_procs) {
}
