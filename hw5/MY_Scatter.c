#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MY_SCATTER_TAG 44

void MY_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm) {
  int rank;
  MPI_Comm_rank(comm, &rank);

  if (rank == root) {
    int total_procs, send_size;
    MPI_Comm_size(comm, &total_procs);
    MPI_Type_size(sendtype, &send_size);

    send_size *= sendcount;
    void *data = sendbuf;
    void *send = malloc(send_size);

    int i;
    for (i = 0; i < total_procs; i++) {
        if (i != root) {
	    printf("Copying data\n");
	    fflush(stdout);
	    data = (char*)data + (i*send_size);
            memcpy(send, data, send_size);

            printf("Sending data from %d to %d.\n", rank, i);
            fflush(stdout);
            MPI_Send(send, sendcount, sendtype, i, MY_SCATTER_TAG, comm);
        }
    }

    free(send);
  } else {
    MPI_Status status;
    MPI_Recv(recvbuf,recvcount,recvtype,root,MY_SCATTER_TAG,comm,&status);
    printf("Rank %d recieved data.\n", rank);
    fflush(stdout);
  }
}

