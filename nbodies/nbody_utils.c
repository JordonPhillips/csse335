#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "nbody_utils.h"

void read_particles(char *filename, double ***particles, int *n) {
	FILE *fp = fopen(filename, "r");

	if (fp == NULL) perror(filename);

	int doubles_per_particle = DIMENSIONS * 2 + 1;
	int line_size = CHARS_PER_DOUBLE * doubles_per_particle * sizeof(char);
	char *line = (char*)malloc(line_size);
	
	fscanf(fp, "%d", n);

	if (*n < 1)
		perror("Too few particles specified.");

	*particles = alloc_2d_double(n, doubles_per_particle);
	double particle;

	char *pch;
	int i, j;

	// Ignoring the first line, which was already read.
	fgets(line, line_size, fp);

	for (i = 0; i < *n && fgets(line, line_size, fp) != NULL; i++) {
		pch = strtok(line, ", ");
		particle = (*particles)[i];

		for (j = 0; j < doubles_per_particle && pch != NULL; j++) {
			particle[j] = atof(pch);
			pch = strtok(NULL, ", ");
		}
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

double **alloc_2d_double(int rows, int cols) {
    double *data = (double *)malloc(rows*cols*sizeof(double));
    double **array= (double **)malloc(rows*sizeof(double*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

void free_2d_double(double **array) {
	free(array[0]);
	free(array);
}