#define DIMENSIONS 2
#define CHARS_PER_DOUBLE 24

void read_particles(char *filename, double ***particles, int *n);
void write_particles(char *filename, double **particles, int n);
void nbody_serial(double ***particles, int n);
