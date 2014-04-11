#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#define MY_ATOABC_TAG 50

void MY_AllToAllBroadcast(void *sendbuf, int sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
    MPI_Datatype recvtype, MPI_Comm comm) {

    int rank, total_procs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);
    MPI_Status status;

    int send_size, recv_size;
    MPI_Type_size(sendtype, &send_size);
    MPI_Type_size(recvtype, &recv_size);

    send_size *= sendcount;
    recv_size *= recvcount;

    if (rank == 0) {
        void *data = recvbuf;
        memcpy(data, sendbuf, send_size);

        int i;
        for (i = 1; i < total_procs; i++) {
            data = (char*)data + recv_size;
            MPI_Recv(data,sendcount,sendtype,i,MY_ATOABC_TAG,comm,&status);
        }

        for (i = 1; i < total_procs; i++) {
            MPI_Send(recvbuf,total_procs*recvcount,recvtype,i,MY_ATOABC_TAG,comm);
        }
    } else {
        MPI_Send(sendbuf, sendcount, sendtype, 0, MY_ATOABC_TAG, comm);
        MPI_Recv(recvbuf, recvcount*total_procs, recvtype, 0, MY_ATOABC_TAG, comm, &status);
    }
}

