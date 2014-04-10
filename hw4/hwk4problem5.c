#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

void master(int total_procs, int n);
void slave(float h);
float rand_lim(int limit);
void gen_shekel_vars(float* buff, int num);
float shekel(float i, float j, float k, float* vars);

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int n = 11;
  if (rank == 0) {
    master(total_procs, n);
  } else {
    float h = 10 / (n - 1);
    slave(h);
  }

  MPI_Finalize();
  return 0;
}

void master(int total_procs, int n) {
    srand(time(NULL));
    float* shekel_vars = (float*)malloc(100*sizeof(float));
    gen_shekel_vars(shekel_vars, 20);
    int i,j,k,p;

    for (i = 1; i < total_procs; i++) {
        MPI_Send(shekel_vars,100,MPI_FLOAT,i,0,MPI_COMM_WORLD);
    }

    p=0;
    int* vars = (int*)malloc(3*sizeof(int));

    if (total_procs > 1) {
        for (i = 0; i < n; i++) {
            vars[0]=i;
            for (j = 0; j < n; j++) {
                vars[1]=j;
                for (k = 0; k < n; k++) {
                    vars[2]=k;
                    p = p%(total_procs-1);
		    printf("Sending (%d,%d,%d) to %d\n",vars[0],vars[1],vars[2],p);
                    MPI_Send(vars,3,MPI_INT,p,0,MPI_COMM_WORLD);
                    p++;
                }
            }
        }
    }

    vars[0] = -1;
    for (i = 1; i < total_procs; i++) {
        MPI_Send(vars,3,MPI_INT,i,0,MPI_COMM_WORLD);
    }

    float max[4] = {FLT_MIN, 0, 0, 0};
    float recv[4];
    MPI_Status status;
    for (i = 1; i < total_procs; i++) {
        MPI_Recv(recv,4,MPI_FLOAT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        if (max[0] < recv[0]) {
            memcpy(&max,&recv,sizeof(recv));
        }
    }

    printf("Max Temp: %f\nPoint Achieved (%f,%f,%f)\n", max[0],max[1],max[2],max[3]);

    free(vars);
    free(shekel_vars);
}

void slave(float h) {
    MPI_Status status;
    float shekel_vars[100];
    MPI_Recv(shekel_vars,100,MPI_FLOAT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

    int vars[3];
    int max_vars[3];
    float tmp, max = FLT_MIN;
    while (1) {
        MPI_Recv(vars,3,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

        if (vars[0] < 0) break;

        tmp = shekel(vars[0]*h,vars[1]*h,vars[2]*h, shekel_vars);
	printf("Shekel: (%d,%d,%d) => %f\n",vars[0],vars[1],vars[2]);

        if (max < tmp) {
            max = tmp;
            memcpy(&max_vars, &vars, sizeof(vars));
        }
    }

    float max_all[4] = {max, (float)max_vars[0], (float)max_vars[1], (float)max_vars[2]};
    printf("Local max is %f at point (%f,%f,%f)\n",max_all[0],max_all[1],max_all[2],max_all[3]);
    fflush(stdout);
    MPI_Send(max_all,4,MPI_FLOAT,0,0,MPI_COMM_WORLD);
}

float shekel(float i, float j, float k, float* vars) {
    int m, v;
    float total=0;
    for (m = 1; m <= 20; m++) {
        v = (m-1)*5;
        total += vars[v+4]/(pow(i+vars[v],2)+pow(j+vars[v+1],2)+pow(k+vars[v+2],2)+vars[v+3]);
    }
    return total;
}

void gen_shekel_vars(float* buff, int num) {
    int i, j = 0;
    for (i = 0; i < num; i++) {
        buff[i] = rand_lim(10);
        buff[i+1] = rand_lim(10);
        buff[i+2] = rand_lim(10);
        buff[i+3] = rand_lim(10);
        buff[i+4] = rand_lim(20);

        printf("Initializing a%d=%f b%d=%f c%d=%f d%d=%f k%d=%f\n",
                i+1,buff[i],i+1,buff[i+1],i+1,buff[i+2],i+1,buff[i+3]
                ,i+1,buff[i+4]
        );
    }
}

float rand_lim(int limit) {
  return (float)limit*rand()/RAND_MAX;
}
