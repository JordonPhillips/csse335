#include <mpi.h>

#define MY_BROADCAST_TAG 46

void MY_Broadcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    int total_procs;
    MPI_Comm_size(comm, &total_procs);

    if (rank == root) {
        if (total_procs > 1)
            MPI_Send(buffer, count, datatype, 1, MY_BROADCAST_TAG, comm);
        if (total_procs > 2)
            MPI_Send(buffer, count, datatype, 2, MY_BROADCAST_TAG, comm);
    } else {
        int sender = rank%2 == 0 ? ((rank/2)-1) : ((rank-1)/2);
        MPI_Status status;
        MPI_Recv(buffer, count, datatype, sender, MY_BROADCAST_TAG, comm, &status);

        int dest = (2*rank)+1;
        if (dest < total_procs) {
            MPI_Send(buffer, count, datatype, dest, MY_BROADCAST_TAG, comm);

            dest += 1;
            if (dest < total_procs)
                MPI_Send(buffer, count, datatype, dest, MY_BROADCAST_TAG, comm);
        }
    }
}

