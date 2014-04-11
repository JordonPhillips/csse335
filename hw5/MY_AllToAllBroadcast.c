#include <mpi.h>
#include <string.h>

void MY_AllToAllBroadcast(void *sendbuf, int sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
    MPI_Datatype recvtype, MPI_Comm comm) {

    MPI_Gather(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, 0, comm);
    MPI_Bcast(recvbuf, recvcount, recvtype, 0, comm);
}

