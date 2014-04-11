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

    int i, k, j = 0;
    for (i = 0; i < total_procs; i++) {
        if (i != root) {
	        data = (char*)data + (j*send_size);
            memcpy(send, data, send_size);

            for (k=0;k<sendcount;k++)
                printf("Seding %d to rank %d\n", *((int*)data+k), i);
            fflush(stdout);

            MPI_Send(send, sendcount, sendtype, i, MY_SCATTER_TAG, comm);
            j++;
        }
    }

    free(send);
  } else {
    MPI_Status status;
    MPI_Recv(recvbuf,recvcount,recvtype,root,MY_SCATTER_TAG,comm,&status);
  }
}

