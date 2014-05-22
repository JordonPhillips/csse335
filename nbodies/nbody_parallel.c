#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "nbody_parallel.h"
#include "nbody_utils.h"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int timestep_size = (int)atof(argv[3]);
  int number_of_timesteps = (int)atof(argv[4]);

  if (rank == 0) {
    master(argv[1], argv[2], timestep_size, number_of_timesteps);
  } else {
    slave(timestep_size, number_of_timesteps);
  }

  MPI_Finalize();
  return 0;
}

void master(char *input_filename, char *output_filename, int timestep_size,
            int number_of_timesteps) {
  double **particles;
  int n, total_procs;

  MPI_Comm_size(MPI_COMM_WORLD, &total_procs);
  read_particles(input_filename, &particles, &n);

  if (n % total_procs != 0) {
    printf("Number of particles is not evenly divisible by number of processors.");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  nbody_parallel(&particles, n, number_of_timesteps, timestep_size, G, 0, MPI_COMM_WORLD);
  write_particles(output_filename, particles, n);

  free_2d_double(particles);
}

void slave(int timestep_size, int number_of_timesteps) {
  int n;
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double **particles = alloc_2d_double(n, DIMENSIONS*2+1);
  nbody_parallel(&particles, n, timestep_size, number_of_timesteps, G, 0, MPI_COMM_WORLD);
  free_2d_double(particles);
}

void nbody_parallel(double ***particles, int n, int timesteps,
                    int timestep_size, double g, int root, MPI_Comm comm) {
  int rank, total_procs;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &total_procs);

  int doubles_per_particle = DIMENSIONS*2+1;
  MPI_Bcast(particles, n*doubles_per_particle, MPI_DOUBLE, 0, comm);

  int particles_per_proc = n / total_procs;
  int first_particle = rank * total_procs;

  double **forces = alloc_2d_double(doubles_per_particle, DIMENSIONS);

  int i, j;
  for (i = timestep_size; i < timesteps; i += timestep_size) {
    // Update position
    for (j = 0; j < particles_per_proc; j++) {
      (*particles)[j+first_particle][X]  += timestep_size * (*particles)[j][DX];
      (*particles)[j+first_particle][Y]  += timestep_size * (*particles)[j][DY];
    }
    get_forces(forces, *particles, n, first_particle, particles_per_proc, g);
    // Update velocities
    for (j = 0; j < particles_per_proc; j++) {
      (*particles)[j+first_particle][X]  += timestep_size * forces[j][0];
      (*particles)[j+first_particle][Y]  += timestep_size * forces[j][1];
    }
  }
}

void get_forces(double **forces, double **particles, int num_particles,
  int initial_target_particle, int num_target_particles, double g) {

  int i, j, target;
  for (i = 0; i < num_target_particles; i++) {
    for (j = 0; j < DIMENSIONS; j++) {
      forces[i][j] = 0.0;
    }
  }

  double dx, dy, distance, distance_cubed, force;

  for (i = 0; i < num_particles; i++) {
    target = initial_target_particle + i;
    for (j = 0; j < num_particles; j++) {
      if (j == target) continue;

      dx = particles[target][X] - particles[j][X];
      dy = particles[target][Y] - particles[j][Y];
      distance = sqrt(pow(dx,2)+pow(dy,2));
      distance_cubed = pow(distance,3);

      force = g*particles[target][MASS]*particles[j][MASS]/distance_cubed;

      forces[i][0] -= force * dx;
      forces[i][1] -= force * dy;
    }
  }
}