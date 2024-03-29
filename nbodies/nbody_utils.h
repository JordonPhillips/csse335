#define DIMENSIONS 2
#define X 0
#define Y 1
#define DX 2
#define DY 3
#define MASS 4
#define G .0000000000667384
#define CHARS_PER_DOUBLE 24

void read_particles(char *filename, double ***particles, int *n);
void write_particles(char *filename, double **particles, int n);

double **alloc_2d_double(int rows, int cols);
void free_2d_double(double **array);