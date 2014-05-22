void master(char *input_filename, char *output_filename, int timestep_size,
	int number_of_timesteps);
void slave(int timestep_size, int number_of_timesteps);
void nbody_parallel(double ***particles, int n, int timesteps,
	int timestep_size, double g, int root, MPI_Comm comm);
void get_forces(double **forces, double **input_particles, int num_particles,
	int target_particle, int num_target_particles, double g);