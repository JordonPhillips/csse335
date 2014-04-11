#include <mpi.h>

#define MY_GATHER_TAG 45

void MY_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    if (rank == root) {
        MPI_Status status;
        int total_procs, data_size;
        MPI_Comm_size(comm, &total_procs);
        MPI_Type_size(recvtype, &data_size);

        data_size *= recvcount;
        void *data = recvbuf;

        int i;
        for (i = 0; i < total_procs; i++) {
            if (i != root) {
                MPI_Recv(data,recvcount,recvtype,i,MY_GATHER_TAG,comm,&status);
                data = (char*)data + data_size;
            }
        }
    } else {
        MPI_Send(sendbuf,sendcount,sendtype,root,MY_GATHER_TAG,comm);
    }
}

