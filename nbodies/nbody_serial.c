#include "nbody_serial.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(int argc, char **argv) {
	if (argc != 5) {
		printf("Invalid number of arguments. Expecting 4, found %d.\n", argc-1);
		return -1;
	}
	double **particles;
	int n, i;
	read_particles(argv[1], &particles, &n);
	nbody_serial(&particles, n, (int)atof(argv[3]), (int)atof(argv[4]), G);
	write_particles(argv[2], particles, n);

	for (i = 0; i < n; i++) {
		free(particles[i]);
	}
	free(particles);
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

void read_particles(char *filename, double ***particles, int *n) {
	FILE *fp = fopen(filename, "r");

	if (fp == NULL) perror(filename);

	int doubles_per_particle = DIMENSIONS * 2 + 1;
	int line_size = CHARS_PER_DOUBLE * doubles_per_particle * sizeof(char);
	char *line = (char*)malloc(line_size);
	
	fscanf(fp, "%d", n);

	if (*n < 1)
		perror("Too few particles specified.");

	double *particle;
	*particles = (double**)malloc(*n*sizeof(double*));

	char *pch;
	int i, j;

	// Ignoring the first line, which was already read.
	fgets(line, line_size, fp);

	for (i = 0; i < *n && fgets(line, line_size, fp) != NULL; i++) {
		pch = strtok(line, ", ");
		particle = (double*)malloc(doubles_per_particle*sizeof(double));

		for (j = 0; j < doubles_per_particle && pch != NULL; j++) {
			particle[j] = atof(pch);
			pch = strtok(NULL, ", ");
		}

		(*particles)[i] = particle;
	}

	fclose(fp);
}

void write_particles(char *filename, double **particles, int n) {
	FILE *fp = fopen(filename, "w");

	fprintf(fp, "%d\n", n);

	int i, j;
	int doubles_per_particle = DIMENSIONS * 2;
	for (i = 0; i < n; i++) {
		for (j = 0; j < doubles_per_particle; j++) {
			fprintf(fp, "%e, ", particles[i][j]);
		}
		fprintf(fp, "%e\n", particles[i][doubles_per_particle]);
	}

	fclose(fp);
}
