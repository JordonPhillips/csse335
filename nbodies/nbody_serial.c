#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nbody_serial.h"
#include "nbody_utils.h"

int main(int argc, char **argv) {
	if (argc != 5) {
		printf("Invalid number of arguments. Expecting 4, found %d.\n", argc-1);
		return -1;
	}
	double **particles;
	int n;
	read_particles(argv[1], &particles, &n);
	nbody_serial(&particles, n, (int)atof(argv[3]), (int)atof(argv[4]), G);
	write_particles(argv[2], particles, n);

	free_2d_double(particles);
  return 0;
}

void nbody_serial(double ***particles, int n, int timesteps, int timestep_size, double g) {
	int i,j;
	double *forces = (double*)malloc(DIMENSIONS*n*sizeof(double));
	for (i = timestep_size; i < timesteps; i+=timestep_size) {
		for (j = 0; j < n; j++) {
			(*particles)[j][X]  += timestep_size * (*particles)[j][DX];
			(*particles)[j][Y]  += timestep_size * (*particles)[j][DY];
		}
		get_forces(forces, *particles, n, g);
		for (j = 0; j < n; j++) {
			(*particles)[j][DX] += timestep_size * forces[DIMENSIONS*j] / (*particles)[j][MASS];
			(*particles)[j][DY] += timestep_size * forces[DIMENSIONS*j+1] / (*particles)[j][MASS];
		}
	}
	free(forces);
}

void get_forces(double *forces, double **particles, int n, double g) {
	int i, j;
	double dx, dy, distance, distance_cubed, force, fx, fy;

	for (i = 0; i < n*DIMENSIONS; i++) {
		forces[i] = 0.0;
	}

	for (i = 0; i < n; i++) {
		for (j = n - 1; j > i; j--) {
			dx = particles[i][X] - particles[j][X];
			dy = particles[i][Y] - particles[j][Y];
			distance = sqrt(pow(dx,2)+pow(dy,2));
			distance_cubed = pow(distance,3);

			force = g*particles[i][MASS]*particles[j][MASS]/distance_cubed;
			fx = force * dx;
			fy = force * dy;

			forces[DIMENSIONS*i]   += fx;
			forces[DIMENSIONS*i+1] += fy;
			forces[DIMENSIONS*j]   -= fx;
			forces[DIMENSIONS*j+1] -= fy;
		}
	}
}
