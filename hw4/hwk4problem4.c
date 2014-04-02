#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

void master(int total_procs);
void slave();

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int total_procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == 0) {
        master(total_procs);
    } else {
        slave(rank, total_procs);
    }

    MPI_Finalize();
    return 0;
}

void master(int total_procs) {
    int num_trials;

    printf("Enter the number of trials to perform: ");
    fflush(stdout);
    scanf("%d",&num_trials);

    float pi = 0;
    int i;
    for (i = 1; i < total_procs; i++) {
        MPI_Send(&num_trials,1,MPI_INT,i,0,MPI_COMM_WORLD);
    }

    float n;
    MPI_Status status;
    for (i = 1; i < total_procs; i++) {
        MPI_Recv(&n,1,MPI_FLOAT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        pi += n;
    }

    pi /= (total_procs-1);
    printf("Pi was calculated to be %f\n",pi);
}

void slave() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec * tv.tv_sec);

    int t;
    MPI_Status status;
    MPI_Recv(&t,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

    int i;
    float x, y;
    double l;
    int s = 0;
    for (i = 0; i < t; i++) {
        x = (float)rand()/RAND_MAX;
        y = (float)rand()/RAND_MAX;

        l = sqrt((x*x)+(y*y));
        if (l <= 1) s++;
    }
    float pi = (float)4*s/t;

    MPI_Send(&pi,1,MPI_FLOAT,0,0,MPI_COMM_WORLD);
}
