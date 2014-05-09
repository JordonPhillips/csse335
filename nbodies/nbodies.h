typedef struct {
  double *position;
  double *velocity;
  int dimensions;
  double mass;
} Body;

void master(char *input_filename, char *output_filename, int timestep_size, int number_of_timesteps);
void slave();
