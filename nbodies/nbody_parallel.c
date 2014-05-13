#include "nbodies.h"
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    master(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
  } else {
    slave();
  }

  MPI_Finalize();
  return 0;
}

void master(char *input_filename, char *output_filename, int timestep_size, int number_of_timesteps) {
  
}

void slave() {}
