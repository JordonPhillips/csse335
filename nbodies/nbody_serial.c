#include "nbody_serial.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	if (argc != 4) {
		printf("Invalid number of arguments. Expecting 4, found %d.\n", argc);
		return -1;
	}
	double **particles;
	int n, i;
	read_particles(argv[1], &particles, &n);
	nbody_serial(&particles, n);
	write_particles(argv[2], particles, n);

	for (i = 0; i < n; i++) {
		free(particles[i]);
	}
	free(particles);
  return 0;
}

void nbody_serial(double ***particles, int n) {
	
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
	int doubles_per_particle = DIMENSIONS * 2 + 1;
	for (i = 0; i < n; i++) {
		for (j = 0; j < doubles_per_particle; j++) {
			fprintf(fp, "%f, ", particles[i][j]);
		}
		fprintf(fp, "%f\n", particles[i][doubles_per_particle - 1]);
	}

	fclose(fp);
}
