#include<stdio.h>
#include<mpi.h>

int main(int argc, char** argv){
  MPI_Init(&argc,&argv);
  int total_procs;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  printf("Hello, world, this is process %d  of %d\n",rank,total_procs);
  MPI_Finalize();

}
