#include <mpi.h>

#define MY_BROADCAST_TAG 46

void MY_Broadcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    if (rank == root) {
        int total_procs;
        MPI_Comm_size(comm, &total_procs);

        int i;
        for (i = 0; i < total_procs; i++) {
            if (i != root)
                MPI_Send(buffer, count, datatype, i, MY_BROADCAST_TAG, comm);
        }
    } else {
        MPI_Status status;
        MPI_Recv(buffer, count, datatype, root, MY_BROADCAST_TAG, comm, &status);
    }
}

