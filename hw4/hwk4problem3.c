#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void source(int rank, int dest_rank, char** argv);
void destination(int rank, int source_rank, char** argv);

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int total_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (argc == 5) {
      int r1 = atoi(argv[1]);
      int r2 = atoi(argv[2]);

      if (rank == r1) {
        source(r1, r2, argv);
      } else if (rank == r2) {
        destination(r2, r1, argv);
      }
  } else {
    printf("Invalid number of arguments. Expecting 4, recieved %d\n",argc-1);
    fflush(stdout);
  }

  MPI_Finalize();
  return 0;
}

void source(int rank, int dest_rank, char** argv) {
    int size = atoi(argv[3]);
    int samples = atoi(argv[4]);
    char bytes[size];

    MPI_Status status;
    double starttime, endtime;
    double total = 0;
    int i;
    for (i = 0; i < samples; i++) {
        starttime = MPI_Wtime();

        MPI_Send(bytes,size,MPI_CHAR,dest_rank,0,MPI_COMM_WORLD);
        MPI_Recv(bytes,size,MPI_CHAR,dest_rank,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

        endtime = MPI_Wtime();
        total += endtime-starttime;
    }
    total /= samples;
    total *= MPI_Wtick();

    total = size/(2*total);

    printf("Data Size: %dB\n",size);
    printf("Number of Samples: %d\n",samples);
    printf("Connection Speed between rank %d and %d was %f MB/s\n", rank, dest_rank, total);
    printf("\nHere is the machine-process pairing\n\n");

    char name[1000];
    gethostname(name,1000);
    printf("Rank %d: %s\n", rank, name);

    int n = 0;
    MPI_Send(&n,1,MPI_INT,dest_rank,0,MPI_COMM_WORLD);
}

void destination(int rank, int source_rank, char** argv) {
    int size = atoi(argv[3]);
    int samples = atoi(argv[4]);
    char bytes[size];

    MPI_Status status;
    int i;
    for (i = 0; i < samples; i++) {
        MPI_Recv(bytes,size,MPI_CHAR,source_rank,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        MPI_Send(bytes,size,MPI_CHAR,source_rank,0,MPI_COMM_WORLD);
    }

    int n;
    MPI_Recv(&n,1,MPI_INT,source_rank,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

    char name[1000];
    gethostname(name,1000);
    printf("Rank %d: %s\n", rank, name);
}
